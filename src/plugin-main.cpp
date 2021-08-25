/*
Nudgis-OBS-Plugin
Copyright (C) 2021 Ubicast 
*/
 
#include <obs-module.h>
#include <obs-frontend-api.h>
#include "plugin-macros.generated.h"
#include "plugin-main.hpp"
#include "nudgis-service.h"
#include "ui_settings.h"

#include <json/json.h>
#include <curl/curl.h>
#include <QMainWindow>
#include <QApplication>
#include <QAction>
#include <QtWidgets>
#include <iostream>
#include <fstream>
#include <sys/stat.h>

bool fileExists(const char* file) 
{
    struct stat buf;
    return (stat(file, &buf) == 0);
}

/*NUDGIS SETTINGS METHODS DEFINITION*/
/*-----------------------------------------------------------------------------------------------*/
static NudgisSettings *settingsWindow = nullptr;

NudgisSettings::NudgisSettings(): QWidget(nullptr), ui(new Ui_Settings)
{
    blog(LOG_INFO, "Plugin Settings Opened (version %s)", PLUGIN_VERSION);
    ui->setupUi(this);
    QString q1_apiKey = QString::fromUtf8("");
    QString q1_url = QString::fromUtf8("");
    QString q1_streamTitle = QString::fromUtf8("");
    QString q1_streamChannel = QString::fromUtf8("");
    QString q1_vodTitle = QString::fromUtf8("");
    QString q1_vodChannel = QString::fromUtf8("");
    ui->lineEdit_2->setText(q1_apiKey);
    ui->lineEdit->setText(q1_url);
    ui->lineEdit_3->setText(q1_streamTitle);
    ui->lineEdit_4->setText(q1_streamChannel);
    ui->lineEdit_5->setText(q1_vodChannel);
    ui->lineEdit_6->setText(q1_vodTitle);
    if(fileExists("data.json"))
    {   
        std::ifstream f("data.json");
        Json::Reader reader;
        Json::Value settings;
        f >> settings;
        
        std::string p_apiKey = settings["apiKey"].toStyledString();
        std::string p_url = settings["url"].toStyledString();
        std::string p_streamTitle = settings["streamTitle"].toStyledString();
        std::string p_streamChannel = settings["streamChannel"].toStyledString();
        std::string p_vodTitle = settings["vodTitle"].toStyledString();
        std::string p_vodChannel = settings["vodChannel"].toStyledString();
        p_apiKey.erase(remove(p_apiKey.begin(), p_apiKey.end(), '\n'), p_apiKey.end());
        p_url.erase(remove(p_url.begin(), p_url.end(), '\n'), p_url.end());
        p_streamTitle.erase(remove(p_streamTitle.begin(), p_streamTitle.end(), '\n'), p_streamTitle.end());
        p_streamChannel.erase(remove(p_streamChannel.begin(), p_streamChannel.end(), '\n'), p_streamChannel.end());
        p_vodTitle.erase(remove(p_vodTitle.begin(), p_vodTitle.end(), '\n'), p_vodTitle.end());
        p_vodChannel.erase(remove(p_vodChannel.begin(), p_vodChannel.end(), '\n'), p_vodChannel.end());

        std::string t_apiKey = NudgisSettings::removeQuotes(p_apiKey);
        std::string t_url = NudgisSettings::removeQuotes(p_url);
        std::string t_streamTitle = NudgisSettings::removeQuotes(p_streamTitle);
        std::string t_streamChannel = NudgisSettings::removeQuotes(p_streamChannel);
        std::string t_vodTitle = NudgisSettings::removeQuotes(p_vodTitle);
        std::string t_vodChannel = NudgisSettings::removeQuotes(p_vodChannel);

        QString q_apiKey = QString::fromUtf8(t_apiKey.c_str());
        QString q_url = QString::fromUtf8(t_url.c_str());
        QString q_streamTitle = QString::fromUtf8(t_streamTitle.c_str());
        QString q_streamChannel = QString::fromUtf8(t_streamChannel.c_str());
        QString q_vodTitle = QString::fromUtf8(t_vodTitle.c_str());
        QString q_vodChannel = QString::fromUtf8(t_vodChannel.c_str());
        ui->lineEdit_2->setText(q_apiKey);
        ui->lineEdit->setText(q_url);
        ui->lineEdit_3->setText(q_streamTitle);
        ui->lineEdit_4->setText(q_streamChannel);
        ui->lineEdit_5->setText(q_vodChannel);
        ui->lineEdit_6->setText(q_vodTitle);
    }

    connect(ui->pushButton, &QPushButton::clicked, this, &NudgisSettings::clearWindow);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &NudgisSettings::saveSettings);
}   

void NudgisSettings::clearWindow()
{
    ui->lineEdit->setText(QString());
    ui->lineEdit_2->setText(QString());
    ui->lineEdit_3->setText(QString());
    ui->lineEdit_4->setText(QString());
    ui->lineEdit_5->setText(QString());
    ui->lineEdit_6->setText(QString());
    blog(LOG_INFO, "Window Cleared Succesfully !");
    //std::cout << ui->lineEdit->text().toStdString() << std::endl;
}

void NudgisSettings::saveSettings()
{
    url = ui->lineEdit->text().toStdString();
    apiKey = ui->lineEdit_2->text().toStdString();
    streamTitle = ui->lineEdit_3->text().toStdString();
    streamChannel = ui->lineEdit_4->text().toStdString();
    vodChannel = ui->lineEdit_5->text().toStdString();
    vodTitle = ui->lineEdit_6->text().toStdString();
    if(ui->checkBox->checkState() == Qt::Checked)
    {
        NudgisSettings::box();
    }
    Json::Value rootJsonValue;
    rootJsonValue["apiKey"] = apiKey;
    rootJsonValue["url"] = url; 
    rootJsonValue["streamTitle"] = streamTitle; 
    rootJsonValue["streamChannel"] = streamChannel; 
    rootJsonValue["vodTitle"] = vodTitle; 
    rootJsonValue["vodChannel"] = vodChannel; 


    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "   ";

    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ofstream outputFileStream("data.json");
    writer -> write(rootJsonValue, &outputFileStream);
    this->close();
}

void NudgisSettings::box()
{
    std::cout << "VOD will be uploaded at the end of the recording" << std::endl;
    upload = true;
}

std::string NudgisSettings::removeQuotes(std::string str)
{
    for(unsigned int i=0; i< str.size(); ++i)  //On parcourt la chaine
    {
        if(str[i] == '"')     //Si c'est un '"'
        {
            str.erase(i,1);   //Efface le i-ème caractère
        }
    }
    return str;
}



/*HANDLING OBS EVENTS*/
/*-----------------------------------------------------------------------------------------------*/
bool isRecorded = false;

static void obs_event(enum obs_frontend_event event, void *)
{
    switch(event)
    {
        case OBS_FRONTEND_EVENT_EXIT:
            delete settingsWindow;
        
        case OBS_FRONTEND_EVENT_STREAMING_STARTING:
            blog(LOG_INFO, "Streaming Preparing");

        case OBS_FRONTEND_EVENT_STREAMING_STARTED:
            blog(LOG_INFO, "Streaming Started");
        
        case OBS_FRONTEND_EVENT_STREAMING_STOPPING:
            blog(LOG_INFO, "Streaming Stopping");
        
        case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
            blog(LOG_INFO, "Streaming Stopped");
        
        case OBS_FRONTEND_EVENT_RECORDING_STARTED:
            blog(LOG_INFO, "Recording Started");
            isRecorded = true;

        case OBS_FRONTEND_EVENT_RECORDING_STOPPED:
            blog(LOG_INFO, "Recording Stopped");
    }
}


void prepareStream(char * streamData)
{
    FILE * prepjson;
    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(curl) 
    {
    curl_easy_setopt(curl, CURLOPT_URL, "https://beta.ubicast.net/api/v2/lives/prepare/");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, streamData);
    res = curl_easy_perform(curl);

    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

void startStream()
{
    
}

void stopStream()
{

}

/*LOAD AND UNLOAD OF THE PLUGIN*/
/*-----------------------------------------------------------------------------------------------*/



OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

static obs_service_t * nudgis_service = NULL;

const char *obs_module_name(void)
{
	return "Nudgis Obs Plugin";
}

void openWindow()
{
    if (!settingsWindow) 
    {
        settingsWindow = new NudgisSettings();
        settingsWindow->resize(375, 410);
        settingsWindow->show();
        settingsWindow->setWindowTitle(QApplication::translate("toplevel", "Nudgis Plugin Settings"));
	} 
    else 
    {
        settingsWindow->show();
        settingsWindow->setWindowTitle(QApplication::translate("toplevel", "Nudgis Plugin Settings"));
        settingsWindow->raise();
	}
}

void closeWindow()
{
    if (!settingsWindow)
    {
        
    }
    else 
    {
        settingsWindow->close();
	}
}


bool obs_module_load()
{
    blog(LOG_INFO, "Nudgis plugin loaded successfully (version %s)", PLUGIN_VERSION);
    QAction * menu_action = (QAction*) obs_frontend_add_tools_menu_qaction("Nudgis Plugin Settings");
    blog(LOG_INFO, "[%s] Menu entry for Settings added", obs_module_name());
    menu_action->connect(menu_action, &QAction::triggered, openWindow);
    nudgis_service_register();
    nudgis_service = obs_service_create("nudgis",NULL,NULL,nullptr);
    obs_frontend_set_streaming_service(nudgis_service);
    obs_frontend_save_streaming_service();
    obs_frontend_add_event_callback(obs_event, nullptr);
    return true;
}

void obs_module_unload()
{
    blog(LOG_INFO, "Nudgis Plugin Successfully Unloaded");
    if (nudgis_service != NULL)
    {
      obs_service_release(nudgis_service);
      nudgis_service = NULL;
    }
}


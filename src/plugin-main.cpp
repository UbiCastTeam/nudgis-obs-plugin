/*
Nudgis-OBS-Plugin
Copyright (C) 2021 Ubicast 
*/
 
#include <obs-module.h>
#include <obs-frontend-api.h>
#include "plugin-macros.generated.h"
#include "plugin-main.hpp"
#include "nudgis-service.hpp"
#include "ui_settings.h"
#include <QMainWindow>
#include <QApplication>
#include <QAction>
#include <QtWidgets>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include "nudgis-config.hpp"

/*NUDGIS SETTINGS METHODS DEFINITION*/
/*-----------------------------------------------------------------------------------------------*/
static NudgisSettings *settingsWindow = nullptr;

NudgisSettings::NudgisSettings(): QWidget(nullptr), ui(new Ui_Settings)
{
    blog(LOG_INFO, "Plugin Settings Opened (version %s)", PLUGIN_VERSION);
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, this, &NudgisSettings::clearWindow);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &NudgisSettings::saveSettings);
}

void NudgisSettings::showEvent(QShowEvent *event)
{
    this->loadSettings();
    QWidget::showEvent(event);
}

void NudgisSettings::loadSettings()
{
    NudgisConfig * nudgis_config = NudgisConfig::GetCurrentNudgisConfig();
    ui->url->setText(QString(nudgis_config->url.c_str()));
    ui->api_key->setText(QString(nudgis_config->api_key.c_str()));
    ui->stream_title->setText(QString(nudgis_config->stream_title.c_str()));
    ui->stream_channel->setText(QString(nudgis_config->stream_channel.c_str()));
    ui->vod_title->setText(QString(nudgis_config->vod_title.c_str()));
    ui->vod_channel->setText(QString(nudgis_config->vod_channel.c_str()));
}

void NudgisSettings::clearWindow()
{
    QLineEdit * lines_edit  [] =
    {
        ui->url,
        ui->api_key,
        ui->stream_title,
        ui->stream_channel,
        ui->vod_title,
        ui->vod_channel,
    };
    for (QLineEdit * line_edit : lines_edit)
        line_edit->clear();
    blog(LOG_INFO, "Window Cleared Succesfully !");
}

void NudgisSettings::saveSettings()
{
    NudgisConfig * nudgis_config = NudgisConfig::GetCurrentNudgisConfig();
    nudgis_config->url = ui->url->text().toStdString();
    nudgis_config->api_key = ui->api_key->text().toStdString();
    nudgis_config->stream_title = ui->stream_title->text().toStdString();
    nudgis_config->stream_channel = ui->stream_channel->text().toStdString();
    nudgis_config->vod_title = ui->vod_title->text().toStdString();
    nudgis_config->vod_channel = ui->vod_channel->text().toStdString();
    nudgis_config->save();
    this->close();
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
            break;
        
        case OBS_FRONTEND_EVENT_STREAMING_STARTING:
            blog(LOG_INFO, "Streaming Preparing");
            break;

        case OBS_FRONTEND_EVENT_STREAMING_STARTED:
            blog(LOG_INFO, "Streaming Started");
            break;
        
        case OBS_FRONTEND_EVENT_STREAMING_STOPPING:
            blog(LOG_INFO, "Streaming Stopping");
            break;
        
        case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
            blog(LOG_INFO, "Streaming Stopped");
            break;
        
        case OBS_FRONTEND_EVENT_RECORDING_STARTED:
            blog(LOG_INFO, "Recording Started");
            isRecorded = true;
            break;

        case OBS_FRONTEND_EVENT_RECORDING_STOPPED:
            blog(LOG_INFO, "Recording Stopped");
            break;

        default:
            blog(LOG_INFO, "Receive other event: %d",event);
            break;

    }
}


void prepareStream(char * streamData)
{
    FILE * prepjson;
    (void)prepjson;
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
	} 
    else 
        settingsWindow->raise();

    settingsWindow->setWindowTitle(QApplication::translate("toplevel", "Nudgis Plugin Settings"));
    //settingsWindow->loadSettings();
    settingsWindow->show();
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
    obs_frontend_add_event_callback(obs_event, nullptr);
    return true;
}

void obs_module_unload()
{
    blog(LOG_INFO, "Nudgis Plugin Successfully Unloaded");
}

/*
Nudgis-OBS-Plugin
Copyright (C) 2021 Ubicast 
*/
#include "nudgis-plugin.hpp"
#include "nudgis-config.hpp"
#include "nudgis-service.hpp"
#include "plugin-macros.generated.h"
#include "ui_settings.h"

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <QAction>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

/*NUDGIS SETTINGS METHODS DEFINITION*/
/*-----------------------------------------------------------------------------------------------*/
static NudgisSettings *settingsWindow = nullptr;

NudgisSettings::NudgisSettings()
        : QWidget(nullptr), ui(new Ui_Settings)
{
    mlog(LOG_INFO, "Plugin Settings Opened (version %s)", PLUGIN_VERSION);
    ui->setupUi(this);

    connect(ui->btn_clearWindow, &QPushButton::clicked, this, &NudgisSettings::clearWindow);
    connect(ui->btn_saveSettings, &QPushButton::clicked, this, &NudgisSettings::saveSettings);
}

void NudgisSettings::showEvent(QShowEvent *event)
{
    this->loadSettings();
    QWidget::showEvent(event);
}

void NudgisSettings::loadSettings()
{
    NudgisConfig *nudgis_config = NudgisConfig::GetCurrentNudgisConfig();
    ui->url->setText(QString(nudgis_config->url.c_str()));
    ui->api_key->setText(QString(nudgis_config->api_key.c_str()));
    ui->stream_title->setText(QString(nudgis_config->stream_title.c_str()));
    ui->stream_channel->setText(QString(nudgis_config->stream_channel.c_str()));
    ui->vod_title->setText(QString(nudgis_config->vod_title.c_str()));
    ui->vod_channel->setText(QString(nudgis_config->vod_channel.c_str()));
}

void NudgisSettings::clearWindow()
{
    QLineEdit *lines_edit[] =
            {
                    ui->url,
                    ui->api_key,
                    ui->stream_title,
                    ui->stream_channel,
                    ui->vod_title,
                    ui->vod_channel,
            };
    for (QLineEdit *line_edit : lines_edit)
        line_edit->clear();
    mlog(LOG_INFO, "Window Cleared Succesfully !");
}

void NudgisSettings::saveSettings()
{
    NudgisConfig *nudgis_config = NudgisConfig::GetCurrentNudgisConfig();
    nudgis_config->url = ui->url->text().toStdString();
    nudgis_config->api_key = ui->api_key->text().toStdString();
    nudgis_config->stream_title = ui->stream_title->text().toStdString();
    nudgis_config->stream_channel = ui->stream_channel->text().toStdString();
    nudgis_config->vod_title = ui->vod_title->text().toStdString();
    nudgis_config->vod_channel = ui->vod_channel->text().toStdString();
    nudgis_config->save();
    this->close();
}

/*LOAD AND UNLOAD OF THE PLUGIN*/
/*-----------------------------------------------------------------------------------------------*/

const char *obs_module_name(void)
{
    return "Nudgis Obs Plugin";
}

void openWindow()
{
    if (!settingsWindow) {
        settingsWindow = new NudgisSettings();
        settingsWindow->resize(375, 410);
        settingsWindow->setWindowTitle(QApplication::translate("toplevel", "Nudgis Plugin Settings"));
    }

    settingsWindow->raise();
    settingsWindow->show();
}

bool obs_module_load()
{
    mlog(LOG_INFO, "Nudgis plugin loaded successfully (version %s)", PLUGIN_VERSION);
    QAction *menu_action = (QAction *)obs_frontend_add_tools_menu_qaction("Nudgis Plugin Settings");
    mlog(LOG_INFO, "Menu entry for Settings added");
    menu_action->connect(menu_action, &QAction::triggered, openWindow);
    obs_register_service(&nudgis_service);
    return true;
}

void obs_module_unload()
{
    mlog(LOG_INFO, "Nudgis Plugin Successfully Unloaded");
    if (settingsWindow != NULL)
        delete settingsWindow;
}

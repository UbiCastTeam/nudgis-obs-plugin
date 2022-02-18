/*
Nudgis-OBS-Plugin
Copyright (C) 2021 Ubicast 
*/
#include "nudgis-plugin.hpp"
#include "nudgis-config.hpp"
#include "nudgis-service.hpp"
#include "plugin-macros.generated.h"
#include "ui_settings.h"
#include "obs-app.hpp"

#include <obs-module.h>
#include <QAction>

Q_DECLARE_METATYPE(const char*)

#define TEXT_NUDGISPLUGIN_SETTINGS_MENUTITLE obs_module_text("NudgisPlugin.settings.MenuTitle")

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

/*NUDGIS SETTINGS METHODS DEFINITION*/
/*-----------------------------------------------------------------------------------------------*/
static NudgisSettings *settingsWindow = nullptr;
static obs_service_t *nudgis_service = nullptr;

NudgisSettings::NudgisSettings()
        : QWidget(nullptr), ui(new Ui_Settings)
{
    mlog(LOG_INFO, "Plugin Settings Opened (version %s)", PLUGIN_VERSION);
    ui->setupUi(this);
    QComboBox *combos_auto_state[] = {ui->auto_delete_uploaded_file, ui->publish_recording_automatically};
    for (QComboBox * combo_auto_state: combos_auto_state)
    {
        const char **all_auto_states = NudgisConfig::GetAllAutoStates();
        for (size_t i=0;i < NudgisConfig::GetAutoStatesCount();i++ )
            combo_auto_state->addItem(obs_module_text(all_auto_states[i]), QVariant::fromValue(all_auto_states[i]));
    }
}

void NudgisSettings::showEvent(QShowEvent *event)
{
    this->loadSettings();
    this->Set_sw_EchoMode(QLineEdit::Password);
    QWidget::showEvent(event);
}

void NudgisSettings::loadSettings()
{
    NudgisConfig *nudgis_config = NudgisConfig::GetCurrentNudgisConfig();
    ui->url->setText(QString(nudgis_config->url.c_str()));
    ui->api_key->setText(QString(nudgis_config->api_key.c_str()));
    ui->stream_title->setText(QString(nudgis_config->stream_title.c_str()));
    ui->stream_channel->setText(QString(nudgis_config->stream_channel.c_str()));
    ui->auto_delete_uploaded_file->setCurrentIndex(ui->auto_delete_uploaded_file->findData(QVariant::fromValue(nudgis_config->auto_delete_uploaded_file)));
    ui->publish_recording_automatically->setCurrentIndex(ui->publish_recording_automatically->findData(QVariant::fromValue(nudgis_config->publish_recording_automatically)));
}

void NudgisSettings::on_btn_clearWindow_clicked()
{
    QLineEdit *lines_edit[] =
            {
                    ui->url,
                    ui->api_key,
                    ui->stream_title,
                    ui->stream_channel,
            };
    for (QLineEdit *line_edit : lines_edit)
        line_edit->clear();
    QComboBox *combos_auto_state[] = {ui->auto_delete_uploaded_file, ui->publish_recording_automatically};
    for (QComboBox * combo_auto_state: combos_auto_state)
        combo_auto_state->setCurrentIndex(0);
    mlog(LOG_INFO, "%s", "Window Cleared Succesfully !");
}

void NudgisSettings::on_btn_saveSettings_clicked()
{
    NudgisConfig *nudgis_config = NudgisConfig::GetCurrentNudgisConfig();
    nudgis_config->url = ui->url->text().toStdString();
    nudgis_config->api_key = ui->api_key->text().toStdString();
    nudgis_config->stream_title = ui->stream_title->text().toStdString();
    nudgis_config->stream_channel = ui->stream_channel->text().toStdString();
    nudgis_config->auto_delete_uploaded_file = ui->auto_delete_uploaded_file->currentData().value<const char *>();
    nudgis_config->publish_recording_automatically = ui->publish_recording_automatically->currentData().value<const char *>();
    nudgis_config->save();
    obs_frontend_set_streaming_service(nudgis_service);
    obs_frontend_save_streaming_service();
    this->close();
}

void NudgisSettings::on_btn_sw_EchoMode_clicked()
{
    this->Set_sw_EchoMode(ui->api_key->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password);
}

void NudgisSettings::Set_sw_EchoMode(QLineEdit::EchoMode mode)
{
    if (mode == QLineEdit::Password)
    {
        ui->api_key->setEchoMode(QLineEdit::Password);
        ui->btn_sw_EchoMode->setText(QTStr("Show"));
    }
    else
    {
        ui->api_key->setEchoMode(QLineEdit::Normal);
        ui->btn_sw_EchoMode->setText(QTStr("Hide"));
    }
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
        obs_frontend_push_ui_translation(obs_module_get_string);
        settingsWindow = new NudgisSettings();
        obs_frontend_pop_ui_translation();
    }

    settingsWindow->raise();
    settingsWindow->show();
}

bool obs_module_load()
{
    mlog(LOG_INFO, "Nudgis plugin loaded successfully (version %s)", PLUGIN_VERSION);
    QAction *menu_action = (QAction *)obs_frontend_add_tools_menu_qaction(TEXT_NUDGISPLUGIN_SETTINGS_MENUTITLE);
    mlog(LOG_INFO, "%s", "Menu entry for Settings added");
    menu_action->connect(menu_action, &QAction::triggered, openWindow);
    obs_register_service(&nudgis_service_info);
    nudgis_service = obs_service_create(nudgis_service_info.id, nudgis_service_info.get_name(nullptr), nullptr, nullptr);

    return true;
}

void obs_module_unload()
{
    mlog(LOG_INFO, "%s", "Nudgis Plugin Successfully Unloaded");
    if (settingsWindow != NULL)
        delete settingsWindow;
    if (nudgis_service != NULL)
        obs_service_release(nudgis_service);
}

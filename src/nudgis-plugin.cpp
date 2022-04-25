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
#include "nudgis-upload-ui.hpp"

#include <obs-module.h>
#include <QAction>

Q_DECLARE_METATYPE(const AutoState *)

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
    for (QComboBox *combo_auto_state : combos_auto_state) {
        for (const AutoState &autostate : AutoState::GetAll())
            combo_auto_state->addItem(obs_module_text(autostate.name.c_str()), QVariant::fromValue(&autostate));
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
    for (QComboBox *combo_auto_state : combos_auto_state)
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
    nudgis_config->auto_delete_uploaded_file = ui->auto_delete_uploaded_file->currentData().value<const AutoState *>();
    nudgis_config->publish_recording_automatically = ui->publish_recording_automatically->currentData().value<const AutoState *>();
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
    if (mode == QLineEdit::Password) {
        ui->api_key->setEchoMode(QLineEdit::Password);
        ui->btn_sw_EchoMode->setText(QTStr("Show"));
    } else {
        ui->api_key->setEchoMode(QLineEdit::Normal);
        ui->btn_sw_EchoMode->setText(QTStr("Hide"));
    }
}

/*LOAD AND UNLOAD OF THE PLUGIN*/
/*-----------------------------------------------------------------------------------------------*/

static const char *obs_frontend_event_str[] =
        {
                [OBS_FRONTEND_EVENT_STREAMING_STARTING] = "STREAMING_STARTING",
                [OBS_FRONTEND_EVENT_STREAMING_STARTED] = "STREAMING_STARTED",
                [OBS_FRONTEND_EVENT_STREAMING_STOPPING] = "STREAMING_STOPPING",
                [OBS_FRONTEND_EVENT_STREAMING_STOPPED] = "STREAMING_STOPPED",
                [OBS_FRONTEND_EVENT_RECORDING_STARTING] = "RECORDING_STARTING",
                [OBS_FRONTEND_EVENT_RECORDING_STARTED] = "RECORDING_STARTED",
                [OBS_FRONTEND_EVENT_RECORDING_STOPPING] = "RECORDING_STOPPING",
                [OBS_FRONTEND_EVENT_RECORDING_STOPPED] = "RECORDING_STOPPED",
                [OBS_FRONTEND_EVENT_SCENE_CHANGED] = "SCENE_CHANGED",
                [OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED] = "SCENE_LIST_CHANGED",
                [OBS_FRONTEND_EVENT_TRANSITION_CHANGED] = "TRANSITION_CHANGED",
                [OBS_FRONTEND_EVENT_TRANSITION_STOPPED] = "TRANSITION_STOPPED",
                [OBS_FRONTEND_EVENT_TRANSITION_LIST_CHANGED] = "TRANSITION_LIST_CHANGED",
                [OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED] = "SCENE_COLLECTION_CHANGED",
                [OBS_FRONTEND_EVENT_SCENE_COLLECTION_LIST_CHANGED] = "SCENE_COLLECTION_LIST_CHANGED",
                [OBS_FRONTEND_EVENT_PROFILE_CHANGED] = "PROFILE_CHANGED",
                [OBS_FRONTEND_EVENT_PROFILE_LIST_CHANGED] = "PROFILE_LIST_CHANGED",
                [OBS_FRONTEND_EVENT_EXIT] = "EXIT",
                [OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTING] = "REPLAY_BUFFER_STARTING",
                [OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED] = "REPLAY_BUFFER_STARTED",
                [OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPING] = "REPLAY_BUFFER_STOPPING",
                [OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED] = "REPLAY_BUFFER_STOPPED",
                [OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED] = "STUDIO_MODE_ENABLED",
                [OBS_FRONTEND_EVENT_STUDIO_MODE_DISABLED] = "STUDIO_MODE_DISABLED",
                [OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED] = "PREVIEW_SCENE_CHANGED",
                [OBS_FRONTEND_EVENT_SCENE_COLLECTION_CLEANUP] = "SCENE_COLLECTION_CLEANUP",
                [OBS_FRONTEND_EVENT_FINISHED_LOADING] = "FINISHED_LOADING",
                [OBS_FRONTEND_EVENT_RECORDING_PAUSED] = "RECORDING_PAUSED",
                [OBS_FRONTEND_EVENT_RECORDING_UNPAUSED] = "RECORDING_UNPAUSED",
                [OBS_FRONTEND_EVENT_TRANSITION_DURATION_CHANGED] = "TRANSITION_DURATION_CHANGED",
                [OBS_FRONTEND_EVENT_REPLAY_BUFFER_SAVED] = "REPLAY_BUFFER_SAVED",
                [OBS_FRONTEND_EVENT_VIRTUALCAM_STARTED] = "VIRTUALCAM_STARTED",
                [OBS_FRONTEND_EVENT_VIRTUALCAM_STOPPED] = "VIRTUALCAM_STOPPED",
                [OBS_FRONTEND_EVENT_TBAR_VALUE_CHANGED] = "TBAR_VALUE_CHANGED",
                [OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGING] = "SCENE_COLLECTION_CHANGING",
                [OBS_FRONTEND_EVENT_PROFILE_CHANGING] = "PROFILE_CHANGING",
                [OBS_FRONTEND_EVENT_SCRIPTING_SHUTDOWN] = "SCRIPTING_SHUTDOWN",
};

#define OBS_FRONTEND_EVENT_STR_LEN (sizeof(obs_frontend_event_str) / sizeof(obs_frontend_event_str[0]))

static void obs_event(enum obs_frontend_event event, void *private_data)
{
    (void)private_data;
    const char *event_str = "UNKNOW EVENT";
    if (event < OBS_FRONTEND_EVENT_STR_LEN)
        event_str = obs_frontend_event_str[event];
    mlog(LOG_INFO, "event: %s", event_str);

    if (event == OBS_FRONTEND_EVENT_RECORDING_STOPPED) {
        obs_output_t *recording_output = obs_frontend_get_recording_output();
        if (recording_output != NULL) {
            obs_data_t *settings = obs_output_get_settings(recording_output);
            if (settings != NULL) {
                if (NudgisConfig::GetCurrentNudgisConfig()->publish_recording_automatically->type != AutoState::AUTOSTATE_NEVER)
                {
                    const char *path = obs_data_get_string(settings, "path");
                    mlog(LOG_INFO, "path: %s", path);
                    //nudgis_upload_file(path);

                    obs_frontend_push_ui_translation(obs_module_get_string);
                    NudgisUploadUi nudgis_upload_ui((QWidget *)obs_frontend_get_main_window(), path);
                    obs_frontend_pop_ui_translation();
                    nudgis_upload_ui.exec();

                    mlog(LOG_INFO, "nudgis_upload_ui->show pass");
                }

                obs_data_release(settings);
            }
            obs_output_release(recording_output);
        }
    }
}

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

    obs_frontend_add_event_callback(obs_event, nullptr);

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

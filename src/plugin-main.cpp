/*
Nudgis-OBS-Plugin
Copyright (C) 2021 Ubicast 
*/
 
#include <obs-module.h>
#include "plugin-macros.generated.h"
#include "plugin-main.hpp"
#include "ui_settings.h"

#include <QMainWindow>
#include <QApplication>
#include <QAction>
#include <QtWidgets>
#include <iostream>

extern "C"
{
    EXPORT void *obs_frontend_get_main_window(void);
    EXPORT void *obs_frontend_add_tools_menu_qaction(const char *name);
    enum obs_frontend_event 
    {
        OBS_FRONTEND_EVENT_STREAMING_STARTING,
        OBS_FRONTEND_EVENT_STREAMING_STARTED,
        OBS_FRONTEND_EVENT_STREAMING_STOPPING,
        OBS_FRONTEND_EVENT_STREAMING_STOPPED,
        OBS_FRONTEND_EVENT_RECORDING_STARTING,
        OBS_FRONTEND_EVENT_RECORDING_STARTED,
        OBS_FRONTEND_EVENT_RECORDING_STOPPING,
        OBS_FRONTEND_EVENT_RECORDING_STOPPED,
        OBS_FRONTEND_EVENT_SCENE_CHANGED,
        OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED,
        OBS_FRONTEND_EVENT_TRANSITION_CHANGED,
        OBS_FRONTEND_EVENT_TRANSITION_STOPPED,
        OBS_FRONTEND_EVENT_TRANSITION_LIST_CHANGED,
        OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED,
        OBS_FRONTEND_EVENT_SCENE_COLLECTION_LIST_CHANGED,
        OBS_FRONTEND_EVENT_PROFILE_CHANGED,
        OBS_FRONTEND_EVENT_PROFILE_LIST_CHANGED,
        OBS_FRONTEND_EVENT_EXIT,

        OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTING,
        OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED,
        OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPING,
        OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED,

        OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED,
        OBS_FRONTEND_EVENT_STUDIO_MODE_DISABLED,
        OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED,

        OBS_FRONTEND_EVENT_SCENE_COLLECTION_CLEANUP,
        OBS_FRONTEND_EVENT_FINISHED_LOADING,

        OBS_FRONTEND_EVENT_RECORDING_PAUSED,
        OBS_FRONTEND_EVENT_RECORDING_UNPAUSED,

        OBS_FRONTEND_EVENT_TRANSITION_DURATION_CHANGED,
        OBS_FRONTEND_EVENT_REPLAY_BUFFER_SAVED,

        OBS_FRONTEND_EVENT_VIRTUALCAM_STARTED,
        OBS_FRONTEND_EVENT_VIRTUALCAM_STOPPED,

        OBS_FRONTEND_EVENT_TBAR_VALUE_CHANGED,
    };
    typedef void (*obs_frontend_event_cb)(enum obs_frontend_event event,
				      void *private_data);
    EXPORT void *obs_frontend_add_event_callback(obs_frontend_event_cb callback,void *private_data);
}

/*NUDGIS SETTINGS METHODS DEFINITION*/
/*-----------------------------------------------------------------------------------------------*/
static NudgisSettings *settingsWindow = nullptr;

NudgisSettings::NudgisSettings(): QWidget(nullptr), ui(new Ui_Settings)
{
    blog(LOG_INFO, "Plugin Settings Opened (version %s)", PLUGIN_VERSION);
    ui->setupUi(this);

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
        settingsWindow->resize(420, 340);
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

bool obs_module_load()
{
    blog(LOG_INFO, "Nudgis plugin loaded successfully (version %s)", PLUGIN_VERSION);
    QAction * menu_action = (QAction*) obs_frontend_add_tools_menu_qaction("Nudgis Plugin Settings");
    blog(LOG_INFO, "[%s] Menu entry for Settings added", obs_module_name());
    menu_action->connect(menu_action, &QAction::triggered, openWindow);
    obs_frontend_add_event_callback(obs_event, nullptr);
    return true;
}

void obs_module_unload()
{
    blog(LOG_INFO, "Nudgis Plugin Successfully Unloaded");
}

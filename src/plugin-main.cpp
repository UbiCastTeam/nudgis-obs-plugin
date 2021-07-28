/*
Nudgis-OBS-Plugin
Copyright (C) 2021 Ubicast 
*/
 
#include <obs-module.h>
#include "plugin-macros.generated.h"
#include "plugin-main.hpp"
#include <QMainWindow>
#include <QApplication>
#include <QAction>
#include <QtWidgets>
#include <iostream>


/*NUDGIS SETTINGS METHODS DEFINITION*/
/*-----------------------------------------------------------------------------------------------*/
static NudgisSettings *settingsWindow = nullptr;

NudgisSettings::NudgisSettings() : QWidget(nullptr)
{
    blog(LOG_INFO, "Plugin Settings Opened (version %s)", PLUGIN_VERSION);
}

NudgisSettings::~NudgisSettings() 
{

}


/*HANDLING OBS EVENTS*/
/*-----------------------------------------------------------------------------------------------*/

/*static void obs_event(enum obs_frontend_event event, void *)
{
	if (event == OBS_FRONTEND_EVENT_EXIT) 
    {
		delete settingsWindow;
	} 
    else if (event == OBS_FRONTEND_EVENT_SCENE_COLLECTION_CLEANUP) 
    {
        
	}
}*/

/*LOAD AND UNLOAD OF THE PLUGIN*/
/*-----------------------------------------------------------------------------------------------*/

extern "C"
{
    EXPORT void *obs_frontend_get_main_window(void);
    EXPORT void *obs_frontend_add_tools_menu_qaction(const char *name);
}

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

const char *obs_module_name(void)
{
	return "Nudgis Obs Plugin";
}

void openWindow()
{
    blog(LOG_INFO, "Plugin Settings Opened (version %s)", PLUGIN_VERSION);
    /*if (!settingsWindow) 
    {
        settingsWindow = new NudgisSettings();
        settingsWindow->resize(320, 240);
        settingsWindow->show();
        settingsWindow->setWindowTitle(QApplication::translate("toplevel", "Nudgis Plugin Settings"));
	} 
    else 
    {
        settingsWindow->show();
        settingsWindow->setWindowTitle(QApplication::translate("toplevel", "Nudgis Plugin Settings"));
        settingsWindow->raise();
	}*/
}

bool obs_module_load()
{

    //std::cout << "Hello World 2, This is my CPP OBS plugin!\n";
    blog(LOG_INFO, "Nudgis plugin loaded successfully (version %s)", PLUGIN_VERSION);
    QAction * menu_action = (QAction*) obs_frontend_add_tools_menu_qaction("Nudgis Plugin Settings");
    blog(LOG_INFO, "[%s] Menu entry for Settings added", obs_module_name());
    menu_action->connect(menu_action, &QAction::triggered, openWindow);
    //obs_frontend_add_event_callback(obs_event, nullptr);
    return true;
}



void obs_module_unload()
{
    blog(LOG_INFO, "plugin unloaded");
}

/*
Nudgis-OBS-Plugin
Copyright (C) 2021 Ubicast 
*/
 
#include <obs-module.h>
#include "plugin-macros.generated.h"
#include "plugin-main.hpp"
#include <QMainWindow>
#include <QAction>
#include <QtWidgets>
#include <iostream>

/*-----------------------------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------------------------*/

NudgisSettings::NudgisSettings() : QWidget()
{
    /*blog(LOG_INFO, "Plugin Settings Opened (version %s)", PLUGIN_VERSION);
    QWidget * window = new QWidget();
    window->resize(320, 240);
    window->show();
    window->setWindowTitle(QApplication::translate("toplevel", "Nudgis Plugin Settings"));*/
}

NudgisSettings::~NudgisSettings() 
{

}

void NudgisSettings::openSettings() 
{

}

void NudgisSettings::closeSettings() 
{

}

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
    QWidget * window = new QWidget();
    window->resize(320, 240);
    window->show();
    window->setWindowTitle(QApplication::translate("toplevel", "Nudgis Plugin Settings"));
}
bool obs_module_load()
{

    //std::cout << "Hello World 2, This is my CPP OBS plugin!\n";
    blog(LOG_INFO, "Nudgis plugin loaded successfully (version %s)", PLUGIN_VERSION);
    QAction * menu_action = (QAction*) obs_frontend_add_tools_menu_qaction("Nudgis Plugin Settings");
    blog(LOG_INFO, "[%s] Menu entry for Settings added", obs_module_name());
    menu_action->connect(menu_action, &QAction::triggered, openWindow);
    return true;
}



void obs_module_unload()
{
    blog(LOG_INFO, "plugin unloaded");
}

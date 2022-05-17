#ifndef NUDGISSETTINGS_H
#define NUDGISSETTINGS_H

#include "nudgis-config.hpp"

#include <QWidget>
#include <QLineEdit>
#include <obs-frontend-api.h>

class Ui_Settings;

class NudgisSettings : public QWidget {
    Q_OBJECT
    Ui_Settings *ui;

public:
    NudgisSettings();
    ~NudgisSettings();

private:
    void loadSettings();
    void loadSettings(const NudgisConfig *nudgis_config);
    void Set_sw_EchoMode(QLineEdit::EchoMode mode);
    NudgisConfig &to_NudgisConfig();

protected:
    void showEvent(QShowEvent *event);

private slots:
    void on_btn_DefaultReset_clicked();
    void on_btn_saveSettings_clicked();
    void on_btn_sw_EchoMode_clicked();
    void on_btn_test_clicked();
};

#endif // NUDGISSETTINGS_H

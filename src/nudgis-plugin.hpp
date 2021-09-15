#ifndef NUDGISSETTINGS_H
#define NUDGISSETTINGS_H

#include <QWidget>
#include <obs-frontend-api.h>

class Ui_Settings;

class NudgisSettings : public QWidget {
    Q_OBJECT
    Ui_Settings *ui;

public:
    NudgisSettings();

private:
    static void obsFrontendEvent(enum obs_frontend_event event, void *private_data);
    void refreshBtnStreamDisabled();
    void clearWindow();
    void saveSettings();
    void loadSettings();
    void stream();

protected:
    void showEvent(QShowEvent *event);
};

#endif // NUDGISSETTINGS_H

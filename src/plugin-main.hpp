#ifndef NUDGISSETTINGS_H
#define NUDGISSETTINGS_H

#include <QWidget>

class Ui_Settings;

class NudgisSettings : public QWidget
{
    Q_OBJECT
    Ui_Settings * ui;

public:
    NudgisSettings();
private:
    void clearWindow();
    void box();
    bool upload = false;

signals:

};

#endif // NUDGISSETTINGS_H





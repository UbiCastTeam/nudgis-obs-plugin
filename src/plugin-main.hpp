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
    void saveSettings();
    void box();
    std::string removeQuotes(std::string str);

    void uploadVod();
    bool upload = false;
    std::string apiKey;
    std::string url;
    std::string streamTitle;
    std::string vodTitle;
    std::string streamChannel;
    std::string vodChannel;  

signals:    

};

#endif // NUDGISSETTINGS_H





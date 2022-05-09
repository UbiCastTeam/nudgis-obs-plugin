#ifndef NUDGIS_UPLOAD_UI_HPP
#define NUDGIS_UPLOAD_UI_HPP

#include <QtCore>
#include <QDialog>

#include "nudgis-service.hpp"
#include "nudgis-config.hpp"

class NudgisUploadUi;

class NudgisUploadThead : public QThread {
    Q_OBJECT

    public:
        NudgisUploadThead(NudgisUpload * nudgis_upload);

    private:
        NudgisUpload * nudgis_upload;
        void run();
        
    signals:
        void startUpload();
        void endUpload(NudgisUploadFileResult *result);
        void progressUpload(int percent);
};

class Ui_NudgisUpload;

class NudgisUploadUi : public QDialog {
    Q_OBJECT

public:
    NudgisUploadUi(QWidget *parent = Q_NULLPTR, const char *fileName = Q_NULLPTR);
    ~NudgisUploadUi();

private:
    enum NUDGIS_UPLOAD_UI_STATE
    {
        NUDGIS_UPLOAD_UI_ASK_UPLOAD_FILE,
        NUDGIS_UPLOAD_UI_UPLOAD_FILE_PROGRESS,
        NUDGIS_UPLOAD_UI_UPLOAD_FILE_DONE,
        NUDGIS_UPLOAD_UI_ASK_REMOVE_FILE,
        NUDGIS_UPLOAD_UI_UNKNOW,
    };

    Ui_NudgisUpload *ui;
    NudgisUploadThead *nudgis_upload_thead;
    NudgisUpload nudgis_upload;
    const char *fileName;
    const NudgisConfig *nudgis_config;
    enum NUDGIS_UPLOAD_UI_STATE currentState = NUDGIS_UPLOAD_UI_UNKNOW;

    void updateState(NUDGIS_UPLOAD_UI_STATE state);
    void manageUploadFile(AutoState::Types auto_state);
    void manageDeleteUploadedFile(AutoState::Types auto_state);
    void updateLabelsTemplate();

private slots:
    void on_pushButton_Yes_UploadFile_clicked();
    void on_pushButton_No_UploadFile_clicked();
    void on_pushButton_Yes_RemoveFile_clicked();
    void on_pushButton_No_RemoveFile_clicked();
    void on_pushButton_Cancel_clicked();
    void on_pushButton_Done_clicked();

    void on_startUpload();
    void on_endUpload(NudgisUploadFileResult *result);
    void on_progressUpload(int percent);
};

#endif //NUDGIS_UPLOAD_UI_HPP

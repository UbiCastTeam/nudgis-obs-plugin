#include "nudgis-upload-ui.hpp"
#include "ui_nudgis-upload.h"
#include "plugin-macros.generated.h"

#include <obs-module.h>

static void NudgisUploadProgress(void *cb_args,int percent)
{
    NudgisUploadThead *nudgis_upload_thead = (NudgisUploadThead *)cb_args;
    mlog(LOG_INFO,"percent: %d",percent);
    emit nudgis_upload_thead->progressUpload(percent);
}

NudgisUploadThead::NudgisUploadThead(const char * fileName):
    QThread()
{
    this->canceled = false;
    //this->fileName = fileName;
    this->fileName = "/home/pagou/Vidéos/test.mkv";
}

void NudgisUploadThead::cancel()
{
    this->canceled = true;
}

void NudgisUploadThead::run()
{
    emit this->startUpload();
    NudgisUploadFileResult *result = nudgis_upload_file(this->fileName, NudgisUploadProgress, this);
    emit this->endUpload(!this->canceled, result);
}

NudgisUploadUi::NudgisUploadUi(QWidget *parent, const char *fileName)
        : QDialog(parent), ui(new Ui_NudgisUpload)
{
    this->nudgis_config = NudgisConfig::GetCurrentNudgisConfig();
    this->fileName = fileName;
    this->nudgis_upload_thead = new NudgisUploadThead(this->fileName);
    connect(this->nudgis_upload_thead, SIGNAL(startUpload()), this, SLOT(on_startUpload()));
    connect(this->nudgis_upload_thead, SIGNAL(endUpload(bool, NudgisUploadFileResult*)), this, SLOT(on_endUpload(bool, NudgisUploadFileResult*)));
    connect(this->nudgis_upload_thead, SIGNAL(progressUpload(int)), this, SLOT(on_progressUpload(int)));
    ui->setupUi(this);
    this->manageUploadFile(this->nudgis_config->publish_recording_automatically->type);
}

NudgisUploadUi::~NudgisUploadUi()
{
    delete this->nudgis_upload_thead;
}

void NudgisUploadUi::on_startUpload()
{
    this->updateState(NUDGIS_UPLOAD_UI_UPLOAD_FILE_PROGRESS);
}

void NudgisUploadUi::on_endUpload(bool is_done, NudgisUploadFileResult *result)
{
    this->nudgis_upload_thead->wait();
    if (is_done)
    {
        this->ui->label_FileUploadedUrl->setText(this->ui->label_FileUploadedUrl->text().replace("$$NUDGIS_URL$$", this->nudgis_config->url.c_str()));
        this->ui->label_FileUploadedUrl->setText(this->ui->label_FileUploadedUrl->text().replace("$$OID$$", obs_data_get_string(result->media_add_response, "oid")));
        this->updateState(NUDGIS_UPLOAD_UI_UPLOAD_FILE_DONE);
    }
    else
        this->on_pushButton_Done_clicked();

    delete result;
}

void NudgisUploadUi::on_progressUpload(int percent)
{
    this->ui->progressBar_UploadFile->setValue(percent);
}

void NudgisUploadUi::on_pushButton_Yes_UploadFile_clicked()
{
    this->manageUploadFile(AutoState::AUTOSTATE_YES);
}

void NudgisUploadUi::on_pushButton_No_UploadFile_clicked()
{
    this->manageUploadFile(AutoState::AUTOSTATE_NEVER);
}

void NudgisUploadUi::on_pushButton_Yes_RemoveFile_clicked()
{
    this->manageDeleteUploadedFile(AutoState::AUTOSTATE_YES);
}

void NudgisUploadUi::on_pushButton_No_RemoveFile_clicked()
{
    this->manageDeleteUploadedFile(AutoState::AUTOSTATE_NEVER);
}

void NudgisUploadUi::on_pushButton_Cancel_clicked()
{
    this->nudgis_upload_thead->cancel();
}

void NudgisUploadUi::on_pushButton_Done_clicked()
{
    this->manageDeleteUploadedFile(this->nudgis_config->auto_delete_uploaded_file->type);
}

void NudgisUploadUi::manageUploadFile(AutoState::Types auto_state)
{
    if (auto_state == AutoState::AUTOSTATE_ASK)
    {
        this->updateState(NUDGIS_UPLOAD_UI_ASK_UPLOAD_FILE);
    }
    else
    {
        if (auto_state == AutoState::AUTOSTATE_YES)
        {
            this->nudgis_upload_thead->start();
        }
        else
            this->on_pushButton_Done_clicked();
    }
}

void NudgisUploadUi::manageDeleteUploadedFile(AutoState::Types auto_state)
{
    if (auto_state == AutoState::AUTOSTATE_ASK)
    {
        this->updateState(NUDGIS_UPLOAD_UI_ASK_REMOVE_FILE);
    }
    else
    {
        if (auto_state == AutoState::AUTOSTATE_YES)
            QFile(this->fileName).remove();
        this->close();
    }
}

void NudgisUploadUi::updateState(NUDGIS_UPLOAD_UI_STATE state)
{
    QWidget * visible_widget = NULL;
    QWidget * widgets[] = { ui->widget_AskUploadFile, 
                            ui->widget_UploadFileProgress, 
                            ui->widget_UploadFileDone, 
                            ui->widget_AskRemoveFile, };

    if (state == NUDGIS_UPLOAD_UI_ASK_UPLOAD_FILE)
    {
        visible_widget = ui->widget_AskUploadFile;
    }
    else if (state == NUDGIS_UPLOAD_UI_UPLOAD_FILE_PROGRESS)
    {
        visible_widget = ui->widget_UploadFileProgress;
    }
    else if (state == NUDGIS_UPLOAD_UI_UPLOAD_FILE_DONE)
    {
        visible_widget = ui->widget_UploadFileDone;
    }
    else if (state == NUDGIS_UPLOAD_UI_ASK_REMOVE_FILE)
    {
        visible_widget = ui->widget_AskRemoveFile;
    }

    if (visible_widget != NULL)
    {
        for (QWidget *widget : widgets)
            widget->setVisible(widget == visible_widget);
    }

    this->currentState = state;
}

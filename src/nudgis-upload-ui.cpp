#include "nudgis-upload-ui.hpp"
#include "ui_nudgis-upload.h"
#include "plugin-macros.generated.h"

#include <obs-module.h>

NudgisUploadThead::NudgisUploadThead(NudgisUpload * nudgis_upload):
    QThread()
{
    this->nudgis_upload = nudgis_upload;
}

void NudgisUploadThead::run()
{
    this->nudgis_upload->run();
}

NudgisUploadUi::NudgisUploadUi(QWidget *parent, const char *fileName)
        : QDialog(parent), ui(new Ui_NudgisUpload), nudgis_upload(fileName)
{
    this->nudgis_config = NudgisConfig::GetCurrentNudgisConfig();
    this->fileName = fileName;
    this->nudgis_upload_thead = new NudgisUploadThead(&this->nudgis_upload);
    connect(&this->nudgis_upload, SIGNAL(progressUpload(int)), this, SLOT(on_progressUpload(int)));
    connect(&this->nudgis_upload, SIGNAL(endUpload()), this, SLOT(on_endUpload()));
    ui->setupUi(this);
    this->updateLabelsTemplate();
    this->manageUploadFile(this->nudgis_config->publish_recording_automatically->type);
}

NudgisUploadUi::~NudgisUploadUi()
{
    delete this->nudgis_upload_thead;
}

void NudgisUploadUi::updateFileUploadedUrl()
{
    this->ui->label_FileUploadedUrl->setText(this->nudgis_upload.GetFileUploadedUrlHtml());
}

void NudgisUploadUi::on_endUpload()
{
    this->nudgis_upload_thead->wait();
    if (this->nudgis_upload.GetState() == NudgisUpload::NUDGIS_UPLOAD_STATE_UPLOAD_SUCESSFULL)
    {
        this->updateFileUploadedUrl();
        this->updateState(NUDGIS_UPLOAD_UI_UPLOAD_FILE_DONE);
    }
    else
        this->on_pushButton_Done_clicked();
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
    this->ui->pushButton_Cancel->setEnabled(false);
    this->nudgis_upload.cancel();
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
            this->updateState(NUDGIS_UPLOAD_UI_UPLOAD_FILE_PROGRESS);
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

void NudgisUploadUi::updateLabelsTemplate()
{
    QLabel *updated_labels[] = { ui->label_AskUploadFile,
                                    ui->label_UploadFileProgress, };

    for (QLabel *updated_label: updated_labels)
    {
        updated_label->setText(updated_label->text().replace("$$NUDGIS_URL$$", this->nudgis_config->url.c_str()));
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

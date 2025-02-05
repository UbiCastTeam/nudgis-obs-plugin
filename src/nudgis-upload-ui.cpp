#include "nudgis-upload-ui.hpp"
#include "ui_nudgis-upload.h"
#include "plugin-support.h"
#include "obs-app.hpp"

#include <obs-module.h>

NudgisUploadThead::NudgisUploadThead(NudgisUpload *nudgis_upload) : QThread()
{
	this->nudgis_upload = nudgis_upload;
}

void NudgisUploadThead::run()
{
	this->nudgis_upload->run();
}

NudgisUploadUi::NudgisUploadUi(QWidget *parent, const char *fileName)
	: QDialog(parent),
	  ui(new Ui_NudgisUpload),
	  nudgis_upload(fileName)
{
	this->ui->setupUi(this);
	this->nudgis_config = NudgisConfig::GetCurrentNudgisConfig();
	this->fileName = fileName;
	this->updateLabelsTemplate();

	if (this->fileName != NULL) {
		this->nudgis_upload_thead = new NudgisUploadThead(&this->nudgis_upload);
		connect(&this->nudgis_upload, SIGNAL(progressUpload(int)), this, SLOT(on_progressUpload(int)));
		connect(&this->nudgis_upload, SIGNAL(endUpload()), this, SLOT(on_endUpload()));
		this->manageUploadFile(this->nudgis_config->publish_recording_automatically->type);
	}
}

NudgisUploadUi::~NudgisUploadUi()
{
	if (this->nudgis_upload_thead != NULL)
		delete this->nudgis_upload_thead;
	delete this->ui;
}

void NudgisUploadUi::updateFileUploadedUrl()
{
	this->ui->label_FileUploadedUrl->setText(this->nudgis_upload.GetFileUploadedUrlHtml());
}

void NudgisUploadUi::updateError(const HttpClientError *http_client_error)
{
	this->ui->value_Error_Code->setText(QString::number(http_client_error->curl_code));
	this->ui->value_Error_HttpCode->setText(QString::number(http_client_error->http_code));
	this->ui->value_Error_Url->setText(http_client_error->url.c_str());
	this->ui->value_Error_Message->setText(http_client_error->error.c_str());
}

void NudgisUploadUi::updateError()
{
	this->updateError(this->nudgis_upload.GetHttpClientError());
}

void NudgisUploadUi::on_endUpload()
{
	this->nudgis_upload_thead->wait();
	if (this->nudgis_upload.GetState() == NudgisUpload::NUDGIS_UPLOAD_STATE_UPLOAD_SUCESSFULL) {
		this->updateFileUploadedUrl();
		this->updateState(NUDGIS_UPLOAD_UI_UPLOAD_FILE_DONE);
	} else if (this->nudgis_upload.GetState() == NudgisUpload::NUDGIS_UPLOAD_STATE_UPLOAD_FAILED) {
		this->updateError();
		this->updateState(NUDGIS_UPLOAD_UI_ERROR);
	} else
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

void NudgisUploadUi::on_pushButton_Error_Done_clicked()
{
	this->on_pushButton_Done_clicked();
}

void NudgisUploadUi::manageUploadFile(AutoState::Types auto_state)
{
	if (auto_state == AutoState::AUTOSTATE_ASK) {
		this->updateState(NUDGIS_UPLOAD_UI_ASK_UPLOAD_FILE);
	} else {
		if (auto_state == AutoState::AUTOSTATE_YES) {
			this->nudgis_upload_thead->start();
			this->updateState(NUDGIS_UPLOAD_UI_UPLOAD_FILE_PROGRESS);
		} else
			this->on_pushButton_Done_clicked();
	}
}

void NudgisUploadUi::manageDeleteUploadedFile(AutoState::Types auto_state)
{
	if (auto_state == AutoState::AUTOSTATE_ASK) {
		this->updateState(NUDGIS_UPLOAD_UI_ASK_REMOVE_FILE);
	} else {
		if (auto_state == AutoState::AUTOSTATE_YES)
			QFile(this->fileName).remove();
		this->close();
	}
}

void NudgisUploadUi::updateLabelsTemplate()
{
	QLabel *updated_labels[] = {
		ui->label_AskUploadFile,
		ui->label_UploadFileProgress,
	};

	for (QLabel *updated_label : updated_labels) {
		updated_label->setText(
			updated_label->text().replace("$$NUDGIS_URL$$", this->nudgis_config->url.c_str()));
	}
}

void NudgisUploadUi::updateState(NUDGIS_UPLOAD_UI_STATE state)
{
	QWidget *visible_widget = NULL;
	QWidget *widgets[] = {
		ui->widget_AskUploadFile,  ui->widget_UploadFileProgress,
		ui->widget_UploadFileDone, ui->widget_AskRemoveFile,
		ui->widget_Error,          ui->widget_TestParamsDone,
	};

	if (state == NUDGIS_UPLOAD_UI_ASK_UPLOAD_FILE) {
		visible_widget = ui->widget_AskUploadFile;
	} else if (state == NUDGIS_UPLOAD_UI_UPLOAD_FILE_PROGRESS) {
		visible_widget = ui->widget_UploadFileProgress;
	} else if (state == NUDGIS_UPLOAD_UI_UPLOAD_FILE_DONE) {
		visible_widget = ui->widget_UploadFileDone;
	} else if (state == NUDGIS_UPLOAD_UI_ASK_REMOVE_FILE) {
		visible_widget = ui->widget_AskRemoveFile;
	} else if (state == NUDGIS_UPLOAD_UI_ERROR) {
		visible_widget = ui->widget_Error;
	} else if (state == NUDGIS_UPLOAD_UI_TEST_PARAMS_DONE) {
		visible_widget = ui->widget_TestParamsDone;
	}

	if (visible_widget != NULL) {
		for (QWidget *widget : widgets)
			widget->setVisible(widget == visible_widget);
	}

	this->currentState = state;
}

NudgisTestParamsUi::NudgisTestParamsUi(QWidget *parent, const NudgisTestParamsData *test_params_data,
				       const char *windowTitle)
	: NudgisUploadUi(parent)
{
	this->setWindowTitle(windowTitle);

	if (test_params_data != NULL &&
	    test_params_data->result == NudgisTestParamsData::NUDGISDATA_LIVE_TEST_RESULT_FAILED) {
		this->updateError(test_params_data->http_client_error);
		this->updateState(NUDGIS_UPLOAD_UI_ERROR);
	} else
		this->updateState(NUDGIS_UPLOAD_UI_TEST_PARAMS_DONE);
}

void NudgisTestParamsUi::on_pushButton_Error_Done_clicked()
{
	this->close();
}

void NudgisTestParamsUi::on_pushButton_TestParamsDone_clicked()
{
	this->on_pushButton_Error_Done_clicked();
}

#ifndef NUDGIS_UPLOAD_UI_HPP
#define NUDGIS_UPLOAD_UI_HPP

#include <QtCore>
#include <QDialog>

#include "nudgis-service.hpp"
#include "nudgis-config.hpp"

class NudgisUploadThead : public QThread {
	Q_OBJECT

public:
	NudgisUploadThead(NudgisUpload *nudgis_upload);

private:
	NudgisUpload *nudgis_upload;
	void run();
};

class Ui_NudgisUpload;

class NudgisUploadUi : public QDialog {
	Q_OBJECT

public:
	NudgisUploadUi(QWidget *parent = Q_NULLPTR,
		       const char *fileName = Q_NULLPTR);
	~NudgisUploadUi();

protected:
	enum NUDGIS_UPLOAD_UI_STATE {
		NUDGIS_UPLOAD_UI_ASK_UPLOAD_FILE,
		NUDGIS_UPLOAD_UI_UPLOAD_FILE_PROGRESS,
		NUDGIS_UPLOAD_UI_UPLOAD_FILE_DONE,
		NUDGIS_UPLOAD_UI_ASK_REMOVE_FILE,
		NUDGIS_UPLOAD_UI_ERROR,
		NUDGIS_UPLOAD_UI_TEST_PARAMS_DONE,
		NUDGIS_UPLOAD_UI_UNKNOW,
	};

	void updateState(NUDGIS_UPLOAD_UI_STATE state);
	void updateError(const HttpClientError *http_client_error);

private:
	Ui_NudgisUpload *ui;
	NudgisUploadThead *nudgis_upload_thead = NULL;
	NudgisUpload nudgis_upload;
	const char *fileName = NULL;
	const NudgisConfig *nudgis_config = NULL;
	enum NUDGIS_UPLOAD_UI_STATE currentState = NUDGIS_UPLOAD_UI_UNKNOW;

	void manageUploadFile(AutoState::Types auto_state);
	void manageDeleteUploadedFile(AutoState::Types auto_state);
	void updateLabelsTemplate();
	void updateFileUploadedUrl();
	void updateError();

private slots:
	void on_pushButton_Yes_UploadFile_clicked();
	void on_pushButton_No_UploadFile_clicked();
	void on_pushButton_Yes_RemoveFile_clicked();
	void on_pushButton_No_RemoveFile_clicked();
	void on_pushButton_Cancel_clicked();
	void on_pushButton_Done_clicked();
	virtual void on_pushButton_Error_Done_clicked();
	virtual void on_pushButton_TestParamsDone_clicked(){};

	void on_endUpload();
	void on_progressUpload(int percent);
};

class NudgisTestParamsUi : public NudgisUploadUi {
	Q_OBJECT

public:
	NudgisTestParamsUi(QWidget *parent,
			   const NudgisTestParamsData *test_params_data,
			   const char *windowTitle);

private slots:
	void on_pushButton_Error_Done_clicked() Q_DECL_OVERRIDE;
	void on_pushButton_TestParamsDone_clicked() Q_DECL_OVERRIDE;
};

#endif //NUDGIS_UPLOAD_UI_HPP

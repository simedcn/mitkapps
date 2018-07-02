#ifndef QTUPLOADFILES_H
#define QTUPLOADFILES_H

#include <QtWidgets/QMainWindow>
#include "ui_qtuploadfiles.h"

class QTUploadFiles : public QMainWindow
{
	Q_OBJECT

public:
	QTUploadFiles(QWidget *parent = 0);
	~QTUploadFiles();

private:
	Ui::QTUploadFilesClass ui;
};

#endif // QTUPLOADFILES_H

#ifndef DICOMVIEWDIALOG_H
#define DICOMVIEWDIALOG_H

#include <QObject>
#include "DicomView.h"
#include <QDialog>
class DicomViewDialog : public QDialog
{

    Q_OBJECT


public:
    DicomViewDialog(QWidget * parent);


private:
    DicomView * dicomView;
};

#endif // DICOMVIEWDIALOG_H

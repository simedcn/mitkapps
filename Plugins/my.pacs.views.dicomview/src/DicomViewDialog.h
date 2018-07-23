#ifndef DICOMVIEWDIALOG_H
#define DICOMVIEWDIALOG_H

#include <QObject>
#include <internal/DicomView.h>
#include <QDialog>


class DicomViewDialog : public QDialog
{
Q_OBJECT
public:
    DicomViewDialog(QWidget * parent);
    ~DicomViewDialog();

    static void execute(QWidget * parent);
private:
    DicomView * dicomView;
};

#endif // DICOMVIEWDIALOG_H

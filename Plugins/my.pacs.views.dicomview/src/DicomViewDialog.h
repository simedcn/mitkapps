#ifndef DICOMVIEWDIALOG_H
#define DICOMVIEWDIALOG_H

#include <QObject>
#include <internal/DicomView.h>
#include <QDialog>
#include <my_pacs_views_dicomview_Export.h>

class MY_PACS_VIEWS_DICOMVIEW_EXPORT DicomViewDialog : public QDialog
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

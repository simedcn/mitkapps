#include "DicomViewDialog.h"
#include <QDialogButtonBox>


DicomViewDialog::DicomViewDialog(QWidget * parent = 0) : QDialog(parent) {
    dicomView = new DicomView();
    dicomView->CreateQtPartControl(this);
    layout()->addWidget(dicomView);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    layout()->addWidget(buttonBox);
    setModal(true);
    connect(dicomView,SIGNAL(addToDataManager()),this,SLOT(accept()));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
};

DicomViewDialog::~DicomViewDialog() {

};

void DicomViewDialog::execute(QWidget *parent) {
    DicomViewDialog * dialog = new DicomViewDialog(parent);
    dialog->exec();
}


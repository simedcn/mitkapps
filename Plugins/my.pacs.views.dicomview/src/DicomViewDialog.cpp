#include "DicomViewDialog.h"

DicomViewDialog::DicomViewDialog(QWidget * parent = 0) : QDialog(parent){
    dicomView = new DicomView();
    dicomView->CreateQtPartControl(parent);
    layout()->addWidget(dicomView);
};

DicomViewDialog::~DicomViewDialog() {

};

void DicomViewDialog::execute(QWidget *parent) {
    DicomViewDialog * dialog = new DicomViewDialog(parent);
    dialog->exec();

}


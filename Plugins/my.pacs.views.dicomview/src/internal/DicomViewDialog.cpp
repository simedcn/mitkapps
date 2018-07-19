#include "DicomViewDialog.h"

DicomViewDialog::DicomViewDialog(QWidget * parent = 0) {
    dicomView = new DicomView();
    dicomView->CreateQtPartControl(parent);
}

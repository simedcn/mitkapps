#include "DicomViewDialog.h"
#include <QDialogButtonBox>

#include <QGuiApplication>
#include <QScreen>
#include <QRect>


DicomViewDialog::DicomViewDialog(QWidget* parent = nullptr) : QDialog(parent)
{
    dicomView = new DicomView();
    dicomView->CreateQtPartControl(this);
    layout()->addWidget(dicomView);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    layout()->addWidget(buttonBox);
    setModal(true);
	//setWindowFlags(Qt::Window);
	showMaximized();
	//auto geo = geometry();
	//geo.setX(geo.x() + 10);
	//geo.setY(geo.y() + 10);
	//geo.setWidth(geo.width() - 20);
	//geo.setHeight(geo.height() - 20);
	//setGeometry(geo);
	auto flags = windowFlags();
	setWindowFlags(Qt::WindowFlags(flags & ~Qt::WindowContextHelpButtonHint));

    //connect(dicomView, SIGNAL(addToDataManager()), this, SLOT(accept()));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

DicomViewDialog::~DicomViewDialog()
{}

void DicomViewDialog::execute(QWidget* parent)
{
    DicomViewDialog * dialog = new DicomViewDialog(parent);
    dialog->exec();
}


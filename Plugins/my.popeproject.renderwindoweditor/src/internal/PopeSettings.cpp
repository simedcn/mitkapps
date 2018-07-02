
#include "ui_PopeSettings.h"

#include "PopeSettings.h"
#include <QString>
#include <QMessageBox>

#include <sstream>

PopeSettings::PopeSettings(QWidget* parent)
	: QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint),
	ui(*new Ui::popesettings)
{
	ui.setupUi(this);

	//this->setWindowTitle("Settings");
	this->setWindowFlags(this->windowFlags() | Qt::WindowCloseButtonHint);

	/// Costomize UI elements
	//ui.label_folder->setVisible(false);
	//ui.label_folderPath->setVisible(false);
}

PopeSettings::~PopeSettings()
{
	delete &ui;
}

void PopeSettings::on_pushButton_OK_clicked()
{
	accept();
}

void PopeSettings::on_pushButton_Cancel_clicked()
{
	reject();
}

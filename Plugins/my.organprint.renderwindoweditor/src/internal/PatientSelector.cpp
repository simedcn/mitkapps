
#include "ui_PatientSelector.h"
#include "PatientSelector.h"

#include <QString>
#include <QMessageBox>

#include <sstream>

PatientSelector::PatientSelector(QWidget* parent)
	: QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint),
	ui(*new Ui::patientselector)
{
	ui.setupUi(this);

	this->setWindowTitle("POPE: Select a patient ID");
	this->setWindowFlags(this->windowFlags() | Qt::WindowCloseButtonHint);

	/// Costomize UI elements
	ui.label_folder->setVisible(false);
	ui.label_folderPath->setVisible(false);

	// Set the item delegate to ui.listWidget_Images to hide the dotted border around selected cell
	ui.listWidget_Images->setItemDelegate(new NoFocusDelegate());
}

PatientSelector::~PatientSelector()
{
	delete &ui;
}

void PatientSelector::SetPatientData(const vector<shared_ptr<PatientDescription>>& patients)
{
	this->patients = patients;
	ui.tableWidget_PatientsIDs->setRowCount((int)patients.size());
	/// Fill out the table of patients
	for (int i = 0; i < patients.size(); i++)
	{
		QTableWidgetItem* item_1 = new QTableWidgetItem(patients[i]->id);
		ui.tableWidget_PatientsIDs->setItem(i, 0, item_1);
		QTableWidgetItem* item_2 = new QTableWidgetItem(patients[i]->name);
		ui.tableWidget_PatientsIDs->setItem(i, 1, item_2);
		item_1->setData(Qt::UserRole, i);
		QString hint_images = patients[i]->ImageNamesAsString();
		item_2->setToolTip(hint_images);
	}
}

void PatientSelector::SetFolder(const QString& folder)
{
	bool visible = !folder.isEmpty();
	ui.label_folder->setVisible(visible);
	ui.label_folderPath->setVisible(visible);
	if (visible)
		ui.label_folderPath->setText(folder);
}

int PatientSelector::SelectedPatientIDIndex()
{
	// Get selected items
	auto selected_items = ui.tableWidget_PatientsIDs->selectedItems();
	if (selected_items.size() == 0)
		return -1;

	int row = selected_items.front()->row();
	auto item = ui.tableWidget_PatientsIDs->item(row, 0);
	//auto it = selected_items.begin();
	//// Keep only the items of the first column
	//while (it != selected_items.end())
	//{
	//	const auto& item = *it;
	//	if (item->column() != 0)
	//		it = selected_items.erase(it);
	//	else
	//		++it;
	//}
	//if (selected_items.size() == 0)
	//	return -1;

	bool is_ok;
	int index = item->data(Qt::UserRole).toInt(&is_ok);
	if (is_ok)
		return index;
	else
		return -2;
}

void PatientSelector::on_tableWidget_PatientsIDs_itemSelectionChanged()
{
	bool enabled = (ui.tableWidget_PatientsIDs->selectedItems().size() > 0);
	ui.pushButton_OK->setEnabled(enabled);
	if (enabled)
		ui.pushButton_OK->setFocus();

	// Remove all old items
	//ui.listWidget_Images->clear();
	while (auto item = ui.listWidget_Images->takeItem(0))
	{
		//delete item;
	}

	if (!enabled)
		return;

	int index = SelectedPatientIDIndex();
	if (index < 0 && index >= patients.size())
		return;

	// Fill out the list of images of the current patient
	vector<QString>& imagesVector = patients[index]->images;
	//list<QString> imagesList{std::make_move_iterator(std::begin(imagesVector)), std::make_move_iterator(std::end(imagesVector))};
	QVector<QString> imagesQVector = QVector<QString>::fromStdVector(imagesVector);
	QStringList images = QStringList::fromVector(imagesQVector);
	ui.listWidget_Images->addItems(images);
}

void PatientSelector::on_pushButton_OK_clicked()
{
	accept();
}

void PatientSelector::on_pushButton_Cancel_clicked()
{
	reject();
}


// PatientDescription

PatientDescription::PatientDescription(const QString& id, const QString& name, const vector<QString>& images)
	: id(id), name(name), images(images)
{}

QString PatientDescription::ImageNamesAsString() const
{
	QString strImages;
	bool is_first = true;
	for (const auto& image : images)
	{
		if (is_first)
			is_first = false;
		else
			strImages += "\n";
		strImages += image;
	}
	return strImages;
}


// NoFocusDelegate

void NoFocusDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
	QStyleOptionViewItem itemOption(option);
	if (itemOption.state & QStyle::State_HasFocus)
		itemOption.state = itemOption.state ^ QStyle::State_HasFocus;
	QStyledItemDelegate::paint(painter, itemOption, index);
}

#include "ui_SpacingSelector.h"
#include "SpacingSelector.h"

#include <QString>
#include <QMessageBox>

#include <sstream>
#include <iomanip>

SpacingSelector::SpacingSelector(QWidget* parent)
	: QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint),
	ui(*new Ui::spacingselector)
{
	ui.setupUi(this);

	is_value_changed_by_user = false;

	this->setWindowTitle("POPE: Select a Voxel Spacing");
	this->setWindowFlags(this->windowFlags() | Qt::WindowCloseButtonHint);

	/// Costomize UI elements
	ui.label_data->setVisible(false);
	ui.label_dataSet->setVisible(false);
}

SpacingSelector::~SpacingSelector()
{
	delete &ui;
}

void SpacingSelector::SetData(const vector<shared_ptr<SpacingDescriptor>>& spacings)
{
	this->spacings = spacings;
	ui.tableWidget_Spacings->setRowCount((int)spacings.size());
	/// Fill out the table of patients
	for (size_t i = 0; i <  spacings.size(); i++)
	{
		int index = (int)i;
		QTableWidgetItem* item_1 = new QTableWidgetItem(spacings[i]->tag);
		QString tag_name = spacings[i]->tag;
		if (i == 0)
		{
			tag_name += " <DEFAULT>";
			ui.doubleSpinBox_userValue->blockSignals(true);
			ui.doubleSpinBox_userValue->setValue(spacings[i]->value);
			ui.doubleSpinBox_userValue->blockSignals(false);
		}
		item_1->setToolTip(tag_name);
		ui.tableWidget_Spacings->setItem(index, 0, item_1);
		QTableWidgetItem* item_2 = new QTableWidgetItem(spacings[i]->key);
		item_2->setToolTip("DICOM." + spacings[i]->key.replace(',', '.'));
		ui.tableWidget_Spacings->setItem(index, 1, item_2);
		QTableWidgetItem* item_3 = new QTableWidgetItem(spacings[i]->Value());
		ui.tableWidget_Spacings->setItem(index, 2, item_3);
		item_1->setData(Qt::UserRole, index);
		//QString hint_images = patients[i]->ImageNamesAsString();
		//item_2->setToolTip(hint_images);
	}
}

void SpacingSelector::SetImageDescription(const QString& imageDescription)
{
	bool visible = !imageDescription.isEmpty();
	ui.label_data->setVisible(visible);
	ui.label_dataSet->setVisible(visible);
	if (visible)
		ui.label_dataSet->setText(imageDescription);
}

int SpacingSelector::SelectedSpacingIndex(float* value)
{
	if (value != nullptr)
		*value = (float)ui.doubleSpinBox_userValue->value();
	// Get selected items
	auto selected_items = ui.tableWidget_Spacings->selectedItems();
	if (selected_items.size() == 0)
		return -1;

	int row = selected_items.front()->row();
	auto item = ui.tableWidget_Spacings->item(row, 0);

	bool is_ok;
	int index = item->data(Qt::UserRole).toInt(&is_ok);
	if (is_ok)
		return index;
	else
		return -2;
}

void SpacingSelector::on_tableWidget_Spacings_itemSelectionChanged()
{
	auto selected_items = ui.tableWidget_Spacings->selectedItems();
	bool enabled = (selected_items.size() > 0);
	ui.pushButton_OK->setEnabled(enabled);
	if (enabled)
	{
		int row = selected_items.front()->row();
		auto item = ui.tableWidget_Spacings->item(row, 0);
		bool is_ok;
		int index = item->data(Qt::UserRole).toInt(&is_ok);
		if (is_ok && index >= 0 && index < (int) spacings.size())
		{
			float value = spacings[(uint)index]->value;
			is_value_changed_by_user = false;
			ui.doubleSpinBox_userValue->blockSignals(true);
			ui.doubleSpinBox_userValue->setValue(value);
			ui.doubleSpinBox_userValue->blockSignals(false);
		}

		ui.pushButton_OK->setFocus();
	}
}
void SpacingSelector::on_doubleSpinBox_userValue_valueChanged(double)
{
	is_value_changed_by_user = true;
	ui.tableWidget_Spacings->blockSignals(true);
	ui.tableWidget_Spacings->clearSelection();
	ui.tableWidget_Spacings->blockSignals(false);
	ui.pushButton_OK->setEnabled(true);
}
void SpacingSelector::on_pushButton_OK_clicked()
{
	accept();
}

void SpacingSelector::on_pushButton_Cancel_clicked()
{
	reject();
}



SpacingDescriptor::SpacingDescriptor(const QString& tag, const QString& key, float value)
	: tag(tag), key(key), value(value)
{
	//this->tag = tag;
	//this->key = key;
	//this->value = value;
}

QString SpacingDescriptor::Value()
{
	stringstream ss;
	ss << std::setprecision(2) << std::fixed << value;
	return QString::fromStdString(ss.str());
}

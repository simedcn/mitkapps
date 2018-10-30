//#pragma once
#ifndef SPACINGSELECTOR_H
#define SPACINGSELECTOR_H

//#include "ui_SpacingSelector.h"

#include <QMainWindow>
#include <QApplication>
#include <QDialog>
#include <QStyledItemDelegate>

#include <vector>
#include <memory>

using namespace std;

// Forward declaration of ui widget
namespace Ui
{
	class spacingselector;
}
struct SpacingDescriptor;

class SpacingSelector : public QDialog
{
	Q_OBJECT
public:
	SpacingSelector(QWidget* parent = nullptr);
	~SpacingSelector();

public:
	void SetData(const vector<shared_ptr<SpacingDescriptor>>& spacings);
	void SetImageDescription(const QString& imageDescription);
	int SelectedSpacingIndex(float* value);

public slots:
	void on_pushButton_OK_clicked();
	void on_pushButton_Cancel_clicked();
	void on_tableWidget_Spacings_itemSelectionChanged();
	void on_doubleSpinBox_userValue_valueChanged(double);

private:
	Ui::spacingselector& ui;
	vector<shared_ptr<SpacingDescriptor>> spacings;
	bool is_value_changed_by_user;
};

struct SpacingDescriptor
{
	SpacingDescriptor(const QString& tag, const QString& key, float value);

	QString tag;
	QString key;
	float value;

	QString Value();
};
#endif // SPACINGSELECTOR_H
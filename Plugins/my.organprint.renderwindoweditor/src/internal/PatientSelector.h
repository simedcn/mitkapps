//#pragma once
#ifndef PATIENTSELECTOR_H
#define PATIENTSELECTOR_H

//#include "ui_PatientSelector.h"

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
	class patientselector;
}
struct PatientDescription;

class PatientSelector : public QDialog
{
	Q_OBJECT
public:
	PatientSelector(QWidget* parent = nullptr);
	~PatientSelector();

public:
	void SetPatientData(const vector<shared_ptr<PatientDescription>>& patients);
	void SetFolder(const QString& folder);
	int SelectedPatientIDIndex();

public slots:
	void on_tableWidget_PatientsIDs_itemSelectionChanged();
	void on_pushButton_OK_clicked();
	void on_pushButton_Cancel_clicked();

private:
	Ui::patientselector& ui;
	vector<shared_ptr<PatientDescription>> patients;
};

struct PatientDescription
{
	QString id;
	QString name;
	vector<QString> images;

	PatientDescription(const QString& id = "", const QString& name = "", const vector<QString>& images = vector<QString>());
	QString ImageNamesAsString() const;
};

// Custom item delegate class
class NoFocusDelegate : public QStyledItemDelegate
{
protected:
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif // PATIENTSELECTOR_H
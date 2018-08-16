//#pragma once
#ifndef POPESETTINGS_H
#define POPESETTINGS_H

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
	class popesettings;
}

class PopeSettings : public QDialog
{
	Q_OBJECT
public:
	PopeSettings(QWidget* parent = nullptr);
	~PopeSettings();

public:

public slots:
	void on_pushButton_OK_clicked();
	void on_pushButton_Cancel_clicked();

private:
	Ui::popesettings& ui;
};

#endif // POPESETTINGS_H
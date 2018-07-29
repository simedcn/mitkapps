#ifndef TISSUPANEL_H
#define TISSUPANEL_H

#include <QmitkAbstractView.h>

class TissuPanel : public QmitkAbstractView
{
public:
    TissuPanel();
    void CreateQtPartControl(QWidget* parent) override;

    void SetFocus() override;
protected:

    Ui::TissuPanelControls * m_Controls;

};

#endif // TISSUPANEL_H

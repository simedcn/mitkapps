#ifndef TISSUPANEL_H
#define TISSUPANEL_H

#include <QmitkAbstractView.h>
#include "ui_TissuPanelControls.h"
#include <a.out.h>
#include <mitkDataNode.h>

#include <mitkMessage.h>

namespace orgpnt {


class TissuPanel : public QmitkAbstractView
{

    typedef mitk::MessageDelegate1<orgpnt::TissuPanel, const mitk::DataNode *> StorageListener;

    Q_OBJECT
public:
    TissuPanel();
    ~TissuPanel();
    static const std::string VIEW_ID;


    std::string * GetName();

    void CreateQtPartControl(QWidget* parent) override;

    void SetFocus() override;

    void OnNodeChanged(const mitk::DataNode * node);

    void UpdateSelectionLabel(const mitk::DataNode * node);

    void UpdateComboBox();

    void UpdateTissuSelection();

public slots:
    void onCurrentTissuTypeChanged(int);

protected:

    Ui::OrganPrintTissuPanelControls m_Controls;

    mitk::DataNode * GetSelectedNode();

    const StorageListener listener;
};
}
#endif // TISSUPANEL_H

#ifndef TISSUPANEL_H
#define TISSUPANEL_H

#include <QmitkAbstractView.h>
#include "ui_TissuePanelControls.h"
#include <a.out.h>
#include <mitkDataNode.h>

#include <mitkMessage.h>

namespace orgpnt {


class TissuePanel : public QmitkAbstractView
{

    typedef mitk::MessageDelegate1<orgpnt::TissuePanel, const mitk::DataNode *> StorageListener;

    Q_OBJECT
public:
    TissuePanel();
    ~TissuePanel();
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

    Ui::OrganPrintTissuePanelControls m_Controls;

    mitk::DataNode * GetSelectedNode();

    const StorageListener listener;
};
}
#endif // TISSUPANEL_H

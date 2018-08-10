#include "TissuePanel.h"

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <TissuTypeService.h>
#include <TissuType.h>
#include <QComboBox>
#include <iostream>

#include <mitkDataStorage.h>
#include <mitkDataNode.h>
#include <mitkProperties.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>

using namespace std;

const std::string orgpnt::TissuePanel::VIEW_ID = "my.organprint.views.tissuepanel";


orgpnt::TissuePanel::TissuePanel()
    : listener(StorageListener(this, &TissuePanel::OnNodeChanged))
{

}

orgpnt::TissuePanel::~TissuePanel() {
    cout << "Destroying tissu panel" << endl;

    mitk::DataStorage * storage = GetDataStorage();

    storage->ChangedNodeEvent.RemoveListener(listener);
}

void orgpnt::TissuePanel::CreateQtPartControl(QWidget *parent) {


    m_Controls.setupUi(parent);

    mitk::DataStorage * storage = GetDataStorage();

    storage->ChangedNodeEvent.AddListener(listener);

    UpdateComboBox();


    QComboBox * cb = m_Controls.tissuTypeComboBox;

    connect(cb,SIGNAL(currentIndexChanged(int)),this,SLOT(onCurrentTissuTypeChanged(int)));


}

void orgpnt::TissuePanel::onCurrentTissuTypeChanged(int newType) {

    mitk::DataNode * selected = GetSelectedNode();

    if(selected != nullptr) {

        int currentTissuType = -1;

        selected->GetIntProperty(TissuType::PROPERTY_KEY.c_str(),currentTissuType);
        if(currentTissuType != newType && newType != -1) {
            selected->SetIntProperty(TissuType::PROPERTY_KEY.c_str(),newType);

        }
    }

}

void orgpnt::TissuePanel::SetFocus() {

}

void orgpnt::TissuePanel::OnNodeChanged(const mitk::DataNode *node) {

    const QString NO_SELECTION("Select a segmentation node");

    if(node) {

        bool isSelected = false;

        node->GetBoolProperty("selected",isSelected);

        if(node == GetSelectedNode()) {

            UpdateSelectionLabel(node);
        }

    }


    UpdateTissuSelection();


}

void orgpnt::TissuePanel::UpdateSelectionLabel(const mitk::DataNode * node) {

    m_Controls.tissuNameLabel->setText(QString::fromStdString(node->GetName()));



}

void orgpnt::TissuePanel::UpdateComboBox() {

    QComboBox * cb = m_Controls.tissuTypeComboBox;

    if(cb) {
        cout << "Found Combobox" << endl;
        TissuTypeService * tts = TissuTypeService::GetInstance();

        if(tts) {
            cout << "Found Service" << endl;
            for(TissuType * type : *(tts->GetTissuTypeList())) {
                if(type) {
                    cout << "Found tissu type" << endl;
                    cb->addItem(QString::fromStdString(*(type->GetName())));
                }
            }
        }

    }
}

void orgpnt::TissuePanel::UpdateTissuSelection() {

    mitk::DataNode * selectedNode = this->GetSelectedNode();
    QComboBox * cb = m_Controls.tissuTypeComboBox;

    if(selectedNode) {

        int selectedTissuType = -1;

        selectedNode->GetIntProperty(TissuType::PROPERTY_KEY.c_str(),selectedTissuType);


        if(selectedTissuType != cb->currentIndex()) {
            cb->setCurrentIndex(selectedTissuType);
        }

    }
    else {
        cb->setCurrentIndex(-1);
    }
}

mitk::DataNode * orgpnt::TissuePanel::GetSelectedNode() {


    mitk::DataStorage * storage = GetDataStorage();

    mitk::DataNode * selected = storage->GetNode(
                                    mitk::NodePredicateAnd::New(
                                        mitk::NodePredicateProperty::New("selected",mitk::BoolProperty::New(true))
                                        ,mitk::NodePredicateProperty::New("segmentation",mitk::BoolProperty::New(true))
                                    )
                                );

    return selected;

}



#ifndef UPLOADTOPACSACTION_H
#define UPLOADTOPACSACTION_H

// Parent classes
#include <QObject>
#include <mitkIContextMenuAction.h>

// Data members
#include <mitkDataNode.h>

class UploadToPACSAction : public QObject, public mitk::IContextMenuAction
{
	Q_OBJECT
	Q_INTERFACES(mitk::IContextMenuAction)

public:
	UploadToPACSAction();
	~UploadToPACSAction();

	// IContextMenuAction
	void Run(const QList<mitk::DataNode::Pointer> &selectedNodes) override;
	void SetDataStorage(mitk::DataStorage *dataStorage) override;
	void SetSmoothed(bool smoothed) override;
	void SetDecimated(bool decimated) override;
	void SetFunctionality(berry::QtViewPart* view) override;

	void OnSurfaceCalculationDone();

private:
	UploadToPACSAction(const UploadToPACSAction&);
	UploadToPACSAction& operator=(const UploadToPACSAction&);

protected:
	bool findStoreSCU();

//signals:
//void run(const string& path);

private:
	mitk::DataStorage::Pointer m_DataStorage;
	bool m_IsSmoothed;
	bool m_IsDecimated;

	QString m_StoreSCUExecutable;
};

#endif

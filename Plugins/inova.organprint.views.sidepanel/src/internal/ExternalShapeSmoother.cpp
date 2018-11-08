#include "ExternalShapeSmoother.h"
#include <mitkSurface.h>

#include "itkIntelligentBinaryClosingFilter.h"
#include "mitkImageCast.h"
#include "mitkImageToItk.h"
#include <itkAddImageFilter.h>
#include <itkBinaryMedianImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkConstantPadImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkMultiplyImageFilter.h>
#include <itkRegionOfInterestImageFilter.h>
#include <mitkGeometry3D.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageToSurfaceFilter.h>
#include <mitkProgressBar.h>
#include <mitkStatusBar.h>
#include <mitkUIDGenerator.h>
#include <mitkVtkRepresentationProperty.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkQuadricDecimation.h>
// MITK

#include <mitkShowSegmentationAsSurface.h>
#include <mitkProgressBar.h>
#include <mitkStatusBar.h>
#include <mitkIRenderWindowPart.h>
#include <mitkSliceNavigationController.h>

// Blueberry
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>
#include <berryPlatformUI.h>
#include <berryIWorkbenchPage.h>

using namespace mitk;
using namespace berry;
using namespace std;


void ExternalShapeSmoother::smooth(const mitk::DataNode * node)   {

    mitk::ShowSegmentationAsSurface::Pointer surfaceFilter = mitk::ShowSegmentationAsSurface::New();

    //itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::Pointer successCommand = itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::New();
    //successCommand->SetCallbackFunction(this, &QmitkCreatePolygonModelAction::OnSurfaceCalculationDone);
    //surfaceFilter->AddObserver(mitk::ResultAvailable(), successCommand);

    //itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::Pointer errorCommand = itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::New();
    //errorCommand->SetCallbackFunction(this, &QmitkCreatePolygonModelAction::OnSurfaceCalculationDone);
    //surfaceFilter->AddObserver(mitk::ProcessingError(), errorCommand);

    surfaceFilter->SetRunOnBackground(true);

    mitk::Image * image = (mitk::Image*) node->GetData();

    //surfaceFilter->SetDataStorage(*m_DataStorage);
    surfaceFilter->SetPointerParameter("Input", image);

    mitk::DataNode::Pointer selectedNode(const_cast<mitk::DataNode*>(node));

    surfaceFilter->SetPointerParameter("Group node", selectedNode);

    berry::IWorkbenchPart::Pointer activePart =
        berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->GetActivePart();
    mitk::IRenderWindowPart* renderPart = dynamic_cast<mitk::IRenderWindowPart*>(activePart.GetPointer());
    mitk::SliceNavigationController* timeNavController = 0;
    if (renderPart != 0)
    {
        timeNavController = renderPart->GetTimeNavigationController();
    }

    int timeNr = timeNavController != 0 ? timeNavController->GetTime()->GetPos() : 0;
    surfaceFilter->SetParameter("TimeNr", timeNr);

    berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
    berry::IPreferences::Pointer segPref = prefService->GetSystemPreferences()->Node("/org.mitk.views.multilabelsegmentation");

    bool smoothingHint = segPref->GetBool("smoothing hint", true);
    mitk::ScalarType smoothing = segPref->GetDouble("smoothing value", 2.0);
    mitk::ScalarType decimation = segPref->GetDouble("decimation rate", 0.5);
    mitk::ScalarType closing = segPref->GetDouble("closing ratio", 0.0);

    if (smoothingHint)
    {
        smoothing = 0.0;
        Vector3D spacing = image->GetGeometry()->GetSpacing();

        for (Vector3D::Iterator iter = spacing.Begin(); iter != spacing.End(); ++iter)
            smoothing = max(smoothing, *iter);
    }

    surfaceFilter->SetParameter("Smoothing", smoothing);
    surfaceFilter->SetParameter("Decimation", decimation);
    surfaceFilter->SetParameter("Closing", closing);



    mitk::ProgressBar::GetInstance()->AddStepsToDo(8);
    mitk::StatusBar::GetInstance()->DisplayText("Smoothed surface creation started in background...");

    try {
        surfaceFilter->StartBlockingAlgorithm();
        //surfaceFilter->ThreadedUpdateSuccessful();
    } catch (...)
    {
        MITK_ERROR<<"Error creating smoothed polygon model: Not enough memory!";
    }

    if(surfaceFilter->GetSurface().IsNotNull()) {
        success = true;
    }
    result = surfaceFilter->GetNode();

}

mitk::DataNode * ExternalShapeSmoother::GetResult() {
    return result;
}

bool ExternalShapeSmoother::Success() {
    return success;
}

/*
* Author: Zhiqiang Liang
* Email: dpstill@126.com
*/

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>
#include <vector>
#include <array>

#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkDICOMImageReader.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkOutlineFilter.h"
#include "vtkCamera.h"
#include "vtkProperty.h"
#include "vtkMarchingCubes.h"
#include "vtkStripper.h"
#include "vtkSmartPointer.h"
#include "vtkImageData.h"
#include "vtkNamedColors.h"
#include "vtkAutoInit.h"

VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2)
VTK_MODULE_INIT(vtkRenderingFreeType)
VTK_MODULE_INIT(vtkInteractionStyle)

int main(int argc, char *argv[])
{

	//const char *dirname = "C:/__2";
	//const char * dirname = "D:\\d___3";
	const char *dirname = "D:\\dataset\\SE1";
	int clip = 0;

	//------------------------------- Read DICOM images
	vtkSmartPointer<vtkAlgorithm> reader = vtkSmartPointer<vtkAlgorithm>::New();
	vtkSmartPointer<vtkImageData> input = vtkSmartPointer<vtkImageData>::New();
	vtkSmartPointer<vtkDICOMImageReader> dicomReader = vtkSmartPointer<vtkDICOMImageReader>::New();
	dicomReader->SetDirectoryName(dirname);
	dicomReader->Update();
	input = dicomReader->GetOutput();
	reader = dicomReader;
	
	//------------------------------- Set the colors.
	vtkSmartPointer<vtkNamedColors> colors = vtkSmartPointer<vtkNamedColors>::New();
	std::array<unsigned char, 4> skinColor{ { 255, 125, 64 } };
	colors->SetColor("SkinColor", skinColor.data());
	std::array<unsigned char, 4> bkg{ { 51, 77, 102, 255 } };
	colors->SetColor("BkgColor", bkg.data());

	//------------------------------- Skin 
	// An isosurface, or contour value of 500 is known to correspond to the skin of the patient.
	// The triangle stripper is used to create triangle strips from the isosurface; these render much faster on many systems.
	vtkSmartPointer<vtkMarchingCubes> skinExtractor = vtkSmartPointer<vtkMarchingCubes>::New();
	skinExtractor->SetInputConnection(reader->GetOutputPort());
	skinExtractor->SetValue(0, 100);

	vtkSmartPointer<vtkStripper> skinStripper = vtkSmartPointer<vtkStripper>::New();
	skinStripper->SetInputConnection(skinExtractor->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> skinMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	skinMapper->SetInputConnection(skinStripper->GetOutputPort());
	skinMapper->ScalarVisibilityOff();

	vtkSmartPointer<vtkActor> skin = vtkSmartPointer<vtkActor>::New();
	skin->SetMapper(skinMapper);
	skin->GetProperty()->SetDiffuseColor(colors->GetColor3d("SkinColor").GetData());
	skin->GetProperty()->SetSpecular(.3);
	skin->GetProperty()->SetSpecularPower(20);
	skin->GetProperty()->SetOpacity(.5);

#if 1
	//------------------------------- Bone
	// An isosurface, or contour value of 1150 is known to correspond to the bone of the patient.
	// The triangle stripper is used to create triangle strips from the isosurface; these render much faster on may systems.
	vtkSmartPointer<vtkMarchingCubes> boneExtractor = vtkSmartPointer<vtkMarchingCubes>::New();
	boneExtractor->SetInputConnection(reader->GetOutputPort());
	boneExtractor->SetValue(0, 300);

	vtkSmartPointer<vtkStripper> boneStripper = vtkSmartPointer<vtkStripper>::New();
	boneStripper->SetInputConnection(boneExtractor->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> boneMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	boneMapper->SetInputConnection(boneStripper->GetOutputPort());
	boneMapper->ScalarVisibilityOff();

	vtkSmartPointer<vtkActor> bone = vtkSmartPointer<vtkActor>::New();
	bone->SetMapper(boneMapper);
	bone->GetProperty()->SetDiffuseColor(colors->GetColor3d("Ivory").GetData());
#endif

	//------------------------------- Outline // An outline provides context around the data.
	vtkSmartPointer<vtkOutlineFilter> outlineData = vtkSmartPointer<vtkOutlineFilter>::New();
	outlineData->SetInputConnection(reader->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> mapOutline = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapOutline->SetInputConnection(outlineData->GetOutputPort());

	vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
	outline->SetMapper(mapOutline);
	outline->GetProperty()->SetColor(colors->GetColor3d("Black").GetData());

	//------------------------------- camera
	// It is convenient to create an initial view of the data. The FocalPoint and Position form a vector direction. Later on (ResetCamera() method)
	// this vector is used to position the camera to look at the data in this direction.
	vtkSmartPointer<vtkCamera> aCamera = vtkSmartPointer<vtkCamera>::New();
	aCamera->SetViewUp(0, 0, -1);
	aCamera->SetPosition(0, -1, 0);
	aCamera->SetFocalPoint(0, 0, 0);
	aCamera->ComputeViewPlaneNormal();
	aCamera->Azimuth(30.0);
	aCamera->Elevation(30.0);
	
	//------------------------------- create winwow
	// Create the renderer, the render window, and the interactor. 
	// The renderer draws into the render window, the interactor enables mouse- and keyboard-based interaction with the data within the render window.
	vtkSmartPointer<vtkRenderer> aRenderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(aRenderer);
	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren->SetRenderWindow(renWin);


	//------------------------------- display
	// Actors are added to the renderer. An initial camera view is created. The Dolly() method moves the camera towards the FocalPoint, thereby enlarging the image.
	//aRenderer->AddActor(outline);
	aRenderer->AddActor(skin);
	//aRenderer->AddActor(bone);
	aRenderer->SetActiveCamera(aCamera);
	aRenderer->ResetCamera();
	aCamera->Dolly(1.5);

	// Set a background color for the renderer and set the size of the render window (expressed in pixels).
	aRenderer->SetBackground(colors->GetColor3d("BkgColor").GetData());
	renWin->SetSize(640, 480);

	// Note that when camera movement occurs (as it does in the Dolly() method), the clipping planes often need adjusting. Clipping planes consist of two planes: near and far along the view direction.
	// The near plane clips out objects in front of the plane; the far plane clips out objects behind the plane. This way only what is drawn between the planes is actually rendered.
	aRenderer->ResetCameraClippingRange();

	// Initialize the event loop and then start it.
	renWin->Render();
	iren->Initialize();
	iren->Start();

	return EXIT_SUCCESS;
}







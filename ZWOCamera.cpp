// ZWOCamera.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <ASICamera2.h>
#include <opencv2/opencv.hpp> //figure this out. Probably need to includ .lib files again

using namespace std;

//#pragma comment (lib, "ASICamera2.lib")

int main()
{
	int cameraCount;
	int controlNum;
	int capacityControl;
	ASI_CAMERA_INFO* ZWOCamera = (ASI_CAMERA_INFO*)malloc(sizeof(ASI_CAMERA_INFO));
	ASI_CONTROL_CAPS* ZWOControl = (ASI_CONTROL_CAPS*)malloc(sizeof(ASI_CONTROL_CAPS));
	ASI_CONTROL_TYPE ZWOType; //= (ASI_CONTROL_TYPE)malloc(sizeof(ASI_CONTROL_TYPE));
	
	ASI_SUPPORTED_MODE* ZWOSupport = (ASI_SUPPORTED_MODE*)malloc(sizeof(ASI_SUPPORTED_MODE));
	ASI_CAMERA_MODE* ZWOMode = (ASI_CAMERA_MODE*)malloc(sizeof(ASI_CAMERA_MODE));

	cameraCount = ASIGetNumOfConnectedCameras(); //detects if a ASI camera is connected
	cout << "Number of cameras connected: ";
	cout << cameraCount << endl;
	
	ASIGetCameraProperty(ZWOCamera,0); //collects the properties of 1st connected ASI camera into a _ASI_CAMERA_INFO struct

	cout << "Camera: " << ZWOCamera->Name << endl;
	
	cout << "Image Resolution: " << ZWOCamera->MaxWidth << "x" << ZWOCamera->MaxHeight << endl;

	ASIOpenCamera(ZWOCamera->CameraID);  //opens
	ASIInitCamera(ZWOCamera->CameraID);

	//ASIGetNumOfControls(ZWOCamera->CameraID, &controlNum);

	//cout << "Number of Controls: " << controlNum << endl;

	//ASIGetControlCaps(ZWOCamera->CameraID, 0, ZWOControl); //use this to access different control categories (access gain, exposure, etc.) from ZWOCamera. the 2nd arguement tells the program which category to access (gain=0, exposure=1). the control category is stored in ZWOControl
	
	//cout << "Controls: " << ZWOControl->Name << endl;

	//cout << ZWOCamera->SupportedVideoFormat[1] << endl;

	ASISetROIFormat(ZWOCamera->CameraID, ZWOCamera->MaxWidth, ZWOCamera->MaxHeight, 1, ASI_IMG_RGB24); //sets the camera up with its proper resolution and in color mode
	ASISetStartPos(ZWOCamera->CameraID, 0, 0);

	//ASISetControlValue(ZWOCamera->CameraID, ZWOType, );

	/*ASIGetCameraSupportMode(ZWOCamera->CameraID, );
	ASISetCameraMode(ZWOCamera->CameraID, *ZWOMode);
	ASIGetCameraMode(ZWOCamera->CameraID, ZWOMode);
	*/
	/*if (ZWOCamera->IsTriggerCam) {
		
	}*/

	ASISetCameraMode(ZWOCamera->CameraID, ASI_MODE_NORMAL);

	//cout << ZWOCamera->IsColorCam << endl;

	char exitCommand = '0';
	unsigned char X[3552];
	ofstream myFile("video.raw");

	ASIStartVideoCapture(ZWOCamera->CameraID);
	for (int i = 0; i < 3552; i++){
		if (ASIGetVideoData(ZWOCamera->CameraID, X, 3552*3552, -1) == ASI_SUCCESS) {
			myFile << X;
		}
		else {

		}
		cout << X << endl;
	}
	ASIStopVideoCapture(ZWOCamera->CameraID);
	myFile.close();
	/*while () {
		cin >> exitCommand;
		if (ASIGetVideoData(ZWOCamera->CameraID, X, 256, 1000) == ASI_SUCCESS) {
			ASIStartVideoCapture(ZWOCamera->CameraID);
			ASIGetVideoData(ZWOCamera->CameraID, X, 256, 1000);
			cout << X << endl;
		}
	}
	ASIStopVideoCapture(ZWOCamera->CameraID);*/

	ASICloseCamera(ZWOCamera->CameraID);
	

	return 0;
}

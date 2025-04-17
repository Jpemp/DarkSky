// ZWOCamera.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <ASICamera2.h>

using namespace std;

//#pragma comment (lib, "ASICamera2.lib")

int main()
{
	int cameraCount;
	ASI_CAMERA_INFO* ZWOCamera = (ASI_CAMERA_INFO*)malloc(sizeof(ASI_CAMERA_INFO*));
	cameraCount = ASIGetNumOfConnectedCameras();
	cout << "Number of cameras connected: ";
	cout << cameraCount << endl;
	ASIGetCameraProperty(ZWOCamera,0);
	//ASIOpenCamera(ZWOCamera->CameraID); //figure out camera ID
	//ASIInitCamera(ZWOCamera->CameraID);

	int x = 0;
	if (x == 1) {
		ASICloseCamera(ZWOCamera->CameraID);
	}

	return 0;
}

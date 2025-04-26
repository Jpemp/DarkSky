// ZWOCamera.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <ASICamera2.h>
#include <opencv2/opencv.hpp> //figure this out. Probably need to include .lib files again



using namespace std;
using namespace cv;

//#pragma comment (lib, "ASICamera2.lib")

int main(){
	int cameraCount;
	ASI_CAMERA_INFO* ZWOCamera = (ASI_CAMERA_INFO*)malloc(sizeof(ASI_CAMERA_INFO));

	cameraCount = ASIGetNumOfConnectedCameras(); //detects if a ASI camera is connected
	cout << "Number of cameras connected: ";
	cout << cameraCount << endl;
	
	ASIGetCameraProperty(ZWOCamera,0); //collects the properties of 1st connected ASI camera into a _ASI_CAMERA_INFO struct
	//cout << ASIGetCameraProperty(ZWOCamera, 0) << endl;
	cout << "Camera: " << ZWOCamera->Name << endl;
	
	cout << "Image Resolution: " << ZWOCamera->MaxWidth << "x" << ZWOCamera->MaxHeight << endl;
	cout << "Pixel Size: " << ZWOCamera->PixelSize << endl;

	int width = ZWOCamera->MaxWidth;
	int height = ZWOCamera->MaxHeight;
	//cout << ZWOCamera->BitDepth << endl;

	if (ASIOpenCamera(ZWOCamera->CameraID) == ASI_SUCCESS) {  //opens
		cout << "Camera Successfully Opened" << endl;
	}

	if (ASIInitCamera(ZWOCamera->CameraID) == ASI_SUCCESS) {
		cout << "Camera Successfully Initialized" << endl;
	}

	//cout << "Controls: " << ZWOControl->Name << endl;

	//cout << ZWOCamera->SupportedVideoFormat[1] << endl;

	ASISetROIFormat(ZWOCamera->CameraID, width, height, 1, ASI_IMG_RAW16); //sets the camera up with its proper resolution and in color mode
	ASISetStartPos(ZWOCamera->CameraID, 0, 0);

	//cout << ZWOCamera->SupportedBins[3] << endl;
	
	/*if (ASISetControlValue(ZWOCamera->CameraID, ASI_HIGH_SPEED_MODE, 1, ASI_TRUE) == ASI_SUCCESS) {
		cout << "Control Value Successfully Set" << endl;
	}*/

	long exposure = 25000;
	long gain = 100;

	ASISetControlValue(ZWOCamera->CameraID, ASI_EXPOSURE, exposure, ASI_TRUE);
	ASISetControlValue(ZWOCamera->CameraID, ASI_GAIN, gain, ASI_TRUE);

	if (ASISetCameraMode(ZWOCamera->CameraID, ASI_MODE_NORMAL) == ASI_SUCCESS) {
		cout << "Camera Mode Successfully Set" << endl;
	}

	
	if (ASIStartVideoCapture(ZWOCamera->CameraID) == ASI_SUCCESS) {
		cout << "Camera Video Successfully Started" << endl;
	}

	Mat image(width, height, CV_16U);
	Mat color_image;

	if ((ASIGetVideoData(ZWOCamera->CameraID, image.data, width*height*2, ASI_EXPOSURE*2+500)) == ASI_SUCCESS) {
		cout << "Video Data Is Captured!" << endl;

		//cvtColor(image, color_image, COLOR_GRAY2RGB);
		imwrite("ASIG.png", image);
		//imshow("Colored Image", image);
		//waitKey(0);
	}
	else {
		cout << "Didn't get video data!" << endl;
	}
	

	if (ASIStopVideoCapture(ZWOCamera->CameraID) == ASI_SUCCESS) {
		cout << "Camera Video Successfully Stopped" << endl;
	}
	

	if (ASICloseCamera(ZWOCamera->CameraID) == ASI_SUCCESS) {
		cout << "Camera Successfully Closed" << endl;
	}
	
	return 0;
}

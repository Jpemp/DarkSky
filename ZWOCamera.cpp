// ZWOCamera.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <thread>
#include <ctime>
#include <chrono>
#include <atomic>
#include <ASICamera2.h>
#include <opencv2/opencv.hpp> //figure this out. Probably need to include .lib files again
//#include <opencv2/imgproc.hpp>
//#include <opencv2/highgui.hpp>
//#include <opencv2/core/mat.hpp>

#include <jni.h>
#include "Main.h"

using namespace std;
using namespace cv;

atomic<bool> thread_end = false;

void capture(ASI_CAMERA_INFO*, int);

JNIEXPORT jint JNICALL Java_Main_Capture(JNIEnv *env, jobject obj, jboolean vidFlag, jint capTimer) {

	int cameraCount;
	ASI_CAMERA_INFO* ZWOCamera = (ASI_CAMERA_INFO*)malloc(sizeof(ASI_CAMERA_INFO));

	bool liveVid = (bool)vidFlag;
	int capTime = (int)capTimer;


	cameraCount = ASIGetNumOfConnectedCameras(); //detects if a ASI camera is connected
	cout << "Number of cameras connected: ";
	cout << cameraCount << endl;
	
	ASIGetCameraProperty(ZWOCamera,0); //collects the properties of 1st connected ASI camera into a _ASI_CAMERA_INFO struct
	cout << "Camera: " << ZWOCamera->Name << endl;
	if (cameraCount != 1) {
		cout << "No Camera/Too Many Cameras Connected!" << endl;
		return -1;
	}

	cout << "Image Resolution: " << ZWOCamera->MaxWidth << "x" << ZWOCamera->MaxHeight << endl;
	cout << "Pixel Size: " << ZWOCamera->PixelSize << endl;

	int width = ZWOCamera->MaxWidth;
	int height = ZWOCamera->MaxHeight;

	if (ASIOpenCamera(ZWOCamera->CameraID) == ASI_SUCCESS) {  //opens
		cout << "Camera Successfully Opened" << endl;
	}
	else {
		cout << "Camera Didn't Open!" << endl;
		return -1;
	}

	if (ASIInitCamera(ZWOCamera->CameraID) == ASI_SUCCESS) {
		cout << "Camera Successfully Initialized" << endl;
	}
	else {
		cout << "Camera Didn't Initialize!" << endl;
		return -1;
	}

	ASISetROIFormat(ZWOCamera->CameraID, width, height, 1, ASI_IMG_RGB24); //sets the camera up with its proper resolution and in color mode
	ASISetStartPos(ZWOCamera->CameraID, 0, 0);



	long exposure = 100000; //in microseconds(us)
	long gain = 250; //keep gain around this

	long* curr_exposure = (long*)malloc(sizeof(long));
	long* curr_gain = (long*)malloc(sizeof(long));

	ASI_BOOL* exposure_auto = (ASI_BOOL*)malloc(sizeof(ASI_BOOL));
	ASI_BOOL* gain_auto = (ASI_BOOL*)malloc(sizeof(ASI_BOOL));

	ASISetControlValue(ZWOCamera->CameraID, ASI_EXPOSURE, exposure, ASI_TRUE); //exposure of camera is set at  100000 microseconds. ASI_TRUE means exposure will auto-adjust
	ASISetControlValue(ZWOCamera->CameraID, ASI_GAIN, gain, ASI_FALSE); //gain is set at 100. ASI_FALSE means gain won't auto-adjust.

	//cout << exposure << endl;
	//cout << gain << endl;


	ASIGetControlValue(ZWOCamera->CameraID, ASI_EXPOSURE, curr_exposure, exposure_auto);
	ASIGetControlValue(ZWOCamera->CameraID, ASI_GAIN, curr_gain, gain_auto);

	//cout << *curr_exposure << endl;
	//cout << *curr_gain << endl;

	if (ASISetCameraMode(ZWOCamera->CameraID, ASI_MODE_NORMAL) == ASI_SUCCESS) {
		cout << "Camera Mode Successfully Set" << endl;
	}
	else {
		cout << "Camera Mode Wasn't Set Properly!" << endl;
		return -1;
	}

	
	if (ASIStartVideoCapture(ZWOCamera->CameraID) == ASI_SUCCESS) {
		cout << "Camera Video Successfully Started" << endl;
	}
	else {
		cout << "Camera Video Didn't Start!" << endl;
		return -1;
	}
	ASISetControlValue(ZWOCamera->CameraID, ASI_HIGH_SPEED_MODE, 1, ASI_FALSE);
	
	Mat image(width, height, CV_8UC3);
	Mat window;
	int timer = 10;
	thread tCap(capture, ZWOCamera, capTime);
	//bool placeHolder = true;
	while (true) {
		if (thread_end == true) {
			tCap.join();
			cout << "Thread ended!" << endl;
			break;
		}
		if (liveVid == true) {
			if ((ASIGetVideoData(ZWOCamera->CameraID, image.data, width * height * 3, ASI_EXPOSURE * 2 + 500)) == ASI_SUCCESS) {

				//cout << "Feed Started" << endl;

				resize(image, window, Size(1920, 1080));
				imshow("Live Feed", window);
				waitKey(0);

				ASIGetControlValue(ZWOCamera->CameraID, ASI_EXPOSURE, curr_exposure, exposure_auto);
				ASIGetControlValue(ZWOCamera->CameraID, ASI_GAIN, curr_gain, gain_auto);
				cout << *curr_exposure << endl;
				cout << *curr_gain << endl;

			}
			else {
				cout << "Didn't Get Video Data!" << endl;
				break;
			}
		}
	}
	

	if (ASIStopVideoCapture(ZWOCamera->CameraID) == ASI_SUCCESS) {
		cout << "Camera Video Successfully Stopped" << endl;
	}
	else {
		cout << "Camera Video Didn't Stop!" << endl;
		return -1;
	}
	

	if (ASICloseCamera(ZWOCamera->CameraID) == ASI_SUCCESS) {
		cout << "Camera Successfully Closed" << endl;
	}
	else {
		cout << "Camera Didn't Close!" << endl;
		return -1;
	}
	
	return 0;
}

void capture(ASI_CAMERA_INFO* ZWOCamera, int captureTime) {
	struct tm date;
	__time64_t timestamp;
	string timestring;
	string image_type = ".png";
	char timebuff[50];
	string buffstring;
	int width = ZWOCamera->MaxWidth;
	int height = ZWOCamera->MaxHeight;

	Mat image(width, height, CV_8UC3);
	while (true) {
		if ((ASIGetVideoData(ZWOCamera->CameraID, image.data, width * height * 1, ASI_EXPOSURE * 2 + 500)) == ASI_SUCCESS) {
			_time64(&timestamp);
			_localtime64_s(&date, &timestamp);
			asctime_s(timebuff, 50, &date);
			timebuff[strlen(timebuff) - 1] = '\0';//gets rid of a newline in the string
			buffstring = timebuff;
			replace(buffstring.begin(), buffstring.end(), ':', '-'); //imwrite won't create files with colons in it
			replace(buffstring.begin(), buffstring.end(), ' ', '-');
			timestring = buffstring + image_type;
			cout << "Video Data Is Captured!" << endl;
			//cout << timestring << endl;
			imwrite(timestring, image);
		}
		else {
				cout << "Didn't Get Video Data!" << endl;
				thread_end = true;
				break;
		}
		//this_thread::sleep_for(chrono::seconds(captureTime));
	}
}

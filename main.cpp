#include <sstream>
#include <string>
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include <opencv/highgui.h>
#include <opencv/cv.h>


#include "opencv2/imgproc/imgproc.hpp"
#include <stdio.h>
#include <stdlib.h>

/*#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/core/core_c.h"
#include "opencv2/core/core.hpp"
#include "opencv2/flann/miniflann.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/highgui/highgui.hpp"*/
//#include "opencv2/contrib/contrib.hpp"

using namespace cv;
using namespace std;


int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS=50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20*20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;
//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";

void on_trackbar( int, void* )
{//This function gets called whenever a
	// trackbar position is changed





}
string intToString(int number){


	std::stringstream ss;
	ss << number;
	return ss.str();
}


void createTrackbars(){
	//create window for trackbars


    namedWindow(trackbarWindowName,0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf( TrackbarName, "H_MIN %d", H_MIN);
	sprintf( TrackbarName, "H_MAX %d", H_MAX);
	sprintf( TrackbarName, "S_MIN %d", S_MIN);
	sprintf( TrackbarName, "S_MAX %d", S_MAX);
	sprintf( TrackbarName, "V_MIN %d", V_MIN);
	sprintf( TrackbarName, "V_MAX %d", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
    createTrackbar( "H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar );
    createTrackbar( "H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar );
    createTrackbar( "S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar );
    createTrackbar( "S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar );
    createTrackbar( "V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar );
    createTrackbar( "V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar );


}


void drawObject(int x, int y,Mat &frame){

	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!

    //UPDATE:JUNE 18TH, 2013
    //added 'if' and 'else' statements to prevent
    //memory errors from writing off the screen (ie. (-25,-25) is not within the window!)

	circle(frame,Point(x,y),20,Scalar(0,255,0),2);
    if(y-25>0)
    line(frame,Point(x,y),Point(x,y-25),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(x,0),Scalar(0,255,0),2);
    if(y+25<FRAME_HEIGHT)
    line(frame,Point(x,y),Point(x,y+25),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(x,FRAME_HEIGHT),Scalar(0,255,0),2);
    if(x-25>0)
    line(frame,Point(x,y),Point(x-25,y),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(0,y),Scalar(0,255,0),2);
    if(x+25<FRAME_WIDTH)
    line(frame,Point(x,y),Point(x+25,y),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(FRAME_WIDTH,y),Scalar(0,255,0),2);

	putText(frame,intToString(x)+","+intToString(y),Point(x,y+30),1,1,Scalar(0,255,0),2);

}
void morphOps(Mat &thresh){

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
    //dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));

	erode(thresh,thresh,erodeElement);
	erode(thresh,thresh,erodeElement);


	dilate(thresh,thresh,dilateElement);
	dilate(thresh,thresh,dilateElement);
	


}
void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed){

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if(numObjects<MAX_NUM_OBJECTS){
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
                if(area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea){
					x = moment.m10/area;
					y = moment.m01/area;
					objectFound = true;
					refArea = area;
				}else objectFound = false;


			}
			//let user know you found an object
			if(objectFound ==true){
				putText(cameraFeed,"Tracking Object",Point(0,50),2,1,Scalar(0,255,0),2);
				//draw object location on screen
				drawObject(x,y,cameraFeed);}

		}else putText(cameraFeed,"TOO MUCH NOISE! ADJUST FILTER",Point(0,50),1,2,Scalar(0,0,255),2);
	}
}

void blobDetection(Mat cameraFeed)
{
	cv::SimpleBlobDetector::Params params;
 
	// Change thresholds
	params.minThreshold = 10;
	params.maxThreshold = 200;
	 
	// Filter by Area.
	params.filterByArea = false;
	params.minArea = 15;
	 
	// Filter by Circularity
	params.filterByCircularity = true;
	params.minCircularity = 0.6;
	params.maxCircularity = 0.9;
	 
	// Filter by Convexity
	params.filterByConvexity = false;
	params.minConvexity = 0.87;
	 
	// Filter by Inertia
	params.filterByInertia = false;
	params.minInertiaRatio = 0.01;

	params.filterByColor = true;
	params.blobColor = 0;

	/*#if CV_MAJOR_VERSION < 3   // If you are using OpenCV 2
 
	  // Set up detector with params
	  SimpleBlobDetector detector(params);
	 
	  // You can use the detector this way
	  // detector.detect( im, keypoints);
	 
	#else
	 
	  // Set up detector with params
	  Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);
	 
	  // SimpleBlobDetector::create creates a smart pointer. 
	  // So you need to use arrow ( ->) instead of dot ( . )
	  // detector->detect( im, keypoints);
	 
	#endif*/
	  //SimpleBlobDetector detector(params);
	cv::Ptr<cv::SimpleBlobDetector> blob_detector = cv::SimpleBlobDetector::create();

	///
	/*vector<MSERParams> pMSER;
	vector<MSERParams>::iterator itMSER;
	itMSER = pMSER.begin();
*/
	// Detect blobs.
	//std::vector<std::vector<cv::KeyPoint> > keypoints;
	std::vector<KeyPoint> keypoints;
	cv::Ptr<cv::BRISK> ptrBrisk = cv::BRISK::create();
	//ptrBrisk->detect(imTmp, objectKeypoints);
	blob_detector->detect( cameraFeed, keypoints);
	std::cout<<"length="<<keypoints.size()<<"\n";

	Mat im_with_keypoints;
	
	//cameraFeed.copyTo(im_with_keypoints);
	//findContours(keypoints,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
	CvSeq* seq;  
	CvMemStorage* storage = cvCreateMemStorage( 0 );  
	  
	cvClearMemStorage( storage );  
	  
	
	// Draw detected blobs as red circles.
	///DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
	
	drawKeypoints( cameraFeed, keypoints, im_with_keypoints, Scalar(0,0,255), DrawMatchesFlags::DEFAULT );
	//drawKeypoints(cameraFeed, keypoints, cameraFeed, Scalar(0,0,255), DrawMatchesFlags::DEFAULT ); 
	 
	//Show blobs
	imshow("keypoints", im_with_keypoints );
	waitKey(5);
	
}
	
int main(int argc, char* argv[])
{
	//some boolean variables for different functionality within this
	//program
    bool trackObjects = false;
    bool useMorphOps = true;
    bool blobDetect = false;
	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	//matrix storage for HSV image
	Mat HSV;
	//matrix storage for binary threshold image
	Mat threshold;
	//matrix storage for blob marked image
	Mat blobImage;
	//x and y values for the location of the object
	int x=0, y=0;
	//create slider bars for HSV filtering
	createTrackbars();
	//video capture object to acquire webcam feed
	VideoCapture capture;
	//open capture object at location zero (default location for webcam)
	///capture.open(0);
	//set height and width of capture frame
	///capture.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
	///capture.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	while(1){
		//store image to matrix
		///capture.read(cameraFeed);
		cameraFeed = imread("/home/aytas32/Desktop/as2.jpg", CV_LOAD_IMAGE_UNCHANGED);;
		//convert frame from BGR to HSV colorspace
		cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
		//filter HSV image between values and store filtered image to
		//threshold matrix
		inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);
		//perform morphological operations on thresholded image to eliminate noise
		//and emphasize the filtered object(s)
		if(useMorphOps)
			morphOps(threshold);
		//pass in thresholded frame to our object tracking function
		//this function will return the x and y coordinates of the
		//filtered object
		if(trackObjects)
			trackFilteredObject(x,y,threshold,cameraFeed);

		//
		//
		if(blobDetect)
			blobDetection(threshold);

		//show frames 
		imshow(windowName2,threshold);
		imshow(windowName,cameraFeed);
		imshow(windowName1,HSV);
		

		//delay 30ms so that screen can refresh.
		waitKey(30);
	}






	return 0;
}
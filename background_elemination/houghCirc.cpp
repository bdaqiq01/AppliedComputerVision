/*
 *
 *  Example by Sam Siewert 
 *
 *  Updated for OpenCV 3.1
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>


using namespace cv;
using namespace std;

#define HRES 640
#define VRES 480
#define SYSTEM_ERROR (-1)

int main( int argc, char** argv )
{
    namedWindow("Capture Example");

    VideoCapture cam0(0);
    Mat frame, gray;
    vector<Vec3f> circles;

   if (!cam0.isOpened())
   {
       exit(SYSTEM_ERROR);
   }

   cam0.set(CAP_PROP_FRAME_WIDTH, 640);
   cam0.set(CAP_PROP_FRAME_HEIGHT, 480);

    while(1)
    {
        cam0.read(frame);
        
        cvtColor(frame, gray, COLOR_BGR2GRAY);

        GaussianBlur(gray, gray, Size(9,9), 2, 2);

        //medianBlur(gray, gray, 5);

        HoughCircles(gray, circles, HOUGH_GRADIENT, 1, gray.rows/8, 100, 50,20, 200);

        printf("circles.size = %ld\n", circles.size());

        for( size_t i = 0; i < circles.size(); i++ )
        {
          Vec3i c = circles[i];
          Point center = Point(c[0], c[1]);
          int radius = c[2];
          // circle center
          circle(frame, center, 1, Scalar(0,100,100), 3, LINE_AA );
          // circle outline
          circle(frame, center, radius, Scalar(255,0,255), 3, LINE_AA );
        }

        // Does not work in OpenCV 3.1
        //cvShowImage("Capture Example", frame);

        imshow("Capture Example", frame);

        char c = waitKey(10);
        if( c == 'q' ) break;
    }

    destroyWindow("Capture Example");
    
};

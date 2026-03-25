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
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#define HRES 640
#define VRES 480
#define ESCAPE_KEY (27)
#define SYSTEM_ERROR (-1)


int main()
{
    VideoCapture cam0(0);
    namedWindow("Capture Example");
    Mat frame;
    Mat gray, canny_frame, cdst;
    vector<Vec4i> lines;

   if (!cam0.isOpened())
   {
       exit(SYSTEM_ERROR);
   }

   cam0.set(CAP_PROP_FRAME_WIDTH, 640);
   cam0.set(CAP_PROP_FRAME_HEIGHT, 480);

    while(1)
    {
        cam0.read(frame);
        Canny(frame, canny_frame, 50, 200, 3);

        cvtColor(canny_frame, cdst, COLOR_GRAY2BGR);
        // cdstP = cdst.clone(); <-- Don't know what this does
        
        cvtColor(frame, gray, COLOR_BGR2GRAY);


/*
    // Standard Hough Line Transform
    HoughLines(canny_frame, lines, 1, CV_PI/180, 150, 0, 0 ); // runs the actual detection
    // Draw the lines
    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        line( cdst, pt1, pt2, Scalar(0,0,255), 3, LINE_AA);
    }

*/
///*
        // Probabilistic Line Transform
        HoughLinesP(canny_frame, lines, 1, CV_PI/180, 50, 50, 10);

        for( size_t i = 0; i < lines.size(); i++ )
        {
          Vec4i l = lines[i];
          line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, LINE_AA);
        }

//*/
        //imshow("Capture Example", frame);

        imshow("Capture Example", cdst);

        char c = waitKey(10);
        if( c == 'q' ) break;
    }

    destroyWindow("Capture Example");
    
};

/*
 *
 *  Example by Sam Siewert 
 *
 *  Updated 12/6/18 for OpenCV 3.1
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

char difftext[20];

int main( int argc, char** argv )
{
    Mat mat_frame, frame_diff, mat_red, mat_green, mat_blue;
    Mat red_diff, green_diff, blue_diff;
    Mat blue_prev, green_prev, red_prev;
    VideoCapture vcap;
    unsigned int blue_diffsum, green_diffsum, red_diffsum, blue_maxdiff, green_maxdiff, red_maxdiff;
    double blue_percent_diff, green_percent_diff, red_percent_diff;
    vector<Mat> prev_colors, colors, final_colors;
    float threshold = 0.5;


    //open the video stream and make sure it's opened
    // "0" is the default video device which is normally the built-in webcam
    if(!vcap.open(0)) 
    {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    }
    else
    {
	   std::cout << "Opened default camera interface" << std::endl;
    }

    while(!vcap.read(mat_frame)) {
	std::cout << "No frame" << std::endl;
	cv::waitKey(33);
    }
	
    //cv::cvtColor(mat_frame, mat_gray, COLOR_BGR2GRAY);
    cv::split(mat_frame, prev_colors);

    //mat_diff = mat_gray.clone();
    //mat_gray_prev = mat_gray.clone();
    blue_diff = prev_colors[0];
	green_diff = prev_colors[1];
	red_diff = prev_colors[2];

	//maxdiff = (mat_diff.cols)*(mat_diff.rows)*255;
    blue_maxdiff = (blue_diff.cols)*(blue_diff.rows)*255;
    green_maxdiff = (green_diff.cols)*(green_diff.rows)*255;
    red_maxdiff = (red_diff.cols)*(red_diff.rows)*255;

    while(1)
    {
		if(!vcap.read(mat_frame)) 
		{
			std::cout << "No frame" << std::endl;
			cv::waitKey();
		}
	
		//cv::cvtColor(mat_frame, mat_gray, COLOR_BGR2GRAY);
		cv::split(mat_frame, colors);

		absdiff(prev_colors[0], colors[0], blue_diff);
		absdiff(prev_colors[1], colors[1], green_diff);
		absdiff(prev_colors[2], colors[2], red_diff);
	//	absdiff(mat_gray_prev, mat_gray, mat_diff);
		
		// worst case sum is resolution * 255
		blue_diffsum = (unsigned int)cv::sum(blue_diff)[0]; // single channel sum
		green_diffsum = (unsigned int)cv::sum(green_diff)[0]; // single channel sum
		red_diffsum = (unsigned int)cv::sum(red_diff)[0]; // single channel sum

		blue_percent_diff = ((double)blue_diffsum / (double)blue_maxdiff)*100.0;
		green_percent_diff = ((double)green_diffsum / (double)green_maxdiff)*100.0;
		red_percent_diff = ((double)red_diffsum / (double)red_maxdiff)*100.0;
		
	       // printf("percent diff=%lf\n", percent_diff);
	       // printf(difftext, "%8d",  diffsum);

	        // tested in ERAU Jetson lab
		if(blue_percent_diff > threshold) cv::putText(blue_diff, difftext, Point(30,30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(200,200,250), 1, CV_AVX);
		if(green_percent_diff > threshold) cv::putText(green_diff, difftext, Point(30,30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(200,200,250), 1, CV_AVX);
		if(red_percent_diff > threshold) cv::putText(red_diff, difftext, Point(30,30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(200,200,250), 1, CV_AVX);


//		TODO: Have to append each final diff value into final_colors vector

		cv::merge(final_colors, frame_diff);

	//	cv::imshow("Gray Example", mat_gray);
	//	cv::imshow("Gray Previous", mat_gray_prev);
		cv::imshow("Color Diff", frame_diff);


	        char c = waitKey(33); // take this out or reduce
	        if( c == 'q' ) break;

	//	mat_gray_prev = mat_gray.clone();
		prev_colors[0] = colors[0];
		prev_colors[1] = colors[1];
		prev_colors[2] = colors[2];
    }

};

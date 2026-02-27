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
    Mat mat_frame, mat_gray;

    vector<Mat> colors, mod_video, final_colors;
    
    
    VideoCapture vcap;
   // unsigned int blue_diffsum, green_diffsum, red_diffsum, blue_maxdiff, green_maxdiff, red_maxdiff;
    //double blue_percent_diff, green_percent_diff, red_percent_diff;


    //open the video stream and make sure it's opened
    // "0" is the default video device which is normally the built-in webcam
    //if(!vcap.open(0))
    if(!vcap.open("Dark-Room-Laser-Spot.mpeg")) 
    {
        std::cout << "Error opening video or file" << std::endl;
        return -1;
    }
    else
    {
	   std::cout << "Opened default camera interface" << std::endl;
    }

	namedWindow("Original", WINDOW_NORMAL);
	resizeWindow("Original", 640, 480);


	if(!vcap.read(mat_frame)) 
	{
		std::cout << "No frame" << std::endl;
		cv::waitKey();
	}

	
	cv::split(mat_frame, colors);	


	Mat zero_channel = Mat::zeros(colors[0].size(), colors[0].type()); 
	final_colors.clear(); 
	final_colors.push_back(zero_channel); // B 
	final_colors.push_back(zero_channel); // G 
	final_colors.push_back(colors[2]); // R 
	cv::merge(final_colors, mat_frame); 
	// Show both windows simultaneously
	cv::cvtColor(mat_frame, mat_gray, COLOR_BGR2GRAY); 
	cv::imshow("Original", mat_frame); 

	cv::imshow("Gray Example", mat_gray);


// TODO: Save frame in list, and add to new VideoCapture object with cv2.VideoWriter
		// May not need this unless we do it all at the end?
		// mod_video.push_back(mat_gray);

		// X264 is for mp4 conversion, parameter might need to be in different format
		int fourcc = VideoWriter::fourcc('m', 'p', '4', 'v');
		VideoWriter gray("output.mp4", fourcc, 30.0, mat_gray.size(), false);


		gray.write(mat_gray);




    while(1)
    {
		if(!vcap.read(mat_frame)) 
		{
			std::cout << "No frame" << std::endl;
			cv::waitKey();
		}
	
		
		cv::split(mat_frame, colors);	


		Mat zero_channel = Mat::zeros(colors[0].size(), colors[0].type()); 
		final_colors.clear(); 
		final_colors.push_back(zero_channel); // B 
		final_colors.push_back(zero_channel); // G 
		final_colors.push_back(colors[2]); // R 
		cv::merge(final_colors, mat_frame); 
		// Show both windows simultaneously
		cv::cvtColor(mat_frame, mat_gray, COLOR_BGR2GRAY); 
		cv::imshow("Original", mat_frame); 

		cv::imshow("Gray Example", mat_gray);


// TODO: Save frame in list, and add to new VideoCapture object with cv2.VideoWriter
		// May not need this unless we do it all at the end?
		// mod_video.push_back(mat_gray);


		gray.write(mat_gray);

	    char c = waitKey(33); // take this out or reduce
	    if( c == 'q' ) break;
	    
    }
    gray.release();

    return 0;

};

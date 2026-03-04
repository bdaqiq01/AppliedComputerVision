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
#include <tuple>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

char difftext[20];


int main( int argc, char** argv )
{
    Mat mat_frame, final_frame;
    vector<uint8_t> output;
    Vec3b* row3;
	uint8_t* out_pntr;
	uint8_t intensity;
	tuple<int, int> x_min(0, 1000), x_max(0, 0), y_min(1000, 0), y_max(0, 0), center(0, 0);
	

    uint8_t spot_threshold = 60;
    
    
    VideoCapture vcap;
   // unsigned int blue_diffsum, green_diffsum, red_diffsum, blue_maxdiff, green_maxdiff, red_maxdiff;
    //double blue_percent_diff, green_percent_diff, red_percent_diff;


    //open the video stream and make sure it's opened
    // "0" is the default video device which is normally the built-in webcam
    //if(!vcap.open(0))
    if(!vcap.open("output.mp4")) 
    {
        std::cout << "Error opening video or file" << std::endl;
        return -1;
    }
    else
    {
	   std::cout << "Opened default camera interface" << std::endl;
    }

//	namedWindow("Original", WINDOW_NORMAL);
//	resizeWindow("Original", 640, 480);
	namedWindow("Final Frame", WINDOW_NORMAL);
//	resizeWindow("Final Frame", 640, 480);

	if(!vcap.read(mat_frame) || mat_frame.empty()) 
	{
		std::cout << "No frame" << std::endl;
		//cv::waitKey();
		return -1;
	}

	
	output.resize(mat_frame.rows * mat_frame.cols);

		get<0>(x_min) = 0;
		get<1>(x_min) = 1000;
		get<0>(x_max) = 0;
		get<1>(x_max) = 0;
		get<0>(y_min) = 1000;
		get<1>(y_min) = 0;
		get<0>(y_max) = 0;
		get<1>(y_max) = 0;

	for (int rows = 0; rows < mat_frame.rows; rows++)
	{

		row3 = mat_frame.ptr<Vec3b>(rows);
		out_pntr = output.data() + rows * mat_frame.cols;

		for (int cols = 0; cols < mat_frame.cols; cols++)
		{
			intensity = (row3[cols][0] + row3[cols][1] + row3[cols][2]) / 3;
			if (intensity > spot_threshold)
			{
				out_pntr[cols] =  255;
				if (cols < get<1>(x_min))
				{
					//get<0>x_min = rows;
					get<1>(x_min) = cols;
				}
				if (cols > get<1>(x_max))
				{
					//get<0>x_max = rows;
					get<1>(x_max) = cols;
				}
				if (rows < get<0>(y_min))
				{
					get<0>(y_min) = rows;
					//get<1>y_min = cols;
				}
				if (rows > get<0>(y_max))
				{
					get<0>(y_max) = rows;
					//get<1>y_max = cols;
				}
			}
			else
			{
				out_pntr[cols] = 0;
			}
		}
	}

	// Calculate COM by finding intersection of linescentroid math of circle
	get<1>(center) = (get<1>(x_min) + get<1>(x_max)) / 2;
	get<0>(center) = (get<0>(y_min) + get<0>(y_max)) / 2;

	// Add crosshairs
	out_pntr = output.data() + get<0>(center) * mat_frame.cols;
	//for (int i = 0; i < 20; i++)
	{
	
		out_pntr[get<1>(center)] = 0;
	}
	
	// Convert back to Mat
	final_frame = Mat(mat_frame.rows, mat_frame.cols, CV_8UC1, output.data());

	// X264 is for mp4 conversion, parameter might need to be in different format
//	int fourcc = VideoWriter::fourcc('m', 'p', '4', 'v');



//	crosshair.write(final_frame);

	// Show both windows simultaneously
//	cv::imshow("Final Frame", final_frame); 

	int frame_count = 0;
    while(1)
    {
		if(!vcap.read(mat_frame)) 
		{
			std::cout << "No frame" << std::endl;
			cv::waitKey();
		}
	

//		imshow("Original", mat_frame);
				get<0>(x_min) = 0;
		get<1>(x_min) = mat_frame.cols;
		get<0>(x_max) = 0;
		get<1>(x_max) = 0;
		get<0>(y_min) = mat_frame.rows;
		get<1>(y_min) = 0;
		get<0>(y_max) = 0;
		get<1>(y_max) = 0;

		
		for (int rows = 0; rows < mat_frame.rows; rows++)
		{
			row3 = mat_frame.ptr<Vec3b>(rows);
			out_pntr = output.data() + rows * mat_frame.cols;

			for (int cols = 0; cols < mat_frame.cols; cols++)
					{
						intensity = (row3[cols][0] + row3[cols][1] + row3[cols][2]) / 3;
						if (intensity > spot_threshold)
						{
							out_pntr[cols] =  255;
							if (cols < get<1>(x_min))
							{
								//get<0>x_min = rows;
								get<1>(x_min) = cols;
							}
							if (cols > get<1>(x_max))
							{
								//get<0>x_max = rows;
								get<1>(x_max) = cols;
							}
							if (rows < get<0>(y_min))
							{
								get<0>(y_min) = rows;
								//get<1>y_min = cols;
							}
							if (rows > get<0>(y_max))
							{
								get<0>(y_max) = rows;
								//get<1>y_max = cols;
							}
						}
						else
						{
							out_pntr[cols] = 0;
						}

					}
		}

		//get<1>(center) = cx
		//get<0>(center) = cy
		

		get<1>(center) = (get<1>(x_min) + get<1>(x_max)) / 2;
		get<0>(center) = (get<0>(y_min) + get<0>(y_max)) / 2;

		// Add crosshairs
		out_pntr = output.data() + get<0>(center) * mat_frame.cols;
		for (int x = get<1>(x_min); x <= get<1>(x_max); x++)
		{
		
			out_pntr[x] = 150;
		}

		for (int y = get<0>(y_min); y <= get<0>(y_max); y++)
		{
		
			output.data()[y * mat_frame.cols + get<1>(center)] = 150;
		}
	 
	 

		// Convert back to Mat
		final_frame = Mat(mat_frame.rows, mat_frame.cols, CV_8UC1, output.data());

//		crosshair.write(final_frame);

		cv::imshow("Final Frame", final_frame);		

		char filename[64];
		snprintf(filename, sizeof(filename), "../frames/frame_%04d.png", frame_count++);
		cv::imwrite(filename, final_frame);
	    char c = waitKey(33); // take this out or reduce
	    if( c == 'q' ) break;
	    
    }
//    crosshair.releaqse();

    return 0;

};

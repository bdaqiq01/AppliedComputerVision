/*
 *
 *  Example by Sam Siewert 
 *
 *  Created for OpenCV 4.x for Jetson Nano 2g, based upon
 *  https://docs.opencv.org/4.1.1
 *
 *  Tested with JetPack 4.6 which installs OpenCV 4.1.1
 *  (https://developer.nvidia.com/embedded/jetpack)
 *
 *  Based upon earlier simpler-capture examples created
 *  for OpenCV 2.x and 3.x (C and C++ mixed API) which show
 *  how to use OpenCV instead of lower level V4L2 API for the
 *  Linux UVC driver.
 *
 *  Verify your hardware and OS configuration with:
 *  1) lsusb
 *  2) ls -l /dev/video*
 *  3) dmesg | grep UVC
 *
 *  Note that OpenCV 4.x only supports the C++ API
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <time.h>
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include <iostream>
using namespace cv;
using namespace std;

// See www.asciitable.com
#define ESCAPE_KEY (27)
#define SYSTEM_ERROR (-1)

static const char* CANNY_WIN = "Edge Map";
static int lowThreshold = 50;              // start somewhere visible
static const int max_lowThreshold = 100;
static const int cannyRatio = 3;
static const int kernel_size = 3;

static Mat g_src;   // latest frame (BGR) for callback to use
static Mat g_canny_out; 
static void onCannyTrackbar(int, void*);


static void initCannyUIOnce()
{
    static bool inited = false;
    if (inited) return;
    inited = true;
    createTrackbar("Min Threshold:", "video_display", &lowThreshold, max_lowThreshold, onCannyTrackbar);
}



static void onCannyTrackbar(int, void*)
{
   if (g_src.empty()) return;
   
       Mat gray, edges, dst;
       cvtColor(g_src, gray, COLOR_BGR2GRAY);
       blur(gray, edges, Size(3,3));
       Canny(edges, edges, lowThreshold, lowThreshold * cannyRatio, kernel_size);
   
       dst = Scalar::all(0);
       g_src.copyTo(dst, edges);
       g_canny_out = dst;
}



Mat canny_main(const Mat& frame)
{

  initCannyUIOnce();
  
  // Update the frame used by the slider callback
  g_src = frame;

  // Render using current threshold (so it updates each frame too)
  onCannyTrackbar(0, 0);

  return g_canny_out.empty() ? frame : g_canny_out;

}

Mat sobel_main(Mat image, int scale, int ksize, int delta)
{
  Mat src, src_gray;
  Mat grad;

//  Cout statement is disgnostic to display what settings are currently set to
//	cout << "Scale: " << scale << " Ksize: " << ksize << " Delta: " << delta << endl;
//  int ksize = parser.get<int>("ksize");
//  int scale = parser.get<int>("scale");
//  int delta = parser.get<int>("delta");
  int ddepth = CV_16S;
//  int scale =  1;
//  int ksize = -1;
//  int delta =  0;
  
//  String imageName = parser.get<String>("@input");
  // As usual we load our source image (src)
//  image = imread( imageName, IMREAD_COLOR ); // Load an image
  // Check if image is loaded fine
//  if( image.empty() )
//  {
//    printf("Error opening image: %s\n", imageName.c_str());
//    return 1;
//  }
//  for (;;)
//  {
    // Remove noise by blurring with a Gaussian filter ( kernel size = 3 )
    GaussianBlur(image, src, Size(3, 3), 0, 0, BORDER_DEFAULT);
    // Convert the image to grayscale
    cvtColor(src, src_gray, COLOR_BGR2GRAY);
    Mat grad_x, grad_y;
    Mat abs_grad_x, abs_grad_y;
    Sobel(src_gray, grad_x, ddepth, 1, 0, ksize, scale, delta, BORDER_DEFAULT);
    Sobel(src_gray, grad_y, ddepth, 0, 1, ksize, scale, delta, BORDER_DEFAULT);
    // converting back to CV_8U
    convertScaleAbs(grad_x, abs_grad_x);
    convertScaleAbs(grad_y, abs_grad_y);
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);
//    imshow("Sobel Operator", grad);

//  }
  return grad;
}

int main()
{
   VideoCapture cam0(0); //device id 
   namedWindow("video_display");
   char winInput;
   char flag = 'v';

   if (!cam0.isOpened())
   {
       exit(SYSTEM_ERROR);
   }

   cam0.set(CAP_PROP_FRAME_WIDTH, 640);
   cam0.set(CAP_PROP_FRAME_HEIGHT, 480);
	clockid_t clk = CLOCK_MONOTONIC;
	
  
   int counter = 0;
   int frameNum = 60;
   struct timespec start;  
   clock_gettime(clk, &start); 
   int scale = 1;
   int ksize = -1;
   int delta = 0;

   while (1)
   {
      Mat frame;
      counter +=1;
     
      if ((counter % frameNum)==0)
      {
      	struct timespec end;
      	clock_gettime(clk, &end);
        time_t seconds = end.tv_sec  - start.tv_sec;
    	long nanoseconds = end.tv_nsec - start.tv_nsec;
    	

    	if (nanoseconds < 0) 
    	{
        	seconds  -= 1;
        	nanoseconds += 1000000000L;
    	}

    	double final_time = (double)counter / ((double)seconds + (double)nanoseconds / 1e9);
		cout << "FPS: " << final_time << endl;
		
      }
      
      cam0.read(frame);
      //imshow("video_display", frame);
      winInput = (char)waitKey(1);
		if (winInput == ESCAPE_KEY)
		    break;
		
		// ---- MODE SWITCHING ----
		if (winInput == 'c')
		{
			flag = 'c';
			printf("Canny Mode Enabled\n");
		} 
		if (winInput == 's') 
		{
			flag = 's';
			printf("Sobel Mode Enabled\n");
		}
		if (winInput == 'v') 
		{
			printf("Video Capture Enabled\n");
		}
		
		// ---- APPLY CURRENT MODE ----
		if (flag == 'c')
		{
		    frame = canny_main(frame);
		}
		else if (flag == 's')
		{
		    // Adjust Sobel parameters using the same winInput
		    if (winInput == 'k') ksize = ksize < 30 ? ksize-2 : +1;
		    if (winInput == 'K') ksize = ksize < 30 ? ksize+2 : -1;
		    if (winInput == 'i') scale--;
		    if (winInput == 'I') scale++;
		    
		    if (winInput == 'd') delta--;
		    if (winInput == 'D') delta++;
		    if (winInput == 'r' || winInput == 'R')
		    {
		        scale = 1;
		        ksize = -1;
		        delta = 0;
		    }
		
		    frame = sobel_main(frame, scale, ksize, delta);
		}
      

      /*
	Original code that we prettied up to make easier to understandflag = 'c';
      if ((winInput = waitKey(1)) == ESCAPE_KEY)
      {
          break;
      }
      else if((winInput = waitKey(1)) == 'c' || flag == 'c')
      {
      		flag = 'c';
      		
      		frame = canny_main(frame);
      }
      else if ((winInput = waitKey(1)) == 's' || flag == 's')
      {
      		flag = 's';

      		char key = (char)waitKey(1);
      		
		    if (key == 'k')
		    {
		      ksize = ksize < 30 ? ksize-2 : +1;
		    }
		    // May have to fix this
		    if (key == 'K')
		    {
		      ksize = ksize < 30 ? ksize+2 : -1;    	
		    }
		    if (key == 's')
		    {
		      	scale--;
		    }
		    if (key == 'S')
		    {
		    	scale++;
		    }
		    if (key == 'd')
		    {
		      delta--;
		    }
		    if (key == 'D')
		    {
		    	delta++;
		    }
		    if (key == 'r' || key == 'R')
		    {
		      scale =  1;
		      ksize = -1;
		      delta =  0;
		    }
		    if (key == 'v')
		    {
		    	flag = 'v';
		    }
      		
      		frame = sobel_main(frame, scale, ksize, delta);
      }
      else if(winInput == 'n')
      {
	  	cout << "input " << winInput << " ignored" << endl;
      }
      */
      
      imshow("video_display", frame);
      
   }

   destroyWindow("video_display"); 
};




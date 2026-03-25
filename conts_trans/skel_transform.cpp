#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>

using namespace cv;
using namespace std;

#define ESCAPE_KEY      (27)
#define SYSTEM_ERROR    (-1)
#define HRES            640
#define VRES            480
#define MAX_FRAMES      3000
#define MAX_SKEL_ITERS  100
#define FPS             30.0

static Mat computeSkeleton(const Mat& binary) //Takes a while and black, the white is the skeleton whcih willbe reduced to one line 
{
    Mat skel(binary.size(), CV_8UC1, Scalar(0)); // skeleton image empty at start
    Mat src = binary.clone(); //clone the binary image to not modifying the original
    Mat temp, eroded; //temporary and eroded images for the skeleton
    Mat element = getStructuringElement(MORPH_CROSS, Size(3, 3));
    bool done;
    int iterations = 0;

    do {
        erode(src, eroded, element); //shrnk the white part of the image by 1 pixel ignore the pizels above and below left and right are ignored
        dilate(eroded, temp, element); //dilute back but only 1 pixel in all directions, the outer ring is not recreated
        subtract(src, temp, temp); //subtract the temp image from the src image to get the skeleton slecting the ring that was removed above
        bitwise_or(skel, temp, skel); //add the temp image to the skel image to get the skeleton
        eroded.copyTo(src); //copy the eroded image to the src image to continue the process
        done = (countNonZero(src) == 0); //check if the src image is empty
        iterations++;
    } while (!done && (iterations < MAX_SKEL_ITERS));

    return skel;
}

int main(int argc, char** argv)
{
    VideoCapture cam0(0);
    if (!cam0.isOpened())
    {
        cerr << "Error: Cannot open camera device 0." << endl;
        return SYSTEM_ERROR;
    }
    cam0.set(CAP_PROP_FRAME_WIDTH,  HRES);
    cam0.set(CAP_PROP_FRAME_HEIGHT, VRES);


    system("mkdir -p frames");

    VideoWriter vwriter; //video writer to save the skeleton video
    int fourcc = VideoWriter::fourcc('m', 'p', '4', 'v');
    vwriter.open("skeleton_output.mp4", fourcc, FPS, Size(HRES * 2, VRES), true);
    if (!vwriter.isOpened())
        cerr << "Warning: Could not open VideoWriter. JPEG frames will still be saved." << endl;

    namedWindow("Original", WINDOW_AUTOSIZE);
    namedWindow("Skeleton", WINDOW_AUTOSIZE);

    int frame_count = 0;

    while (1)
    {
        Mat frame;
        if (!cam0.read(frame) || frame.empty())
        {
            cerr << "Warning: Failed to read frame from camera." << endl;
            break;
        }

        Mat hsv, skin_mask, mfblur;
        cvtColor(frame, hsv, COLOR_BGR2HSV);

        Mat lower_mask, upper_mask;
        inRange(hsv, Scalar(0,  30, 60), Scalar(20, 180, 255), lower_mask);
        inRange(hsv, Scalar(160, 30, 60), Scalar(180, 180, 255), upper_mask);
        bitwise_or(lower_mask, upper_mask, skin_mask);

        Mat close_elem = getStructuringElement(MORPH_ELLIPSE, Size(7, 7));
        morphologyEx(skin_mask, skin_mask, MORPH_CLOSE, close_elem);

        medianBlur(skin_mask, mfblur, 7);

        Mat skel = computeSkeleton(mfblur);

        imshow("Original", frame);
        imshow("Skeleton", skel);

        if (frame_count < MAX_FRAMES)
        {
            ostringstream fname;
            fname << "frames/frame_" << setw(4) << setfill('0') << frame_count << ".jpg";
            imwrite(fname.str(), skel);
        }

        if (vwriter.isOpened())
        {
            Mat skel_bgr, combined;
            cvtColor(skel, skel_bgr, COLOR_GRAY2BGR);
            hconcat(frame, skel_bgr, combined);
            vwriter.write(combined);
        }

        frame_count++;
        if (frame_count >= MAX_FRAMES)
        {
            cout << "Reached " << MAX_FRAMES << " frames. Stopping." << endl;
            break;
        }

        char c = (char)waitKey(1);
        if (c == 'q' || c == ESCAPE_KEY) break;
    }

    cout << "Total frames captured: " << frame_count << endl;
    cam0.release();
    if (vwriter.isOpened()) vwriter.release();
    destroyAllWindows();
    return 0;
}

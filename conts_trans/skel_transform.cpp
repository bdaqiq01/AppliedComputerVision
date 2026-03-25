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
    int manual_thresh = -1;
    if (argc >= 2)
        manual_thresh = atoi(argv[1]);

    VideoCapture cam0(0);
    if (!cam0.isOpened())
    {
        cerr << "Error: Cannot open camera device 0." << endl;
        return SYSTEM_ERROR;
    }
    cam0.set(CAP_PROP_FRAME_WIDTH,  HRES); //camera resolution width
    cam0.set(CAP_PROP_FRAME_HEIGHT, VRES); //camera resolution height

    if (manual_thresh >= 0)
        cout << "Manual threshold: " << manual_thresh << " (inverted)" << endl;
    else
        cout << "Using Otsu auto-threshold (inverted)" << endl;


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

        Mat gray, binary, mfblur;
        cvtColor(frame, gray, COLOR_BGR2GRAY); //convert the frame to grayscale

        int thresh_type = THRESH_BINARY_INV; //above threshold is black, below threshold is white for skeleten implementation
        int t = (manual_thresh >= 0) ? manual_thresh : 0; //threshold value
        if (manual_thresh < 0) //if manual threshold is below 0, use Otsu thresholding
            thresh_type |= THRESH_OTSU; //if manual threshold is not provided, use Otsu thresholding
 
        threshold(gray, binary, t, 255, thresh_type); //apply thresholding
        medianBlur(binary, mfblur, 3); //apply median blur

        Mat skel = computeSkeleton(mfblur); //compute the skeleton

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

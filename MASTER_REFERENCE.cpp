/*
 * ============================================================
 *   MASTER OPENCV REFERENCE SCRIPT — MIDTERM STUDY GUIDE
 *   Covers ALL functionality from:
 *     - background_elemination/  (bg_elim, bg_elim_stream,
 *       bottom_up, conv_gray, houghCirc, capture_HU, sobel, canny, capture)
 *     - edge_detection/          (sobel, canny, capture, super_capture)
 *
 *   Sections:
 *     0.  HEADERS & NAMESPACES
 *     1.  KEY DATA TYPES (Mat, Vec, Scalar, Point, Size)
 *     2.  IMAGE DEPTH / TYPE CONSTANTS
 *     3.  READING AN IMAGE FROM DISK
 *     4.  WRITING / SAVING AN IMAGE TO DISK
 *     5.  VIDEO CAPTURE — camera & file
 *     6.  VIDEO WRITER — saving processed video
 *     7.  WINDOW MANAGEMENT  (namedWindow, imshow, waitKey …)
 *     8.  TRACKBARS  (createTrackbar + callback)
 *     9.  COLOR CONVERSION  (cvtColor)
 *    10.  CHANNEL SPLIT & MERGE
 *    11.  BLURRING / NOISE REDUCTION
 *    12.  EDGE DETECTION — SOBEL
 *    13.  EDGE DETECTION — CANNY
 *    14.  BACKGROUND ELIMINATION (frame differencing + threshold)
 *    15.  HISTOGRAM (calcHist + normalize + draw)
 *    16.  HOUGH LINES  (standard & probabilistic)
 *    17.  HOUGH CIRCLES
 *    18.  DRAWING PRIMITIVES (line, circle, putText, rectangle)
 *    19.  MANUAL PIXEL ACCESS  (ptr<>, at<>, Vec3b)
 *    20.  BOTTOM-UP BOUNDING BOX + CENTROID  (manual loop)
 *    21.  FPS MEASUREMENT  (POSIX clock_gettime)
 *    22.  COMMAND-LINE PARSER
 *    23.  CMAKE BUILD SYSTEM CHEAT-SHEET
 *    24.  FULL MINI-TEMPLATES (copy-paste skeletons)
 * ============================================================
 */

// ─────────────────────────────────────────────────────────────
// 0. HEADERS & NAMESPACES
// ─────────────────────────────────────────────────────────────
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <tuple>           // std::tuple, std::get<>

// Core OpenCV — always needed
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>   // imshow, waitKey, VideoCapture, VideoWriter
#include <opencv2/imgproc/imgproc.hpp>   // cvtColor, Canny, Sobel, threshold …
#include <opencv2/imgcodecs.hpp>         // imread, imwrite (sometimes included via highgui)

// Optional — timing
#include <time.h>          // clock_gettime, struct timespec, CLOCK_MONOTONIC

using namespace cv;   // lets you write Mat instead of cv::Mat
using namespace std;  // lets you write cout instead of std::cout

/*
 * IMPORTANT: OpenCV 4.x ONLY has the C++ API.
 *            OpenCV 2/3 had a C API (IplImage*, cvXxx) — avoid it.
 */


// ─────────────────────────────────────────────────────────────
// 1. KEY DATA TYPES
// ─────────────────────────────────────────────────────────────
void dataTypes_demo()
{
    // --- Mat ---
    // The fundamental image/matrix container.
    // Mat = smart pointer; assignment shares data. Use .clone() for deep copy.
    Mat img;                                   // empty
    Mat zeros = Mat::zeros(480, 640, CV_8UC3); // black BGR image
    Mat ones  = Mat::ones (480, 640, CV_8UC1); // all-1 grayscale
    Mat copy  = img.clone();                   // independent deep copy

    // Useful properties:
    //   img.rows       -> height in pixels
    //   img.cols       -> width  in pixels
    //   img.channels() -> 1 (gray) or 3 (BGR)
    //   img.type()     -> e.g. CV_8UC3
    //   img.size()     -> Size(cols, rows)
    //   img.empty()    -> true if no data

    // --- Scalar ---
    // Holds up to 4 values — used for pixel colors and fill values
    Scalar black(0);                // grayscale 0
    Scalar white(255);              // grayscale 255
    Scalar blue (255, 0, 0);        // BGR blue
    Scalar green(0,   255, 0);      // BGR green
    Scalar red  (0,   0,   255);    // BGR red  ← NOTE: OpenCV is BGR, not RGB!
    Scalar all0 = Scalar::all(0);   // fill with 0 (used to blank a Mat)

    // --- Point ---
    Point pt(100, 200);    // (x=col, y=row)
    Point pt2 = Point(pt.x + 10, pt.y - 5);

    // --- Size ---
    Size sz(640, 480);   // (width, height)

    // --- Vec3b / Vec3f / Vec4i ---
    Vec3b pixel;           // 3-channel uchar pixel (BGR)
    pixel[0] = 255;        // Blue
    pixel[1] = 0;          // Green
    pixel[2] = 128;        // Red

    Vec3f circle_data;     // used by HoughCircles: [x, y, radius]
    Vec4i line_data;       // used by HoughLinesP:  [x1, y1, x2, y2]
}


// ─────────────────────────────────────────────────────────────
// 2. IMAGE DEPTH / TYPE CONSTANTS
// ─────────────────────────────────────────────────────────────
/*
 *  FORMAT: CV_<bit depth><S|U|F>C<channels>
 *
 *  CV_8U   = unsigned  8-bit   (0–255)     ← most common for display
 *  CV_8S   = signed    8-bit   (-128–127)
 *  CV_16S  = signed   16-bit   ← used by Sobel (avoids overflow)
 *  CV_32F  = float    32-bit
 *  CV_64F  = double   64-bit
 *
 *  C1 = 1 channel (grayscale)
 *  C3 = 3 channels (BGR color)
 *  C4 = 4 channels (BGRA)
 *
 *  Examples:
 *    CV_8UC1  — grayscale image
 *    CV_8UC3  — color (BGR) image
 *    CV_16SC1 — single-channel 16-bit signed (Sobel intermediate)
 *
 *  convertScaleAbs(src, dst)  → converts any depth back to CV_8U (abs + scale)
 */


// ─────────────────────────────────────────────────────────────
// 3. READING AN IMAGE FROM DISK  (imread)
// ─────────────────────────────────────────────────────────────
void readImage_demo()
{
    // imread(path, flag)
    //   IMREAD_COLOR     → load as 3-channel BGR  (default)
    //   IMREAD_GRAYSCALE → load as 1-channel gray
    //   IMREAD_UNCHANGED → load as-is (including alpha if present)
    Mat img = imread("photo.jpg", IMREAD_COLOR);

    if (img.empty())
    {
        printf("Error: could not open image\n");
        return;
    }

    // Using CommandLineParser to get path from argv (see section 22)
}


// ─────────────────────────────────────────────────────────────
// 4. WRITING / SAVING AN IMAGE TO DISK  (imwrite)
// ─────────────────────────────────────────────────────────────
void writeImage_demo(const Mat& frame, int frame_count)
{
    // imwrite(filename, image)  → returns true on success
    imwrite("output.png", frame);

    // Save numbered frames using snprintf
    char filename[64];
    snprintf(filename, sizeof(filename), "../frames/frame_%04d.png", frame_count);
    // %04d → zero-padded 4-digit number  e.g. frame_0042.png
    cv::imwrite(filename, frame);
}


// ─────────────────────────────────────────────────────────────
// 5. VIDEO CAPTURE — camera & file  (VideoCapture)
// ─────────────────────────────────────────────────────────────
void videoCapture_demo()
{
    // ── Option A: Open default webcam (device index 0) ──
    VideoCapture cam0(0);              // constructor shorthand
    // OR:
    VideoCapture vcap;
    vcap.open(0);                      // equivalent

    // ── Option B: Open a video file ──
    if (!vcap.open("Dark-Room-Laser-Spot.mpeg"))
    {
        cout << "Error opening video or file" << endl;
        return;
    }

    // ── Check if opened ──
    if (!cam0.isOpened())
    {
        exit(-1);   // or return -1 from main
    }

    // ── Set resolution (must be done BEFORE reading) ──
    cam0.set(CAP_PROP_FRAME_WIDTH,  640);
    cam0.set(CAP_PROP_FRAME_HEIGHT, 480);
    // Other useful CAP_PROP values:
    //   CAP_PROP_FPS       → set/get frames per second
    //   CAP_PROP_POS_FRAMES → current frame number (for files)
    //   CAP_PROP_FRAME_COUNT → total frames in file

    // ── Read a single frame ──
    Mat frame;
    cam0.read(frame);
    // OR equivalently:
    // cam0 >> frame;

    // ── Read in a loop ──
    while (1)
    {
        Mat f;
        if (!cam0.read(f))         // returns false at end-of-file or on error
        {
            cout << "No frame" << endl;
            cv::waitKey();         // pause
            break;
        }
        // process f …
        imshow("window", f);
        if (waitKey(33) == 'q') break;   // ~30 fps cap
    }

    // ── No explicit release needed (destructor handles it), but can call: ──
    // cam0.release();
}


// ─────────────────────────────────────────────────────────────
// 6. VIDEO WRITER — saving processed video  (VideoWriter)
// ─────────────────────────────────────────────────────────────
void videoWriter_demo(const Mat& first_gray_frame)
{
    // fourcc = four-character codec code
    int fourcc = VideoWriter::fourcc('m', 'p', '4', 'v');  // .mp4
    // Other common codecs:
    //   VideoWriter::fourcc('X','2','6','4')  → H.264
    //   VideoWriter::fourcc('M','J','P','G')  → Motion JPEG (.avi)
    //   VideoWriter::fourcc('P','I','M','1')  → MPEG-1

    double fps = 30.0;

    // VideoWriter(filename, fourcc, fps, frameSize, isColor)
    //   isColor = false → grayscale   (must match what you write!)
    //   isColor = true  → BGR color
    VideoWriter writer("output.mp4", fourcc, fps, first_gray_frame.size(), false);

    // Write a frame
    writer.write(first_gray_frame);
    // OR:  writer << frame;

    // ── Always release when done ──
    writer.release();

    // IMPORTANT:
    //   frameSize must exactly match the size of frames you write.
    //   If isColor=false, write grayscale (CV_8UC1) frames only.
    //   If isColor=true,  write BGR (CV_8UC3) frames only.
}


// ─────────────────────────────────────────────────────────────
// 7. WINDOW MANAGEMENT
// ─────────────────────────────────────────────────────────────
void windowManagement_demo(const Mat& frame)
{
    // Create a named window (must exist before imshow or createTrackbar)
    namedWindow("My Window");                    // default flag
    namedWindow("My Window2", WINDOW_NORMAL);    // resizable
    namedWindow("My Window3", WINDOW_AUTOSIZE);  // auto-fits image (default)

    // Resize a WINDOW_NORMAL window
    resizeWindow("My Window", 640, 480);

    // Display frame in window
    imshow("My Window", frame);

    // waitKey(ms) → waits ms milliseconds for a key press
    //   returns the ASCII value of the key pressed, or -1 if timeout
    //   waitKey(0)  → wait forever until a key is pressed
    //   waitKey(33) → ~30 fps (1000ms / 30 ≈ 33ms per frame)
    //   waitKey(10) → ~100 fps, used in some examples
    //   waitKey(1)  → minimal delay (used in super_capture for responsiveness)
    char key = (char)waitKey(33);
    if (key == 'q')     { /* quit  */ }
    if (key == 27)      { /* ESC   */ }  // ASCII 27 = ESC
    if (key == 'c')     { /* canny */ }
    if (key == 's')     { /* sobel */ }

    // Destroy windows
    destroyWindow("My Window");   // one window
    destroyAllWindows();          // all windows
}


// ─────────────────────────────────────────────────────────────
// 8. TRACKBARS  (interactive sliders)
// ─────────────────────────────────────────────────────────────

// Trackbar callback signature: void callback(int value, void* userdata)
// The variable it controls is updated AUTOMATICALLY by OpenCV;
// the callback is called every time the slider moves.

static int g_threshold = 50;
static const int g_max_threshold = 100;

static void onThresholdChange(int /*newVal*/, void* /*userdata*/)
{
    // 'g_threshold' is already updated by OpenCV before this is called
    // Do processing here using the new g_threshold value
}

void trackbar_demo()
{
    namedWindow("My Window");

    // createTrackbar(trackbarName, windowName, &variable, maxValue, callback)
    //   &variable → pointer to int that is automatically updated
    //   callback  → called each time slider moves (can be NULL)
    createTrackbar("Min Threshold:", "My Window",
                   &g_threshold, g_max_threshold, onThresholdChange);

    // Manually call callback once to initialize display
    onThresholdChange(0, 0);

    waitKey(0);
}


// ─────────────────────────────────────────────────────────────
// 9. COLOR CONVERSION  (cvtColor)
// ─────────────────────────────────────────────────────────────
void colorConversion_demo(const Mat& bgr_frame)
{
    Mat gray, hsv, bgr2;

    // BGR → Grayscale (most common — required before Canny, Sobel, HoughCircles)
    cvtColor(bgr_frame, gray, COLOR_BGR2GRAY);

    // Grayscale → BGR (needed to draw colored things on a gray result)
    cvtColor(gray, bgr2, COLOR_GRAY2BGR);

    // BGR → HSV (useful for color-based segmentation)
    cvtColor(bgr_frame, hsv, COLOR_BGR2HSV);

    // NOTE: OpenCV ALWAYS uses BGR order, NOT RGB.
    //       channels[0]=Blue, channels[1]=Green, channels[2]=Red
}


// ─────────────────────────────────────────────────────────────
// 10. CHANNEL SPLIT & MERGE
// ─────────────────────────────────────────────────────────────
void splitMerge_demo(const Mat& bgr_frame)
{
    // ── Split BGR frame into 3 separate single-channel Mats ──
    vector<Mat> channels;
    cv::split(bgr_frame, channels);
    // channels[0] → Blue  plane
    // channels[1] → Green plane
    // channels[2] → Red   plane

    // ── Modify channels (e.g. zero-out blue and green) ──
    Mat zero = Mat::zeros(channels[0].size(), channels[0].type());

    // ── Merge back into a 3-channel image ──
    vector<Mat> out_channels;
    out_channels.push_back(zero);         // Blue  = 0
    out_channels.push_back(zero);         // Green = 0
    out_channels.push_back(channels[2]);  // Red   = original red channel

    Mat red_only;
    cv::merge(out_channels, red_only);

    // Keep previous frame channels for differencing:
    // prev_channels[i] = channels[i].clone();  // deep copy!
}


// ─────────────────────────────────────────────────────────────
// 11. BLURRING / NOISE REDUCTION
// ─────────────────────────────────────────────────────────────
void blurring_demo(const Mat& src, Mat& dst)
{
    // ── Gaussian Blur (most common noise reducer before Sobel/Canny) ──
    // GaussianBlur(src, dst, kernelSize, sigmaX, sigmaY, borderType)
    //   kernelSize must be ODD and positive  (e.g. Size(3,3), Size(9,9))
    //   sigma=0 → auto-compute from kernel size
    GaussianBlur(src, dst, Size(3, 3), 0, 0, BORDER_DEFAULT);
    GaussianBlur(src, dst, Size(9, 9), 2, 2);  // used before HoughCircles

    // ── Simple average blur (used in Canny preprocessing) ──
    // blur(src, dst, kernelSize)
    blur(src, dst, Size(3, 3));

    // ── Median Blur (good for salt-and-pepper noise) ──
    // medianBlur(src, dst, ksize)   ksize must be ODD
    medianBlur(src, dst, 5);
}


// ─────────────────────────────────────────────────────────────
// 12. EDGE DETECTION — SOBEL
// ─────────────────────────────────────────────────────────────
/*
 * THEORY:
 *   Sobel computes the image gradient (rate of intensity change).
 *   It runs TWO 1D derivative filters:
 *     - grad_x: horizontal edges  (dx=1, dy=0)
 *     - grad_y: vertical edges    (dx=0, dy=1)
 *   Then combines them: gradient = 0.5*|grad_x| + 0.5*|grad_y|
 *
 *   ksize=-1 → uses Scharr filter (more accurate for small kernels)
 *   ksize=1,3,5,7 → standard Sobel
 */
Mat sobel_demo(Mat image, int scale = 1, int ksize = -1, int delta = 0)
{
    Mat src, src_gray;
    Mat grad_x, grad_y;
    Mat abs_grad_x, abs_grad_y;
    Mat grad;
    int ddepth = CV_16S;  // 16-bit signed to avoid overflow during derivative

    // Step 1: Blur to remove noise
    GaussianBlur(image, src, Size(3, 3), 0, 0, BORDER_DEFAULT);

    // Step 2: Convert to grayscale (Sobel works on single-channel)
    cvtColor(src, src_gray, COLOR_BGR2GRAY);

    // Step 3: Compute X and Y gradients
    // Sobel(src, dst, ddepth, dx, dy, ksize, scale, delta, borderType)
    //   dx=1, dy=0 → x-direction derivative (detects vertical edges)
    //   dx=0, dy=1 → y-direction derivative (detects horizontal edges)
    //   scale → amplifier for the derivative values
    //   delta → value added to each result pixel
    Sobel(src_gray, grad_x, ddepth, 1, 0, ksize, scale, delta, BORDER_DEFAULT);
    Sobel(src_gray, grad_y, ddepth, 0, 1, ksize, scale, delta, BORDER_DEFAULT);

    // Step 4: Convert 16S back to 8U using absolute value + scaling
    convertScaleAbs(grad_x, abs_grad_x);
    convertScaleAbs(grad_y, abs_grad_y);

    // Step 5: Combine X and Y gradients
    // addWeighted(src1, alpha, src2, beta, gamma, dst)
    //   dst = alpha*src1 + beta*src2 + gamma
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

    return grad;  // CV_8UC1
}


// ─────────────────────────────────────────────────────────────
// 13. EDGE DETECTION — CANNY
// ─────────────────────────────────────────────────────────────
/*
 * THEORY:
 *   Canny is a multi-stage edge detector:
 *   1. Gaussian smooth to reduce noise
 *   2. Gradient magnitude + direction (Sobel inside)
 *   3. Non-maximum suppression (thin edges to 1px)
 *   4. Hysteresis thresholding with low/high thresholds:
 *        pixel > highThreshold → definitely an edge
 *        pixel < lowThreshold  → definitely NOT an edge
 *        in between            → edge only if connected to a strong edge
 *   Typical ratio: highThreshold = 3 * lowThreshold
 *
 *   Output: binary edge map (0 or 255)
 */
Mat canny_demo(const Mat& bgr_frame, int lowThreshold = 50)
{
    const int ratio = 3;          // high = low * ratio
    const int kernel_size = 3;    // Sobel kernel inside Canny

    Mat gray, edges, dst;

    // Step 1: Grayscale
    cvtColor(bgr_frame, gray, COLOR_BGR2GRAY);

    // Step 2: Blur (simple box blur)
    blur(gray, edges, Size(3, 3));

    // Step 3: Canny
    // Canny(src, dst, lowThreshold, highThreshold, apertureSize)
    Canny(edges, edges, lowThreshold, lowThreshold * ratio, kernel_size);
    // 'edges' is now a binary map (0=no edge, 255=edge)

    // Step 4 (optional): Mask original image — keep only edge pixels
    dst = Scalar::all(0);               // black canvas same size as src
    bgr_frame.copyTo(dst, edges);       // copy src pixels where edges != 0

    return dst;
    // OR just return edges for the raw binary edge map
}


// ─────────────────────────────────────────────────────────────
// 14. BACKGROUND ELIMINATION  (frame differencing + threshold)
// ─────────────────────────────────────────────────────────────
/*
 * IDEA: Compare current frame to previous frame pixel-by-pixel.
 *       Pixels that changed a lot → foreground (moving object).
 *       Pixels that stayed same  → background.
 *
 * PIPELINE:
 *   1. split current & previous frames into channels
 *   2. absdiff per channel
 *   3. threshold the diff → binary mask
 *   4. merge masked channels back to color image for display
 */
void bgElim_demo(
    const vector<Mat>& prev_colors,  // previous frame's channels (B,G,R)
    const vector<Mat>& curr_colors,  // current  frame's channels (B,G,R)
    Mat& frame_diff_out)
{
    Mat blue_diff, green_diff, red_diff;
    Mat red_thresh;

    // ── Step 1: Per-channel absolute difference ──
    // absdiff(src1, src2, dst)  →  dst[i] = |src1[i] - src2[i]|
    absdiff(prev_colors[0], curr_colors[0], blue_diff);
    absdiff(prev_colors[1], curr_colors[1], green_diff);
    absdiff(prev_colors[2], curr_colors[2], red_diff);

    // ── Step 2: Threshold the red difference channel ──
    // threshold(src, dst, thresh, maxVal, type)
    //   THRESH_BINARY: dst[i] = (src[i] > thresh) ? maxVal : 0
    float spot_threshold = 50.0;
    cv::threshold(red_diff, red_thresh, spot_threshold, 255, THRESH_BINARY);

    // ── Step 3: Build output — red channel shows movement, B/G zeroed ──
    Mat zero = Mat::zeros(blue_diff.size(), blue_diff.type());
    vector<Mat> out;
    out.push_back(zero);       // B
    out.push_back(zero);       // G
    out.push_back(red_thresh); // R — shows where motion was detected
    cv::merge(out, frame_diff_out);

    // ── Step 4 (in the loop): Update previous frame ──
    // Use .clone() to make an independent copy!
    // prev_colors[0] = curr_colors[0].clone();  etc.
}

// ── Sum-based change detection (alternative metric) ──
void sumDiff_demo(const Mat& diff_channel)
{
    // cv::sum() returns a Scalar with the sum of each channel
    unsigned int diffsum = (unsigned int)cv::sum(diff_channel)[0];

    int maxdiff = diff_channel.cols * diff_channel.rows * 255; // worst case
    double percent_diff = ((double)diffsum / (double)maxdiff) * 100.0;

    printf("percent diff = %.2f%%\n", percent_diff);
}


// ─────────────────────────────────────────────────────────────
// 15. HISTOGRAM  (calcHist + normalize + draw)
// ─────────────────────────────────────────────────────────────
void histogram_demo(const Mat& channel, const string& win_name)
{
    int binNum = 256;
    float range[] = {0, 256};         // intensity range 0–255
    const float* histRange = {range};
    bool uniform = true;
    bool accumulate = false;

    Mat hist;
    // calcHist(images, numImages, channels, mask, hist, dims, histSize, ranges, uniform, accumulate)
    //   &channel  → array of source images (pass address of Mat)
    //   1         → 1 image
    //   0         → channel index (0 for first/only channel)
    //   Mat()     → no mask (use all pixels)
    //   hist      → output histogram (CV_32F, size = binNum x 1)
    //   1         → 1D histogram
    //   &binNum   → number of bins per dimension
    cv::calcHist(&channel, 1, 0, Mat(), hist, 1, &binNum, &histRange, uniform, accumulate);

    // Optional: zero out bin 0 so unchanged/black pixels don't dominate
    hist.at<float>(0) = 0;

    // Create canvas to draw histogram on
    int hist_w = 1024;
    int hist_h = 400;
    int bin_w = cvRound((double)hist_w / binNum);  // pixel width per bin
    Mat histImage(hist_h, hist_w, CV_8UC1, Scalar(0));  // black canvas

    // Normalize histogram heights to fit in the image
    // normalize(src, dst, alpha, beta, normType)
    //   NORM_MINMAX → scale so min=alpha, max=beta
    cv::normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

    // Draw each bin as a line segment
    for (int i = 1; i < binNum; i++)
    {
        line(histImage,
             Point(bin_w * (i-1), hist_h - cvRound(hist.at<float>(i-1))),
             Point(bin_w * i,     hist_h - cvRound(hist.at<float>(i))),
             Scalar(255), 2, 8, 0);
    }

    cv::imshow(win_name, histImage);
}


// ─────────────────────────────────────────────────────────────
// 16. HOUGH LINES  (standard and probabilistic)
// ─────────────────────────────────────────────────────────────
/*
 * PIPELINE:
 *   1. Grayscale
 *   2. Canny (edge map)
 *   3. cvtColor(canny, cdst, COLOR_GRAY2BGR)  → canvas for drawing
 *   4. HoughLines or HoughLinesP on the Canny edge map
 *   5. Draw detected lines on cdst
 */
void houghLines_demo(const Mat& bgr_frame)
{
    Mat gray, canny_frame, cdst;

    cvtColor(bgr_frame, gray, COLOR_BGR2GRAY);
    Canny(bgr_frame, canny_frame, 50, 200, 3);
    cvtColor(canny_frame, cdst, COLOR_GRAY2BGR);  // color canvas

    // ── A) STANDARD HOUGH TRANSFORM ──
    // Detects INFINITE lines represented as (rho, theta)
    // HoughLines(src, lines, rho, theta, threshold, srn, stn)
    //   rho       = distance resolution in pixels (typically 1)
    //   theta     = angle resolution in radians   (typically CV_PI/180)
    //   threshold = minimum votes to be a line
    vector<Vec2f> lines_std;
    HoughLines(canny_frame, lines_std, 1, CV_PI/180, 150, 0, 0);

    for (size_t i = 0; i < lines_std.size(); i++)
    {
        float rho = lines_std[i][0], theta = lines_std[i][1];
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        Point pt1(cvRound(x0 + 1000*(-b)), cvRound(y0 + 1000*(a)));
        Point pt2(cvRound(x0 - 1000*(-b)), cvRound(y0 - 1000*(a)));
        line(cdst, pt1, pt2, Scalar(0, 0, 255), 3, LINE_AA);
    }

    // ── B) PROBABILISTIC HOUGH TRANSFORM (preferred — gives endpoints) ──
    // Detects LINE SEGMENTS with start and end points
    // HoughLinesP(src, lines, rho, theta, threshold, minLineLen, maxLineGap)
    //   minLineLen = minimum line length in pixels
    //   maxLineGap = maximum gap between collinear segments to merge
    vector<Vec4i> lines_prob;
    HoughLinesP(canny_frame, lines_prob, 1, CV_PI/180, 50, 50, 10);

    for (size_t i = 0; i < lines_prob.size(); i++)
    {
        Vec4i l = lines_prob[i];
        // l[0]=x1, l[1]=y1, l[2]=x2, l[3]=y2
        line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, LINE_AA);
    }

    imshow("Hough Lines", cdst);
}


// ─────────────────────────────────────────────────────────────
// 17. HOUGH CIRCLES
// ─────────────────────────────────────────────────────────────
/*
 * PIPELINE:
 *   1. Convert to grayscale
 *   2. GaussianBlur (REQUIRED — circles are very noise-sensitive)
 *   3. HoughCircles
 *   4. Draw circles on original BGR frame
 */
void houghCircles_demo(Mat frame)
{
    Mat gray;
    vector<Vec3f> circles;  // each circle = (center_x, center_y, radius)

    cvtColor(frame, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, gray, Size(9, 9), 2, 2);  // blur is critical!

    // HoughCircles(src, circles, method, dp, minDist, param1, param2, minRadius, maxRadius)
    //   HOUGH_GRADIENT → only method currently supported
    //   dp             → inverse ratio of accumulator resolution to image (1 = same resolution)
    //   minDist        → minimum distance between detected circle CENTERS
    //   param1         → upper Canny threshold (lower is param1/2)
    //   param2         → accumulator threshold (lower = more circles, more false positives)
    //   minRadius      → minimum circle radius (0 = any)
    //   maxRadius      → maximum circle radius (0 = any, negative = return centers only)
    HoughCircles(gray, circles, HOUGH_GRADIENT,
                 1,              // dp
                 gray.rows / 8,  // minDist between circle centers
                 100,            // param1 (Canny high threshold)
                 50,             // param2 (accumulator threshold)
                 20,             // minRadius
                 200);           // maxRadius

    printf("circles found: %ld\n", circles.size());

    for (size_t i = 0; i < circles.size(); i++)
    {
        Vec3i c = circles[i];
        Point center(c[0], c[1]);
        int radius = c[2];

        // Draw center dot
        circle(frame, center, 1, Scalar(0, 100, 100), 3, LINE_AA);

        // Draw circle outline
        circle(frame, center, radius, Scalar(255, 0, 255), 3, LINE_AA);
    }

    imshow("Hough Circles", frame);
}


// ─────────────────────────────────────────────────────────────
// 18. DRAWING PRIMITIVES
// ─────────────────────────────────────────────────────────────
void drawing_demo(Mat& img)
{
    // ── Line ──
    // line(img, pt1, pt2, color, thickness, lineType, shift)
    line(img, Point(0, 0), Point(100, 100), Scalar(0, 255, 0), 2, LINE_AA);
    // LINE_AA = anti-aliased, LINE_8 = 8-connected (faster, default)

    // ── Circle ──
    // circle(img, center, radius, color, thickness, lineType)
    //   thickness = -1 → filled circle
    circle(img, Point(320, 240), 50, Scalar(255, 0, 0), 3, LINE_AA);  // outline
    circle(img, Point(320, 240),  1, Scalar(0, 100, 100), 3, LINE_AA); // center dot

    // ── Rectangle ──
    // rectangle(img, topLeft, bottomRight, color, thickness)
    rectangle(img, Point(10, 10), Point(200, 100), Scalar(0, 0, 255), 2);

    // ── Text (putText) ──
    // putText(img, text, origin, fontFace, fontScale, color, thickness, lineType)
    cv::putText(img, "Hello CV", Point(30, 30),
                FONT_HERSHEY_COMPLEX_SMALL,  // font
                0.8,                          // scale
                Scalar(200, 200, 250),        // color
                1,                            // thickness
                LINE_AA);
    // Other fonts: FONT_HERSHEY_SIMPLEX, FONT_HERSHEY_PLAIN, FONT_HERSHEY_DUPLEX
}


// ─────────────────────────────────────────────────────────────
// 19. MANUAL PIXEL ACCESS  (ptr<> and at<>)
// ─────────────────────────────────────────────────────────────
void pixelAccess_demo(const Mat& bgr_frame)
{
    // ── Method A: ptr<> (FAST — raw row pointer) ──
    // Use for performance-critical inner loops
    for (int row = 0; row < bgr_frame.rows; row++)
    {
        // ptr<Vec3b>(row) → pointer to first pixel in that row
        // Vec3b = 3-element uchar array (B, G, R)
        Vec3b* rowPtr = bgr_frame.ptr<Vec3b>(row);

        for (int col = 0; col < bgr_frame.cols; col++)
        {
            uchar blue  = rowPtr[col][0];
            uchar green = rowPtr[col][1];
            uchar red   = rowPtr[col][2];

            // Compute intensity as average of channels
            uint8_t intensity = (rowPtr[col][0] + rowPtr[col][1] + rowPtr[col][2]) / 3;
        }
    }

    // ── Method B: at<> (SAFE but slower — bounds-checked) ──
    // Use for accessing individual pixels or histogram values
    float histVal = 0; // example usage
    // histVal = hist.at<float>(i);  // access histogram bin i (float type)

    // ── Flat buffer pointer (bottom_up.cpp pattern) ──
    // Build a flat byte array, then wrap it in a Mat at the end
    vector<uint8_t> output;
    output.resize(bgr_frame.rows * bgr_frame.cols);

    for (int row = 0; row < bgr_frame.rows; row++)
    {
        Vec3b* rowPtr3 = bgr_frame.ptr<Vec3b>(row);
        uint8_t* outRow = output.data() + row * bgr_frame.cols; // flat offset

        for (int col = 0; col < bgr_frame.cols; col++)
        {
            uint8_t intensity = (rowPtr3[col][0] + rowPtr3[col][1] + rowPtr3[col][2]) / 3;
            outRow[col] = (intensity > 60) ? 255 : 0;  // threshold
        }
    }

    // Wrap flat buffer as a Mat (NO copy — shares memory with vector)
    Mat result(bgr_frame.rows, bgr_frame.cols, CV_8UC1, output.data());
    // WARNING: 'result' is only valid while 'output' is alive!
    Mat safe_result = result.clone();  // if you need it to outlive 'output'
}


// ─────────────────────────────────────────────────────────────
// 20. BOTTOM-UP BOUNDING BOX + CENTROID  (from bottom_up.cpp)
// ─────────────────────────────────────────────────────────────
/*
 * GOAL: Find a bright spot in a grayscale-intensity image and
 *       locate its center (centroid) by scanning all pixels manually.
 *
 * Uses std::tuple<int,int> to store (row, col) of extremes:
 *   x_min → leftmost bright pixel  (tuple: (row, col))
 *   x_max → rightmost bright pixel
 *   y_min → topmost bright pixel
 *   y_max → bottommost bright pixel
 */
void bottomUp_demo(const Mat& bgr_frame, vector<uint8_t>& output)
{
    uint8_t spot_threshold = 60;

    // tuple<int,int>: get<0>=row component, get<1>=col component
    tuple<int,int> x_min(0, bgr_frame.cols); // start at rightmost
    tuple<int,int> x_max(0, 0);              // start at leftmost
    tuple<int,int> y_min(bgr_frame.rows, 0); // start at bottommost
    tuple<int,int> y_max(0, 0);              // start at topmost
    tuple<int,int> center(0, 0);

    output.resize(bgr_frame.rows * bgr_frame.cols);

    for (int row = 0; row < bgr_frame.rows; row++)
    {
        Vec3b* rowPtr = bgr_frame.ptr<Vec3b>(row);
        uint8_t* outRow = output.data() + row * bgr_frame.cols;

        for (int col = 0; col < bgr_frame.cols; col++)
        {
            uint8_t intensity = (rowPtr[col][0] + rowPtr[col][1] + rowPtr[col][2]) / 3;

            if (intensity > spot_threshold)
            {
                outRow[col] = 255;  // mark as foreground

                if (col < get<1>(x_min)) get<1>(x_min) = col;  // leftmost col
                if (col > get<1>(x_max)) get<1>(x_max) = col;  // rightmost col
                if (row < get<0>(y_min)) get<0>(y_min) = row;  // topmost row
                if (row > get<0>(y_max)) get<0>(y_max) = row;  // bottommost row
            }
            else
            {
                outRow[col] = 0;  // background
            }
        }
    }

    // Centroid = midpoint of bounding box  (simple, works for convex blobs)
    get<1>(center) = (get<1>(x_min) + get<1>(x_max)) / 2;  // cx
    get<0>(center) = (get<0>(y_min) + get<0>(y_max)) / 2;  // cy

    // Draw horizontal crosshair line
    for (int x = get<1>(x_min); x <= get<1>(x_max); x++)
        output.data()[get<0>(center) * bgr_frame.cols + x] = 150;

    // Draw vertical crosshair line
    for (int y = get<0>(y_min); y <= get<0>(y_max); y++)
        output.data()[y * bgr_frame.cols + get<1>(center)] = 150;

    // Wrap result in Mat
    Mat final_frame(bgr_frame.rows, bgr_frame.cols, CV_8UC1, output.data());
    imshow("Final Frame", final_frame);
}


// ─────────────────────────────────────────────────────────────
// 21. FPS MEASUREMENT  (POSIX clock_gettime)
// ─────────────────────────────────────────────────────────────
void fps_demo()
{
    clockid_t clk = CLOCK_MONOTONIC;  // monotonic = never jumps backward
    struct timespec start, end;
    int counter = 0;
    int frameNum = 60;  // compute FPS every 60 frames

    clock_gettime(clk, &start);

    while (1)
    {
        counter++;

        if ((counter % frameNum) == 0)  // every N frames
        {
            clock_gettime(clk, &end);

            time_t seconds     = end.tv_sec  - start.tv_sec;
            long   nanoseconds = end.tv_nsec - start.tv_nsec;

            // Handle nanosecond borrow
            if (nanoseconds < 0)
            {
                seconds    -= 1;
                nanoseconds += 1000000000L;
            }

            double elapsed = (double)seconds + (double)nanoseconds / 1e9;
            double fps = (double)counter / elapsed;
            cout << "FPS: " << fps << endl;
        }

        // ... capture and process frame ...
        if (waitKey(1) == 27) break;
    }
}


// ─────────────────────────────────────────────────────────────
// 22. COMMAND-LINE PARSER  (cv::CommandLineParser)
// ─────────────────────────────────────────────────────────────
void cmdParser_demo(int argc, char** argv)
{
    // Format: "{name  shortName | defaultValue | description}"
    // "@input" (with @) → positional argument
    cv::CommandLineParser parser(argc, argv,
        "{@input   |../data/lena.jpg | input image  }"
        "{ksize  k | 1              | kernel size  }"
        "{scale  s | 1              | scale factor }"
        "{delta  d | 0              | delta value  }"
        "{help   h | false          | show help    }");

    if (parser.get<bool>("help"))
    {
        parser.printMessage();
        return;
    }

    // Get values (template argument must match the default value's type)
    string imageName = parser.get<String>("@input");
    int    ksize     = parser.get<int>("ksize");
    int    scale     = parser.get<int>("scale");
    int    delta     = parser.get<int>("delta");

    // Usage: ./program image.jpg --ksize=3 --scale=2
}


// ─────────────────────────────────────────────────────────────
// 23. CMAKE BUILD SYSTEM CHEAT-SHEET
// ─────────────────────────────────────────────────────────────
/*
 *  CMakeLists.txt template:
 *  ─────────────────────────────
 *  cmake_minimum_required(VERSION 3.10)
 *  project(MyProject C CXX)
 *  set(CMAKE_CXX_STANDARD 17)
 *
 *  find_package(OpenCV REQUIRED)               ← finds OpenCV on the system
 *  include_directories(${OpenCV_INCLUDE_DIRS}) ← adds header search paths
 *
 *  add_executable(myapp main.cpp)              ← creates binary named 'myapp'
 *  target_link_libraries(myapp ${OpenCV_LIBS}) ← links all OpenCV libraries
 *  ─────────────────────────────
 *
 *  Build commands (run in project root):
 *    mkdir build && cd build
 *    cmake ..          ← configure (reads CMakeLists.txt)
 *    make              ← compile and link
 *    ./myapp           ← run
 *
 *  Rebuild after changes:  cd build && make
 */


// ─────────────────────────────────────────────────────────────
// 24. FULL MINI-TEMPLATES  (copy-paste skeletons)
// ─────────────────────────────────────────────────────────────

// ════════════════════════════════════════
//  TEMPLATE A: Basic webcam display loop
// ════════════════════════════════════════
/*
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;
int main()
{
    VideoCapture cam(0);
    if (!cam.isOpened()) return -1;
    cam.set(CAP_PROP_FRAME_WIDTH, 640);
    cam.set(CAP_PROP_FRAME_HEIGHT, 480);
    namedWindow("display");
    while (1) {
        Mat frame;
        cam.read(frame);
        imshow("display", frame);
        if (waitKey(33) == 27) break;  // ESC to quit
    }
    destroyAllWindows();
}
*/

// ════════════════════════════════════════
//  TEMPLATE B: Sobel on live webcam
// ════════════════════════════════════════
/*
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
using namespace cv;
int main()
{
    VideoCapture cam(0);
    if (!cam.isOpened()) return -1;
    namedWindow("Sobel");
    while (1) {
        Mat frame, gray, src, gx, gy, agx, agy, grad;
        cam.read(frame);
        GaussianBlur(frame, src, Size(3,3), 0, 0, BORDER_DEFAULT);
        cvtColor(src, gray, COLOR_BGR2GRAY);
        Sobel(gray, gx, CV_16S, 1, 0, -1, 1, 0, BORDER_DEFAULT);
        Sobel(gray, gy, CV_16S, 0, 1, -1, 1, 0, BORDER_DEFAULT);
        convertScaleAbs(gx, agx);
        convertScaleAbs(gy, agy);
        addWeighted(agx, 0.5, agy, 0.5, 0, grad);
        imshow("Sobel", grad);
        if (waitKey(33) == 27) break;
    }
}
*/

// ════════════════════════════════════════
//  TEMPLATE C: Canny on live webcam with trackbar
// ════════════════════════════════════════
/*
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
using namespace cv;
static int thresh = 50;
static Mat g_frame;
static void onTrack(int, void*) {
    if (g_frame.empty()) return;
    Mat gray, edges, dst;
    cvtColor(g_frame, gray, COLOR_BGR2GRAY);
    blur(gray, edges, Size(3,3));
    Canny(edges, edges, thresh, thresh * 3, 3);
    dst = Scalar::all(0);
    g_frame.copyTo(dst, edges);
    imshow("Canny", dst);
}
int main()
{
    VideoCapture cam(0);
    if (!cam.isOpened()) return -1;
    namedWindow("Canny");
    createTrackbar("Threshold:", "Canny", &thresh, 100, onTrack);
    while (1) {
        cam.read(g_frame);
        onTrack(0, 0);
        if (waitKey(33) == 27) break;
    }
}
*/

// ════════════════════════════════════════
//  TEMPLATE D: Background elimination from video file
// ════════════════════════════════════════
/*
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
using namespace cv;
int main()
{
    VideoCapture vcap("input.mpeg");
    if (!vcap.isOpened()) return -1;
    namedWindow("Original"); namedWindow("Diff");
    Mat frame;
    vcap.read(frame);
    vector<Mat> prev; cv::split(frame, prev);
    while (1) {
        if (!vcap.read(frame)) { waitKey(); break; }
        vector<Mat> curr; cv::split(frame, curr);
        Mat bd, gd, rd, rt;
        absdiff(prev[0], curr[0], bd);
        absdiff(prev[1], curr[1], gd);
        absdiff(prev[2], curr[2], rd);
        cv::threshold(rd, rt, 50, 255, THRESH_BINARY);
        Mat zero = Mat::zeros(rd.size(), rd.type());
        vector<Mat> out = {zero, zero, rt};
        Mat diff; cv::merge(out, diff);
        imshow("Original", frame);
        imshow("Diff", diff);
        if (waitKey(33) == 'q') break;
        prev[0]=curr[0].clone(); prev[1]=curr[1].clone(); prev[2]=curr[2].clone();
    }
}
*/

// ════════════════════════════════════════
//  TEMPLATE E: Save processed frames + write video
// ════════════════════════════════════════
/*
    // Setup BEFORE loop:
    int fourcc = VideoWriter::fourcc('m','p','4','v');
    VideoWriter out("result.mp4", fourcc, 30.0, frame.size(), false);  // grayscale
    int fcount = 0;

    // Inside loop:
    out.write(gray_frame);
    char fname[64];
    snprintf(fname, sizeof(fname), "frames/frame_%04d.png", fcount++);
    cv::imwrite(fname, gray_frame);

    // After loop:
    out.release();
*/

// ════════════════════════════════════════
//  TEMPLATE F: Hough Circles on webcam
// ════════════════════════════════════════
/*
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
using namespace cv;
int main()
{
    VideoCapture cam(0);
    if (!cam.isOpened()) return -1;
    cam.set(CAP_PROP_FRAME_WIDTH, 640);
    cam.set(CAP_PROP_FRAME_HEIGHT, 480);
    namedWindow("Circles");
    while (1) {
        Mat frame, gray;
        cam.read(frame);
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        GaussianBlur(gray, gray, Size(9,9), 2, 2);
        vector<Vec3f> circles;
        HoughCircles(gray, circles, HOUGH_GRADIENT, 1, gray.rows/8, 100, 50, 20, 200);
        for (auto& c : circles) {
            circle(frame, Point(c[0],c[1]), 1,   Scalar(0,100,100), 3, LINE_AA);
            circle(frame, Point(c[0],c[1]), c[2], Scalar(255,0,255), 3, LINE_AA);
        }
        imshow("Circles", frame);
        if (waitKey(10) == 'q') break;
    }
}
*/

// ════════════════════════════════════════════════════════════
//  QUICK-REFERENCE CARD  (key API signatures at a glance)
// ════════════════════════════════════════════════════════════
/*
 VIDEO I/O:
   VideoCapture cap(0);                    // open device 0
   VideoCapture cap("file.mp4");           // open file
   cap.isOpened()                          // check success
   cap.set(CAP_PROP_FRAME_WIDTH, W);       // set property
   cap.read(frame);                        // grab next frame → bool
   cap >> frame;                           // same as read
   VideoWriter w("out.mp4", fourcc, fps, size, isColor);
   w.write(frame);  w.release();

 IMAGE I/O:
   Mat img = imread("f.jpg", IMREAD_COLOR);
   imwrite("out.png", img);

 DISPLAY:
   namedWindow("name" [, WINDOW_NORMAL]);
   resizeWindow("name", w, h);
   imshow("name", mat);
   char k = (char)waitKey(ms);   // 0=wait forever
   destroyWindow("name");  destroyAllWindows();

 COLOR CONVERSION:
   cvtColor(src, dst, COLOR_BGR2GRAY);
   cvtColor(src, dst, COLOR_GRAY2BGR);
   cvtColor(src, dst, COLOR_BGR2HSV);

 CHANNEL OPS:
   cv::split(bgr, vector<Mat>);   // BGR → 3 planes
   cv::merge(vector<Mat>, bgr);   // 3 planes → BGR

 BLUR:
   GaussianBlur(src, dst, Size(k,k), sigX, sigY, BORDER_DEFAULT);
   blur(src, dst, Size(3,3));
   medianBlur(src, dst, k);

 EDGE DETECTION:
   Sobel(gray, grad, CV_16S, dx, dy, ksize, scale, delta, BORDER_DEFAULT);
   convertScaleAbs(grad16s, grad8u);
   addWeighted(agx, 0.5, agy, 0.5, 0, combined);
   Canny(gray_blurred, edges, lowT, highT, apertureSize);

 THRESHOLD:
   cv::threshold(src, dst, thresh, maxVal, THRESH_BINARY);

 DIFF / STATS:
   absdiff(src1, src2, dst);
   cv::sum(mat);  // returns Scalar with per-channel sum

 HISTOGRAM:
   calcHist(&img, 1, 0, Mat(), hist, 1, &bins, &range, true, false);
   normalize(hist, hist, 0, rows, NORM_MINMAX, -1, Mat());
   hist.at<float>(i);   // access bin i

 HOUGH:
   HoughLines(edges, lines, 1, CV_PI/180, thresh);         // Vec2f
   HoughLinesP(edges, lines, 1, CV_PI/180, thresh, minL, gap); // Vec4i
   HoughCircles(gray, circles, HOUGH_GRADIENT, dp, minDist,
                p1, p2, minR, maxR);                        // Vec3f

 DRAWING:
   line(img, pt1, pt2, Scalar(B,G,R), thickness, LINE_AA);
   circle(img, center, radius, Scalar(B,G,R), thickness, LINE_AA);
   rectangle(img, tl, br, color, thickness);
   cv::putText(img, text, pt, fontFace, scale, color, thick, LINE_AA);

 MAT CREATION:
   Mat::zeros(rows, cols, type);
   Mat::ones(rows, cols, type);
   src.clone();  // deep copy

 PIXEL ACCESS:
   Vec3b* row = mat.ptr<Vec3b>(r);  row[c][0..2]  // fast
   mat.at<float>(i)                               // safe, by value

 TRACKBAR:
   createTrackbar("label", "window", &intVar, maxVal, callback);

 TIMING:
   clock_gettime(CLOCK_MONOTONIC, &ts);
   elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9;
*/

// End of MASTER_REFERENCE.cpp

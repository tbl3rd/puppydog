#ifndef INCLUDE_PUPPYDOG_UTILITIES_HPP
#define INCLUDE_PUPPYDOG_UTILITIES_HPP


#include <opencv2/highgui.hpp>


// Create a new unobscured named window for image.
// Reset windows layout with when reset is not 0.
//
// The 23 term works around how MacOSX decorates windows.
//
static void makeWindow(const char *window, const cv::Size &size, int reset = 0)
{
    static int across = 1;
    static int count, moveX, moveY, maxY = 0;
    if (reset) {
        across = reset;
        count = moveX = moveY = maxY = 0;
    }
    if (count % across == 0) {
        moveY += maxY + 23;
        maxY = moveX = 0;
    }
    ++count;
    cv::namedWindow(window, cv::WINDOW_AUTOSIZE);
    cv::moveWindow(window, moveX, moveY);
    moveX += size.width;
    maxY = std::max(maxY, size.height);
}


#endif // #ifndef INCLUDE_PUPPYDOG_UTILITIES_HPP

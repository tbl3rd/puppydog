#ifndef INCLUDE_PUPPYDOG_VIDEO_HPP
#define INCLUDE_PUPPYDOG_VIDEO_HPP


#include <opencv2/highgui/highgui.hpp>


// Just cv::VideoCapture extended for convenience.  The const_cast<>()s
// work around the missing member const on cv::VideoCapture::get().
//
struct VideoSource: cv::VideoCapture {

    double getFramesPerSecond() const {
        VideoSource *const p = const_cast<VideoSource *>(this);
        const double fps = p->get(cv::CAP_PROP_FPS);
        return fps ? fps : 30.0;        // for MacBook iSight camera
    }

    int getFourCcCodec() const {
        VideoSource *const p = const_cast<VideoSource *>(this);
        return p->get(cv::CAP_PROP_FOURCC);
    }

    std::string getFourCcCodecString() const {
        char result[] = "????";
        VideoSource *const p = const_cast<VideoSource *>(this);
        const int code = p->getFourCcCodec();
        result[0] = ((code >>  0) & 0xff);
        result[1] = ((code >>  8) & 0xff);
        result[2] = ((code >> 16) & 0xff);
        result[3] = ((code >> 24) & 0xff);
        result[4] = ""[0];
        return std::string(result);
    }

    int getFrameCount() const {
        VideoSource *const p = const_cast<VideoSource *>(this);
        return p->get(cv::CAP_PROP_FRAME_COUNT);
    }

    cv::Size getFrameSize() const {
        VideoSource *const p = const_cast<VideoSource *>(this);
        const int w = p->get(cv::CAP_PROP_FRAME_WIDTH);
        const int h = p->get(cv::CAP_PROP_FRAME_HEIGHT);
        const cv::Size result(w, h);
        return result;
    }

    int getPosition(void) const {
        VideoSource *const p = const_cast<VideoSource *>(this);
        return p->get(cv::CAP_PROP_POS_FRAMES);
    }
    void setPosition(int p) { this->set(cv::CAP_PROP_POS_FRAMES, p); }

    VideoSource(const std::string &fileName): VideoCapture(fileName) {}
    VideoSource(int n): VideoCapture(n) {}
    VideoSource(): VideoCapture() {}
};


// Return a video capture object suitable for the source string.
// Return the camera with specified ID if source contains an integer.
// Otherwise attempt to open a video file.
// Otherwise return the default camera (-1).
//
static VideoSource openVideo(const char *source)
{
    int cameraId = 0;
    std::istringstream iss(source); iss >> cameraId;
    if (iss) return VideoSource(cameraId);
    std::string filename;
    std::istringstream sss(source); sss >> filename;
    if (sss) return VideoSource(filename);
    return VideoSource(-1);
}


#endif // #define INCLUDE_PUPPYDOG_VIDEO_HPP

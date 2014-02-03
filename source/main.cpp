#include <iostream>

#include <background.hpp>
#include <classifier.hpp>
#include <utilities.hpp>
#include <video.hpp>


static void showUsage(const char *av0)
{
    std::cerr << av0 << ": Remove background from a video."
              << std::endl << std::endl
              << "Usage: " << av0 << " <camera> <output>" << std::endl
              << std::endl
              << "Where: <camera> is a camera number or video file name."
              << std::endl
              << "       <output> is the output file name."
              << std::endl << std::endl
              << "Example: " << av0 << " 0" << std::endl << std::endl;
}


// Return regions of interest detected by classifier in gray.
//
static void detectCascade(cv::CascadeClassifier &classifier,
                          const cv::Mat &gray,
                          std::vector<cv::Rect> &regions)
{
    static double scaleFactor = 1.1;
    static const int minNeighbors = 2;
    static const cv::Size minSize(30, 30);
    static const cv::Size maxSize;
    classifier.detectMultiScale(gray, regions, scaleFactor, minNeighbors,
                                cv::CASCADE_SCALE_IMAGE, minSize, maxSize);
}



int main(int ac, const char *av[])
{
    if (ac == 4) {
        std::cout << av[0] << ": Camera is " << av[1] << std::endl;
        std::cout << av[0] << ": Output is " << av[2] << std::endl;
        std::cout << av[0] << ": Training is " << av[3] << std::endl;
        VideoSource camera = openVideo(av[1]);
        Classifier classifier(av[3]);
        if (camera.isOpened()) {
            const int codec = camera.getFourCcCodec();
            const double fps = camera.getFramesPerSecond();
            const cv::Size size = camera.getFrameSize();
            const int count = camera.getFrameCount();
            cv::VideoWriter output(av[2], codec, fps, size);
            if (output.isOpened()) {
                std::cout << av[0] << ": " << camera.getFourCcCodecString()
                          << " " << count
                          << " (" << size.width << "x" << size.height << ")"
                          << " frames at " << fps << " FPS" << std::endl;
                std::cout << av[0] << ": Writing to " << av[2] << std::endl;
                static BackgroundRemoverMog br;
                for (int i = 0; i < count; ++i) {
                    static cv::Mat frame; camera >> frame;
                    if (!frame.empty()) output << br(frame);
                }
                return 0;
            }
        }
    }
    showUsage(av[0]);
    return 1;
}

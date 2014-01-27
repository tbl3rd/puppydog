#include <iostream>

#include <background.hpp>
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


int main(int ac, const char *av[])
{
    if (ac == 3) {
        std::cout << av[0] << ": Camera is " << av[1] << std::endl;
        std::cout << av[0] << ": Output is " << av[2] << std::endl;
        CvVideoCapture camera = openVideo(av[1]);
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

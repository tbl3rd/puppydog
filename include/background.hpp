#ifndef INCLUDE_PUPPYDOG_BACKGROUND_HPP
#define INCLUDE_PUPPYDOG_BACKGROUND_HPP


#include <opencv2/video/background_segm.hpp>


// Remove video background with BackgroundSubtractor classes.
// PtrBs is cv::Ptr<cv::BackgroundSubtractorMOG> or something.
//
template <typename PtrBs> class BackgroundRemover {
    PtrBs itsBs;
    cv::Mat itsMask;
    cv::Mat itsOutput;

    static PtrBs makeBr() { return PtrBs(0); }

public:

    // Apply frame to background tracker.
    //
    const cv::Mat &operator()(const cv::Mat &frame)
    {
        static cv::Mat black = cv::Mat::zeros(frame.size(), frame.type());
        itsBs->apply(frame, itsMask);
        black.copyTo(itsOutput);
        frame.copyTo(itsOutput, itsMask);
        return itsOutput;
    }

    BackgroundRemover(): itsBs(makeBr()) {}
};


// Hide template syntax behind typedefs.
//
typedef BackgroundRemover<cv::Ptr<cv::BackgroundSubtractorMOG> >
BackgroundRemoverMog;
typedef BackgroundRemover<cv::Ptr<cv::BackgroundSubtractorMOG2> >
BackgroundRemoverMog2;
typedef BackgroundRemover<cv::Ptr<cv::BackgroundSubtractorGMG> >
BackgroundRemoverGmg;


// Hide differing create.*() syntax behind a function template.
//
template<> cv::Ptr<cv::BackgroundSubtractorMOG>
BackgroundRemoverMog::makeBr()
{
    return cv::Ptr<cv::BackgroundSubtractorMOG>
        (cv::createBackgroundSubtractorMOG());
}
template<> cv::Ptr<cv::BackgroundSubtractorMOG2>
BackgroundRemoverMog2::makeBr()
{
    return cv::Ptr<cv::BackgroundSubtractorMOG2>
        (cv::createBackgroundSubtractorMOG2());
}
template<> cv::Ptr<cv::BackgroundSubtractorGMG>
BackgroundRemoverGmg::makeBr()
{
    return cv::Ptr<cv::BackgroundSubtractorGMG>
        (cv::createBackgroundSubtractorGMG());
}


#endif // #define INCLUDE_PUPPYDOG_BACKGROUND_HPP

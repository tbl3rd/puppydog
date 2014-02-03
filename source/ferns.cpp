#include <opencv2/opencv.hpp>


// Parameters to tune a ferns classifier.
//
#define FCPIN(FILENODE, P, M)     (FILENODE[#M] >> P.M)
#define FCPOUT(FILESTORAGE, P, M) (FILESTORAGE << #M << P.M)
struct FernsClassifierParameters
{
    int fernCount;
    int featureCount;
    float fernValid;
    float confidenceLevel;
    float fernThrehsold;
    float nearestNeighborThreshold;
    float nearestNeighborValid;

    // Read parameters p out of fs.
    //
    friend const cv::FileStorage &operator>>(const cv::FileStorage &fs,
                                             FernsClassifierParameters &p)
    {
        const cv::FileNode fn = fs["FernsClassifierParameters"];
        FCPIN(fn, p, fernCount);
        FCPIN(fn, p, featureCount);
        FCPIN(fn, p, fernValid);
        FCPIN(fn, p, confidenceLevel);
        FCPIN(fn, p, fernThrehsold);
        FCPIN(fn, p, nearestNeighborThreshold);
        FCPIN(fn, p, nearestNeighborValid);
        return fs;
    }

    // Write parameters p into fs.
    //
    friend cv::FileStorage &operator<<(cv::FileStorage &fs,
                                       const FernsClassifierParameters &p)
    {
        fs << "FernsClassifierParameters" << "{";
        FCPOUT(fs, p, fernCount);
        FCPOUT(fs, p, featureCount);
        FCPOUT(fs, p, fernValid);
        FCPOUT(fs, p, confidenceLevel);
        FCPOUT(fs, p, fernThrehsold);
        FCPOUT(fs, p, nearestNeighborThreshold);
        FCPOUT(fs, p, nearestNeighborValid);
        fs << "}";
        return fs;
    }

    FernsClassifierParameters(const char *fileName)
        : fernCount(0),
          featureCount(0),
          fernValid(0.0),
          confidenceLevel(0.0),
          fernThrehsold(0.0),
          nearestNeighborThreshold(0.0),
          nearestNeighborValid(0.0)
    {
        const cv::FileStorage fs(fileName, cv::FileStorage::READ);
        if (fs.isOpened()) fs >> *this;
    }
};
#undef  FCPOUT
#undef  FCPIN


// A feature in a fern.  A predicate on two points in a patch.
// True iff the patch at p is greater than the patch at q.
//
class FernsClassifierFeature
{
    const cv::Point_<uchar> p;
    const cv::Point_<uchar> q;
public:
    FernsClassifierFeature()
        : p(0, 0), q(0, 0)
    {}
    FernsClassifierFeature(const cv::Point &pp, const cv::Point &qq)
        : p(pp), q(qq)
    {}
    bool operator()(cv::Mat const &patch) const {
        return patch.at<uchar>(p) > patch.at<uchar>(q);
    }
};


// A 1-bit randomized Ferns classifier.
//
class FernsClassifier {

    typedef FernsClassifierParameters Parameters;
    typedef FernsClassifierFeature Feature;
    typedef std::vector<Feature> Fern;

    Parameters itsParameters;

    // This classifier has itsParameters.fernCount ferns.
    // Each fern has itsParameters.featureCount features.
    //
    std::vector<Fern> itsFerns;

    // Threshold for a negative match, initially:
    // 0.5 * itsParameters.fernCount
    //
    float itsNegativeThreshold;

    // Threshold for a positive match, initially:
    // itsParameters.fernThreshold * itsParameters.fernCount
    //
    float itsPositiveThreshold;

    std::vector< std::vector<int> > itsNegativeCounts;
    std::vector< std::vector<int> > itsPositiveCounts;
    std::vector< std::vector<float> > itsPosteriors;

    std::vector<cv::Mat> itsNegativePatches;
    std::vector<cv::Mat> itsPositivePatches;

public:

    FernsClassifier(const char *xml)
        : itsParameters(xml)
    {}
};

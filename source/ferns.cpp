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
    float correlationThreshold;
    float fernThreshold;
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
        FCPIN(fn, p, correlationThreshold);
        FCPIN(fn, p, fernThreshold);
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
        FCPOUT(fs, p, correlationThreshold);
        FCPOUT(fs, p, fernThreshold);
        FCPOUT(fs, p, nearestNeighborThreshold);
        FCPOUT(fs, p, nearestNeighborValid);
        fs << "}";
        return fs;
    }

    FernsClassifierParameters(const char *fileName)
        : fernCount(0)
        , featureCount(0)
        , fernValid(0.0)
        , correlationThreshold(0.0)
        , fernThreshold(0.0)
        , nearestNeighborThreshold(0.0)
        , nearestNeighborValid(0.0)
    {
        cv::FileStorage fs(fileName, cv::FileStorage::READ);
        if (fs.isOpened()) fs >> *this;
        fs.release();
    }
};
#undef  FCPOUT
#undef  FCPIN


// A feature in a fern.  A predicate on two points in a patch.
// True iff the patch at p is greater than the patch at q.
//
class FernsClassifierFeature
{
    cv::Point_<uchar> p;
    cv::Point_<uchar> q;
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


// A randomized Ferns classifier of 1-bit features.
//
class FernsClassifier {

    typedef FernsClassifierParameters Parameters;
    typedef FernsClassifierFeature Feature;
    typedef std::vector<Feature> Fern;

    const Parameters itsParameters;

    // Threshold for a negative match.
    //
    const float itsNegativeThreshold;

    // Threshold for a positive match.
    //
    const float itsPositiveThreshold;

    // This classifier has itsParameters.fernCount ferns: one for each
    // scale.  Each fern has itsParameters.featureCount features.
    //
    std::vector<Fern> itsFerns;

    std::vector< std::vector<float> > itsPriors;
    std::vector< std::vector<int> > itsNegativeCounts;
    std::vector< std::vector<int> > itsPositiveCounts;

    std::vector<cv::Mat> itsNegativePatches;
    std::vector<cv::Mat> itsPositivePatches;

    // Update this classifier with fern according to match.
    //
    void update(const std::vector<int> &fern, bool match);

public:

    // Compute the features for patch at the scale indexed by n and write
    // it into fern.
    //
    void computeFern(const cv::Mat &patch, int n, std::vector<int> &fern);

    // Return the measure of fern against the priors of this classifier.
    //
    float measure(const std::vector<int> &fern);

    void trainOnFerns(const std::vector< std::pair<std::vector<int>, int> > &ferns);

    // Read tuning parameters from xml file, then construct 1-bit features
    // for pairs of random points in a fern for each size in scales.
    // Initialize priors and counts for each fern to 0.
    //
    FernsClassifier(const char *xml, const std::vector<cv::Size> &scales);
};


FernsClassifier::FernsClassifier(const char *xml,
                                 const std::vector<cv::Size> &scales)
    : itsParameters(xml)
    , itsNegativeThreshold(0.5 * itsParameters.fernCount)
    , itsPositiveThreshold(itsParameters.fernThreshold
                           * itsParameters.fernCount)
    , itsFerns(0)
    , itsPriors(0)
    , itsNegativeCounts(0)
    , itsPositiveCounts(0)
    , itsNegativePatches(0)
    , itsPositivePatches(0)
{
    static cv::RNG rng;
    const int fernCount = itsParameters.fernCount;
    const int featureCount = itsParameters.featureCount;
    const int totalFeatures = fernCount * featureCount;
    const std::size_t scalesCount = scales.size();
    itsFerns = std::vector<Fern>(scalesCount, Fern(totalFeatures));
    for (int i = 0; i < totalFeatures; ++i) {
        const cv::Point_<float> p1((float)rng, (float)rng);
        const cv::Point_<float> p2((float)rng, (float)rng);
        for (std::size_t j = 0; j < scalesCount; ++j) {
            Fern &f = itsFerns[j];
            const cv::Size &s = scales[j];
            const cv::Point_<uchar> p(p1.x * s.width, p1.y * s.height);
            const cv::Point_<uchar> q(p2.x * s.width, p2.y * s.height);
            f[i] = Feature(p, q);
        }
    }
    const std::size_t count = 1 << featureCount;
    const std::vector<float> floatZeros(count, 0.0);
    const std::vector<int> intZeros(count, 0);
    for (int i = 0; i < fernCount; ++i) {
        itsPriors.push_back(floatZeros);
        itsNegativeCounts.push_back(intZeros);
        itsPositiveCounts.push_back(intZeros);
    }
}


// Update this classifier with fern according to match.
//
void FernsClassifier::update(const std::vector<int> &fern, bool match)
{
    const int fernCount = itsParameters.fernCount;
    for (int i = 0; i < fernCount; ++i) {
        const int index = fern[i];
        int &positive = itsPositiveCounts[i][index];
        int &negative = itsNegativeCounts[i][index];
        float &prior  =         itsPriors[i][index];
        int &count = match ? positive : negative;
        ++count;
        if (positive == 0) {
            prior = 0.0;
        } else {
            const float total = positive + negative;
            prior = (float)positive / total;
        }
    }
}


// Compute the features for patch at the scale indexed by n and write it
// into fern.
//
void FernsClassifier::computeFern(const cv::Mat &patch, int n,
                                  std::vector<int> &fern)
{
    const int fernCount = itsParameters.fernCount;
    const int featureCount = itsParameters.featureCount;
    const Fern &fernAtScale = itsFerns[n];
    for (int i = 0; i < fernCount; ++i) {
        const int featureOffset = i * fernCount;
        int classification = 0x0;
        for (int j = 0; j < featureCount; ++j) {
            const Feature &feature = fernAtScale[j + featureOffset];
            classification <<= 1;
            classification += feature(patch);
        }
        fern[i] = classification;
    }
}


// Return the probability that fern matches the priors of this
// classifier.
//
float FernsClassifier::measure(const std::vector<int> &fern)
{
    const int fernCount = itsParameters.fernCount;
    float result = 0.0;
    for (int i = 0; i < fernCount; ++i) {
        const std::vector<float> &prior = itsPriors[i];
        const int decision = fern[i];
        result += prior[decision];
    }
    return result;
}

// FIXME: Shouldn't measure be >= itsPositiveThreshold?
//
void FernsClassifier::trainOnFerns
(const std::vector< std::pair<std::vector<int>, int> > &ferns)
{
    for (int i = 0; i < ferns.size(); ++i) {
        const std::pair<std::vector<int>, int> &fernPair = ferns[i];
        const std::vector<int> &fern = fernPair.first;
        const bool match = fernPair.second == 1;
        if (match) {
            if (measure(fern) <= itsPositiveThreshold) update(fern, true);
        } else {
            if (measure(fern) >= itsNegativeThreshold) update(fern, false);
        }
    }
}

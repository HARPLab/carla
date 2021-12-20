// From https://www-ti.informatik.uni-tuebingen.de/santini/I-BDT
#ifndef IBDT_H
#define IBDT_H

#include <deque>
#include <vector>
#include <algorithm>

#include "GazeData.h"

#include <opencv2/ml.hpp>

class IBDT_Prob {
public:
    IBDT_Prob() : prior(0), likelihood(0), posterior(0) {}
    double prior;
    double likelihood;
    double posterior;
    void update() { posterior = prior*likelihood; }
};

class IBDT_Data : public GazeDataEntry {
public:
    IBDT_Data(GazeDataEntry base) :
        GazeDataEntry(base),
        pursuit(),
        fixation(),
        saccade() { }

    IBDT_Prob pursuit;
    IBDT_Prob fixation;
    IBDT_Prob saccade;
};

class IBDT
{
public:
    enum CLASSIFICATION {
        TERNARY = 0,
        BINARY = 1
    };

    IBDT(const double &maxSaccadeDurationMs=80, const double &minSampleConfidence=0.5, const enum CLASSIFICATION &classification=TERNARY);
    void addPoint(GazeDataEntry &entry);
    bool train(std::vector<GazeDataEntry> &gaze);
    double estimateVelocity(const GazeDataEntry &cur, const GazeDataEntry &prev);
	double get_fMean(void) { return fMean; }
	double get_sMean(void) { return sMean; }

private:
    double maxSaccadeDurationMs;
    double minSampleConfidence;
    CLASSIFICATION classification;
    std::deque<IBDT_Data> window;

    IBDT_Data *cur;
    IBDT_Data *prev;
    bool firstPoint;
    cv::Ptr<cv::ml::EM> model;
    unsigned int fIdx, sIdx;
    double fMean, sMean;

    void updatePursuitPrior();
    void updatePursuitLikelihood();
    void updatePursuitLikelihoodNew();
    void updateFixationAndSaccadeLikelihood();

    void binaryClassification();
    void ternaryClassification();
};

#endif // IBDT_H

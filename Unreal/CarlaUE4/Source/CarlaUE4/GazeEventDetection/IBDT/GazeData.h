#ifndef GAZEDATA_H
#define GAZEDATA_H

#include <vector>

#include "opencv2/opencv.hpp"

enum Movement {
    FIXATION = 0,
    SACCADE = 1,
    PURSUIT = 2,
    NOISE = 3,
    UNDEF = 4
};

class GazeDataEntry
{
public:
    GazeDataEntry(const double &ts, const double &confidence, const double &x, const double &y) :
        ts(ts),
        confidence(confidence),
        x(x),
        y(y),
        v(0),
        classification(UNDEF)
    {}

    bool isFixation() { return classification == FIXATION; }
    bool isSaccade() { return classification == SACCADE; }
    bool isPursuit() { return classification == PURSUIT; }
    bool isNoise() { return classification == NOISE; }
    bool isUndef() { return classification == UNDEF; }

    unsigned int pause();

    double x, y, v;
    double confidence;
    double ts;
    Movement classification;

	friend std::ostream& operator<<(std::ostream & os, const GazeDataEntry & gde)
	{
		 os << "Timestamps, confidence, X, Y, V, classification" << std::endl
			<< gde.ts << ", "
			<< gde.confidence << ", "
			<< gde.x << ", "
			<< gde.y << ", "
			<< gde.v << ", "
			<< gde.classification << ";" <<std::endl;
		return os;
	}
};

#endif // GAZEDATA_H

#ifndef GAZEDATA_H
#define GAZEDATA_H

#include <vector>

#include "opencv2/opencv.hpp"

enum GazeMovement {
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
        classification(GazeMovement::UNDEF)
    {}

    bool isFixation() { return classification == GazeMovement::FIXATION; }
    bool isSaccade() { return classification == GazeMovement::SACCADE; }
    bool isPursuit() { return classification == GazeMovement::PURSUIT; }
    bool isNoise() { return classification == GazeMovement::NOISE; }
    bool isUndef() { return classification == GazeMovement::UNDEF; }

    unsigned int pause();

    double ts;
    double confidence;
    double x, y, v;

    GazeMovement classification;

    std::string to_str(void) const
    {
        std::stringstream ss;
        ss << "Timestamps, confidence, X, Y, V, classification" << std::endl
           << ts << ", "
           << confidence << ", "
           << x << ", "
           << y << ", "
           << v << ", "
           << classification << ";" <<std::endl;
        return ss.str();
    }

	friend std::ostream& operator<<(std::ostream & os, const GazeDataEntry & gde)
	{ // TODO : dont repeat yourself replace w something like  os << this->to_str()
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

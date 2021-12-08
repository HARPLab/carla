// From https://www-ti.informatik.uni-tuebingen.de/santini/I-BDT
#include "IBDT.h"

#include <climits>
#include <numeric>
#include <assert.h>
#include "opencv2/core.hpp"

using namespace std;
using namespace cv;

IBDT::IBDT(const double &maxSaccadeDurationMs, const double &minSampleConfidence, const CLASSIFICATION &classification) :
	maxSaccadeDurationMs(maxSaccadeDurationMs),
	minSampleConfidence(minSampleConfidence),
	classification(classification),
	cur(NULL)
{

}

double IBDT::estimateVelocity(const GazeDataEntry &cur_gaze, const GazeDataEntry &prev_gaze)
{
	double dist = norm(Point2f(cur_gaze.x, cur_gaze.y) - Point2f(prev_gaze.x, prev_gaze.y));
	double dt = cur_gaze.ts - prev_gaze.ts;
	return dist / dt;
}

void IBDT::train(std::vector<GazeDataEntry> &gaze)
{
	Mat samples;

	// Find first valid sample
	auto previous = gaze.begin();
	while (previous != gaze.end() && previous->confidence < minSampleConfidence) {
		previous->v = std::numeric_limits<double>::quiet_NaN();
		previous++;
	}
	// what if no valid sample?
    // assert(std::distance(gaze.begin(), g)) <= 1)


	// Estimate velocities for remaining training samples
	for (auto g = previous + 1; g != gaze.end(); g++) {
		if (g->confidence < minSampleConfidence) {

//            UE_LOG(LogTemp, Log, TEXT("iterator distance %d"),
//                   std::distance(gaze.begin(), g));
//            UE_LOG(LogTemp, Log, TEXT("timestamp %f, %f"),
//                   gaze.begin()->ts, g->ts);

            g->v = std::numeric_limits<double>::quiet_NaN();
			continue;
		}

		g->v = estimateVelocity(*g, *previous);
		if (!isnan(g->v))
			samples.push_back(g->v);

		previous = g;
	}

	model = ml::EM::create();
	model->setClustersNumber(2);
	model->setCovarianceMatrixType(ml::EM::COV_MAT_GENERIC);
	model->setTermCriteria(TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 15000, 1e-6));
	model->trainEM(samples);

	fIdx = 0;
	sIdx = 1;
	Mat means = model->getMeans();
	if (means.at<double>(0) > means.at<double>(1)) {
		fIdx = 1;
		sIdx = 0;
	}

	fMean = means.at<double>(fIdx);
	sMean = means.at<double>(sIdx);
}

void IBDT::updateFixationAndSaccadeLikelihood()
{
	if (cur->v < fMean) {
		cur->fixation.likelihood = 1;
		cur->saccade.likelihood = 0;
		return;
	}

	if (cur->v > sMean) {
		cur->fixation.likelihood = 0;
		cur->saccade.likelihood = 1;
		return;
	}

	Mat sample = (Mat_<double>(1, 1) << cur->v);
	Mat likelihoods;
	model->predict(sample, likelihoods);
	cur->fixation.likelihood = likelihoods.at<double>(fIdx);
	cur->saccade.likelihood = likelihoods.at<double>(sIdx);
}

void IBDT::binaryClassification()
{
	if (cur->fixation.likelihood > cur->saccade.likelihood)
		cur->classification = GazeMovement::FIXATION;
	else
		cur->classification = GazeMovement::SACCADE;
}

void IBDT::ternaryClassification()
{
	// Class that maximizes posterior probability
	double maxPosterior = cur->fixation.posterior;
	cur->classification = GazeMovement::FIXATION;

	if (cur->saccade.posterior > maxPosterior) {
		cur->classification = GazeMovement::SACCADE;
		maxPosterior = cur->saccade.posterior;
	}

	if (cur->pursuit.posterior > maxPosterior)
		cur->classification = GazeMovement::PURSUIT;

	// Catch up saccades as saccades
	if (cur->v > sMean)
		cur->classification = GazeMovement::SACCADE;
}

void IBDT::addPoint(GazeDataEntry &entry)
{
	entry.classification = GazeMovement::UNDEF;

	// Low confidence, ignore it
	if (entry.confidence < minSampleConfidence)
		return;

	// Add new point to window and update previous valid point
	window.push_back(entry);
	prev = cur;
	cur = &window.back();

	// Remove old entries from window
	while (true) {
		if (cur->ts - window.front().ts > 2 * maxSaccadeDurationMs)
			window.pop_front();
		else
			break;
	}

	// First point being classified is a special case (since we classify interframe periods)
	if (!prev) {
		cur->classification = GazeMovement::UNDEF;
		entry.classification = cur->classification;
		return;
	}

	// We have an intersample period, let's classify it
	cur->v = estimateVelocity(*cur, *prev);

	// Update the priors
	updatePursuitPrior();
	cur->saccade.prior = cur->fixation.prior = 1 - cur->pursuit.prior;

	// Update the likelihoods
	updatePursuitLikelihood();
	updateFixationAndSaccadeLikelihood();

	// Update the posteriors
	cur->pursuit.update();
	cur->fixation.update();
	cur->saccade.update();

	// Decision
	switch (classification) {
	case TERNARY:
		ternaryClassification();
		break;
	case BINARY:
		binaryClassification();
		break;
	}

	entry.classification = cur->classification;
}

void IBDT::updatePursuitLikelihood()
{
	if (window.size() < 2)
		return;

	double movement = 0;
	for (auto d = window.begin() + 1; d != window.end(); d++) {
		// if (d-v > 0) // original
		if (d->v > fMean && d->v < sMean) // adaptive: don't activate with too small or too large movements
			movement++;
	}

	double n = window.size() - 1;
	double movementRatio = movement / n;
	cur->pursuit.likelihood = movementRatio;
}

void IBDT::updatePursuitPrior()
{
	vector<double> previousLikelihoods;
	for (auto d = window.begin(); d != window.end() - 1; d++)
		previousLikelihoods.push_back(d->pursuit.likelihood);
	cur->pursuit.prior =
		std::accumulate(previousLikelihoods.begin(), previousLikelihoods.end(), 0.0)
		/ previousLikelihoods.size();
}


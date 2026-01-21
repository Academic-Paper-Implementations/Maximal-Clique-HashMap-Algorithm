/**
 * @file utils.cpp
 * @brief Implementation: Feature counting, dispersion calculation, and rare intensity
 */

#include "utils.h"
#include <unordered_map>

// Count instances per feature type and sort by frequency (ascending)
std::map<FeatureType, int> countAndSortFeatures(
	const std::vector<SpatialInstance>& instances) {
	//////////////////////////////////////
	//////// TODO: Implement (1)//////////
	//////////////////////////////////////



};

// Calculate dispersion (delta) from feature distribution
double calculateDirpersion(const std::map<FeatureType, int>& featureCount) {
	//////////////////////////////////////
	//////// TODO: Implement (2)//////////
	//////////////////////////////////////



};

// Calculate rare intensity for each instance in a colocation
std::unordered_map<InstanceID, double> calcRareIntensity(
	Colocation c,
	const std::map<FeatureType, int>& featureCounts,
	double delta) {
	//////////////////////////////////////
	//////// TODO: Implement (11)//////////
	//////////////////////////////////////



};
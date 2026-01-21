/**
 * @file utils.cpp
 * @brief Implementation: Feature counting, dispersion calculation, and rare intensity
 */

#include "utils.h"
#include <unordered_map>
#include <set>
#include <chrono>
#include <windows.h>
#include <psapi.h>
#include <iostream> 
#include <iomanip>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <vector>
#include <numeric>

// Count instances per feature type and sort by frequency (ascending)
std::map<FeatureType, int> countAndSortFeatures(
	const std::vector<SpatialInstance>& instances) {
	//////// TODO: Implement (1)//////////
	std::map<FeatureType, int> counts;
	for (const auto& instance : instances) {
		counts[instance.type]++;
	}
	return counts;
};

// Calculate dispersion (delta) from feature distribution
double calculateDirpersion(const std::map<FeatureType, int>& featureCount) {
		//////// TODO: Implement (2)//////////
    if (featureCount.empty()) return 0.0;
    if (featureCount.size() == 1) return 0.0; // Cannot calculate std dev with 1 item

    std::vector<double> logCounts;
    logCounts.reserve(featureCount.size());

    // 1. Calculate ln(N(fi))
    for (const auto& pair : featureCount) {
        logCounts.push_back(std::log(static_cast<double>(pair.second)));
    }

    size_t m = logCounts.size();
    if (m < 2) return 0.0;

    // 2. Calculate mean of log counts
    double sumLog = std::accumulate(logCounts.begin(), logCounts.end(), 0.0);
    double meanLog = sumLog / m;

    // 3. Calculate sum of squared differences
    double sumSqDiff = 0.0;
    for (double val : logCounts) {
        sumSqDiff += (val - meanLog) * (val - meanLog);
    }

    // 4. Calculate standard deviation (sample base: m-1)
    double variance = sumSqDiff / (m - 1);
    
    return std::sqrt(variance);
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
/**
 * @file miner.h
 * @brief Prevalent colocation pattern mining
 */

#pragma once
#include "types.h"
#include <set>
#include <map>
#include <unordered_map>
#include <queue>
#include <unordered_map>

/**
 * @brief Class for mining prevalent colocation patterns
 */
class Miner {
private:
	// Query instances of a colocation from hashmap
	std::map<FeatureType, std::set<const SpatialInstance*>> queryInstances(
		Colocation c,
		const std::map<Colocation, std::map<FeatureType, std::set<const SpatialInstance*>>>& hashMap);

	// Compute weighted participation index for a colocation
	double computeWeightedPI(
		const std::map<FeatureType, std::set<const SpatialInstance*>>& partInstances,
		Colocation c,
		const std::unordered_map<FeatureType, double>& rareIntensityMap,
		const std::map<FeatureType, int>& featureCounts);

	// Generate all size-1 subsets of a colocation
	std::set<Colocation> generateSubsets(const Colocation& c);

	// Deduce prevalent subsets using downward closure property
	std::set<Colocation> deducePrevalentSubsets(std::set<Colocation>& subsets, const Colocation& c);

public:
	// Mine prevalent colocation patterns (main algorithm)
	std::set<Colocation> minePCPs(
		std::priority_queue<Colocation>& candidateColocations,
		const std::map<Colocation, std::map<FeatureType, std::set<const SpatialInstance*>>>& hashMap,
		const std::map<FeatureType, int>& featureCounts,
		double delta,
		double min_prev
	);
};
/**
 * @file miner.cpp
 * @brief Implementation: Mining prevalent colocation patterns using weighted PI
 */

#include "miner.h"
#include "utils.h"
#include <vector>
#include <queue>
#include <map>
#include <unordered_map>
#include <algorithm>


// Main mining algorithm: find all prevalent colocation patterns
std::set<Colocation> Miner::minePCPs(
	std::priority_queue<Colocation, std::vector<Colocation>, ColocationPriorityComp>& candidateColocations,
	const std::map<Colocation, std::map<FeatureType, std::set<const SpatialInstance*>>>& hashMap,
	const std::map<FeatureType, int>& featureCounts,
	double delta,
	double min_prev) {

	std::set<Colocation> prevalentPCs;
	std::set<Colocation> nonPrevalentPCs;
	std::set<Colocation> newCs = {};

	while (!candidateColocations.empty()) {
		Colocation c = candidateColocations.top();
		candidateColocations.pop();

		if (prevalentPCs.find(c) == prevalentPCs.end() && 
			nonPrevalentPCs.find(c) == nonPrevalentPCs.end()) {

			auto partInstances = queryInstances(c, hashMap);
			auto rareIntensityMap = calcRareIntensity(c, featureCounts, delta);
			
			// 5. Calculate Weighted PI
			// Note: Changed signature of computeWeightedPI as per user authorization to include featureCounts
			double weightedPI = computeWeightedPI(partInstances, c, rareIntensityMap, featureCounts);
			newCs = generateSubsets(c);

			if (weightedPI >= min_prev) {
				prevalentPCs.insert(c);
				auto prevalentSubsets = deducePrevalentSubsets(newCs, c);
				for (const auto& subset : prevalentSubsets) {
					prevalentPCs.insert(subset);
				}

				std::set<Colocation> filteredSubsets;
				for (const auto& subset : newCs) {
					if (prevalentSubsets.find(subset) == prevalentSubsets.end()) {
						filteredSubsets.insert(subset);
					}
				}
			}
			else {
				nonPrevalentPCs.insert(c);
			}
		}
		for (const auto& subset : newCs) {
			candidateColocations.push(subset);
		}
	}
	return prevalentPCs;
};


// Query instances of a colocation from hashmap
std::map<FeatureType, std::set<const SpatialInstance*>> Miner::queryInstances(
	Colocation c,
	const std::map<Colocation, std::map<FeatureType, std::set<const SpatialInstance*>>>& hashMap) {
		//////// TODO: Implement (10)/////////

	std::map<FeatureType, std::set<const SpatialInstance*>> instancesMap;

	for (const auto& entry : hashMap) {
		const Colocation& maximalClique = entry.first;
		const auto& cliqueInstances = entry.second;

		// Check if c is a subset of maximalClique
		bool isSubset = true;
		for (const auto& f : c) {
			bool found = false;
			for (const auto& mf : maximalClique) {
				if (mf == f) {
					found = true;
					break;
				}
			}
			if (!found) {
				isSubset = false;
				break;
			}
		}

		// If c is a subset, merge instances
		if (isSubset) {
			for (const auto& f : c) {
				if (cliqueInstances.count(f)) {
					const auto& insts = cliqueInstances.at(f);
					instancesMap[f].insert(insts.begin(), insts.end());
				}
			}
		}
	}

	return instancesMap;
};

// Compute weighted participation index for a colocation
double Miner::computeWeightedPI(
	const std::map<FeatureType, std::set<const SpatialInstance*>>& partInstances,
	Colocation c,
	const std::unordered_map<FeatureType, double>& rareIntensityMap,
	const std::map<FeatureType, int>& featureCounts) {
		//////// TODO: Implement (12)/////////
	if (c.empty()) return 0.0;
	
	double minWPR = -1.0;

	for (const auto& f : c) {
		// Calculate PR = count / N
		int count = 0;
		if (partInstances.count(f)) {
			count = partInstances.at(f).size();
		}

		int totalCount = 0;
		if (featureCounts.count(f)) {
			totalCount = featureCounts.at(f);
		}

		if (totalCount == 0) continue;

		double pr = static_cast<double>(count) / totalCount;

		// Retrieve RI
		double ri = 1.0; // Default if missing? Or should skip? 
		// If RI is missing, it implies error or skip. 
		// According to logic, RI depends on featureCounts. If >0 count, RI exists.
		if (rareIntensityMap.count(f)) {
			ri = rareIntensityMap.at(f);
		}

		// Calculate W_log = 1 / RI
		double w_log = 0.0;
		if (ri > 1e-9) {
			w_log = 1.0 / ri;
		}

		// WPR = PR * W_log
		double wpr = pr * w_log;

		if (minWPR < 0 || wpr < minWPR) {
			minWPR = wpr;
		}
	}

	return (minWPR < 0) ? 0.0 : minWPR;
};

// Generate all size-1 subsets (remove one feature at a time)
std::set<Colocation> Miner::generateSubsets(const Colocation& c) {
	//////////////////////////////////////
	//////// TODO: Implement (13)/////////
	//////////////////////////////////////

	return {};

};

// Deduce prevalent subsets using downward closure property
std::set<Colocation> Miner::deducePrevalentSubsets(
	std::set<Colocation>& subsets,
	const Colocation& c){
	//////////////////////////////////////
	//////// TODO: Implement (16)/////////
	//////////////////////////////////////

	return {};

};
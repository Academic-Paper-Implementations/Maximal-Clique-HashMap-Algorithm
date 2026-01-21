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


// Main mining algorithm: find all prevalent colocation patterns
std::set<Colocation> Miner::minePCPs(
	std::priority_queue<Colocation>& candidateColocations,
	const std::unordered_map<Colocation, std::map<FeatureType, std::set<SpatialInstance*>>>& hashMap,
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
			double weightedPI = computeWeightedPI(partInstances, c, rareIntensityMap);
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
std::map<FeatureType, std::set<SpatialInstance*>> Miner::queryInstances(
	Colocation c,
	const std::unordered_map<Colocation, std::map<FeatureType, std::set<SpatialInstance*>>>& hashMap) {
	//////////////////////////////////////
	//////// TODO: Implement (10)/////////
	//////////////////////////////////////



};

// Compute weighted participation index for a colocation
double Miner::computeWeightedPI(
	const std::map<FeatureType, std::set<SpatialInstance*>>& partInstances,
	Colocation c,
	const std::unordered_map<InstanceID, double>& rareIntensityMap) {
	//////////////////////////////////////
	//////// TODO: Implement (12)/////////
	//////////////////////////////////////



};

// Generate all size-1 subsets (remove one feature at a time)
std::set<Colocation> Miner::generateSubsets(const Colocation& c) {
	//////////////////////////////////////
	//////// TODO: Implement (13)/////////
	//////////////////////////////////////



};

// Deduce prevalent subsets using downward closure property
std::set<Colocation> Miner::deducePrevalentSubsets(
	std::set<Colocation>& subsets,
	const Colocation& c){
	//////////////////////////////////////
	//////// TODO: Implement (16)/////////
	//////////////////////////////////////



};
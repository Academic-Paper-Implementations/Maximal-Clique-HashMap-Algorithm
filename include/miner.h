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
	 std::map<Colocation, bool> deducedMap_;
 
	 std::map<FeatureType, std::set<const SpatialInstance*>> queryInstances(
		 Colocation c,
		 const std::map<Colocation, std::unordered_map<FeatureType, std::set<const SpatialInstance*>>>& hashMap);
 
	 double computeWeightedPI(
		 const std::map<FeatureType, std::set<const SpatialInstance*>>& partInstances,
		 Colocation c,
		 const std::unordered_map<FeatureType, double>& rareIntensityMap,
		 const std::map<FeatureType, int>& featureCounts);
 
	 std::set<Colocation> generateSubsets(const Colocation& c);
 
	 std::set<Colocation> deducePrevalentSubsets(std::set<Colocation>& subsets, const Colocation& c, const std::map<FeatureType, int>& featureCounts);
 
 public:
	 std::set<Colocation> minePCPs(
		 std::priority_queue<Colocation, std::vector<Colocation>, ColocationPriorityComp>& candidateColocations,
		 const std::map<Colocation, std::unordered_map<FeatureType, std::set<const SpatialInstance*>>>& hashMap,
		 const std::map<FeatureType, int>& featureCounts,
		 double delta,
		 double min_prev
	 );
 
	 const std::map<Colocation, bool>& getDeducedMap() const { return deducedMap_; }
 };
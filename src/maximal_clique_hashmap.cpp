/**
 * @file maximal_clique_hashmap.cpp
 * @brief Implementation: Maximal clique enumeration and hashmap construction
 */

#include "maximal_clique_hashmap.h"

// Execute Bron-Kerbosch algorithm to find all maximal cliques
std::vector<std::vector<ColocationInstance>> MaximalCliqueHashmap::executeDivBK(
	const std::vector<NeighborSet>& neighborSets) {
	//////////////////////////////////////
	//////// TODO: Implement (4)//////////
	//////////////////////////////////////



};

// Build instance hashmap from maximal cliques
std::unordered_map<Colocation, std::map<FeatureType, std::set<SpatialInstance*>>> MaximalCliqueHashmap::buildInstanceHash(
	const std::vector<NeighborSet>& neighborSets) {
	//////////////////////////////////////
	//////// TODO: Implement (5)//////////
	//////////////////////////////////////



};

// Extract initial candidate colocations from hashmap
std::priority_queue<Colocation> MaximalCliqueHashmap::extractInitialCandidates(
	const std::unordered_map<Colocation, std::map<FeatureType, std::set<SpatialInstance*>>>& hashMap) {
	//////////////////////////////////////
	//////// TODO: Implement (6)//////////
	//////////////////////////////////////



};
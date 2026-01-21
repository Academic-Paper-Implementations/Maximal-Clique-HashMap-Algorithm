/**
 * @file neighbor_graph.cpp
 * @brief Implementation: Build spatial neighbor graph from instances
 */

#include "neighbor_graph.h"

// Calculate Euclidean distance between two spatial instances
double NeighborGraph::euclideanDist(const SpatialInstance& a, const SpatialInstance& b) {
	
};

// Find all neighbor pairs within distance threshold
std::vector<std::pair<SpatialInstance, SpatialInstance>> NeighborGraph::findNeighborPair(
	const std::vector<SpatialInstance>& instances,
	double distanceThreshold) {

};

// Build neighbor graph: create NeighborSet for each instance
std::vector<NeighborSet> NeighborGraph::buildNeighborGraph(
	const std::vector<SpatialInstance>& instances,
	double distanceThreshold) {
	//////////////////////////////////////
	//////// TODO: Implement (3)//////////
	//////////////////////////////////////



};
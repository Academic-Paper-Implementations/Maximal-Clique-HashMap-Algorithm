/**
 * @file maximal_clique_hashmap.cpp
 * @brief Implementation: Standard Bron-Kerbosch with Pivot
 */

#include "maximal_clique_hashmap.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>
#include <map>
#include <set>

namespace {

	using NodeID = int; // Internal ID (0, 1, 2... N-1)
	using CliqueVec = std::vector<NodeID>;

	// Union of two sorted vectors
	CliqueVec set_union_helper(const CliqueVec& A, const CliqueVec& B) {
		CliqueVec result;
		result.reserve(A.size() + B.size());
		std::set_union(A.begin(), A.end(), B.begin(), B.end(), std::back_inserter(result));
		return result;
	}

	// Intersection of two sorted vectors
	CliqueVec set_intersection_helper(const CliqueVec& A, const CliqueVec& B) {
		CliqueVec result;
		// Optimization: reserve distinct memory size based on smaller set
		result.reserve(std::min(A.size(), B.size()));
		std::set_intersection(A.begin(), A.end(), B.begin(), B.end(), std::back_inserter(result));
		return result;
	}

	// Difference of two sorted vectors (A \ B)
	CliqueVec set_difference_helper(const CliqueVec& A, const CliqueVec& B) {
		CliqueVec result;
		result.reserve(A.size());
		std::set_difference(A.begin(), A.end(), B.begin(), B.end(), std::back_inserter(result));
		return result;
	}
/**
	     * @brief Standard Recursive Bron-Kerbosch with Pivot
	     * Algorithm:
	     * 1. Select pivot u from P U X (maximize |P n N(u)|)
	     * 2. For each v in P \ N(u):
	     * Recurse(R + v, P n N(v), X n N(v))
	     * P = P - v
	     * X = X + v
	     */
	int count_intersection(const CliqueVec& A, const CliqueVec& B) {
		int i = 0, j = 0, count = 0;
		while (i < A.size() && j < B.size()) {
			if (A[i] == B[j]) { count++; i++; j++; }
			else if (A[i] < B[j]) i++;
			else j++;
		}
		return count;
	}

	void runBronKerbosch(
		CliqueVec R,
		CliqueVec P,
		CliqueVec X,
		const std::vector<std::vector<NodeID>>& adj,
		std::vector<CliqueVec>& cliques)
	{
		if (P.empty() && X.empty()) {
			if (R.size() >= 2) { // Only save cliques with size >= 2
				cliques.push_back(R);
			}
			return;
		}

		if (P.empty()) return;

		// --- Pivot Selection Strategy ---
		// Find pivot u in P U X that maximizes |P intersect N(u)|
		// Why? To minimize the number of recursive calls (candidates = P \ N(u))

		int u_pivot = -1;
		int max_intersection_size = -1;

		// We can iterate over P and X separately to avoid creating a union vector
		auto check_pivot = [&](int candidate_node) {
			const auto& neighbors = adj[candidate_node];

			int inter_size = count_intersection(P, neighbors);

			if (inter_size > max_intersection_size) {
				max_intersection_size = inter_size;
				u_pivot = candidate_node;
			}
		};

		for (int node : P) check_pivot(node);
		for (int node : X) check_pivot(node);

		// --- Candidates Calculation: P \ N(pivot) ---
		CliqueVec candidates;
		if (u_pivot != -1) {
			candidates = set_difference_helper(P, adj[u_pivot]);
		}
		else {
			candidates = P;
		}

		// --- Recursive Step ---
		for (int v : candidates) {
			CliqueVec newR = R;
			newR.push_back(v);

			CliqueVec newP;
			if (P.size() < adj[v].size())
				newP = set_intersection_helper(P, adj[v]);
			else
				newP = set_intersection_helper(adj[v], P);

			CliqueVec newX;
			if (X.size() < adj[v].size())
				newX = set_intersection_helper(X, adj[v]);
			else
				newX = set_intersection_helper(adj[v], X);

			runBronKerbosch(newR, newP, newX, adj, cliques);
		}
	}
}

// ============================================================================
// PUBLIC METHODS IMPLEMENTATION
// ============================================================================

// Although named "executeDivBK" to keep interface compatibility, 
// this now runs the Standard Recursive Bron-Kerbosch algorithm.
std::vector<ColocationInstance> MaximalCliqueHashmap::executeDivBK(
	const std::vector<NeighborSet>& neighborSets) {

	// --- Step 1: Mapping SpatialInstance* to Integer ID (0..N-1) ---
	std::map<std::pair<FeatureType, InstanceID>, int> uniqueNodeMap;
	std::vector<const SpatialInstance*> internalToInstance;
	int nextInternalID = 0;

	auto getInternalID = [&](const SpatialInstance* inst) {
		std::pair<FeatureType, InstanceID> key = { inst->type, inst->id };
		if (uniqueNodeMap.find(key) == uniqueNodeMap.end()) {
			uniqueNodeMap[key] = nextInternalID++;
			internalToInstance.push_back(inst);
		}
		return uniqueNodeMap[key];
		};

	// --- Step 2: Build Adjacency List (Dense Graph) ---
	std::vector<std::set<int>> tempAdj;
	for (const auto& ns : neighborSets) {
		int u = getInternalID(ns.center);
		if (tempAdj.size() <= u) tempAdj.resize(u + 1);

		for (const SpatialInstance* neighbor : ns.neighbors) {
			int v = getInternalID(neighbor);
			if (tempAdj.size() <= v) tempAdj.resize(v + 1);

			if (u != v) {
				tempAdj[u].insert(v);
				tempAdj[v].insert(u);
			}
		}
	}

	int num_nodes = internalToInstance.size();
	std::vector<std::vector<int>> adj(num_nodes);
	for (int i = 0; i < num_nodes; ++i) {
		if (i < tempAdj.size()) {
			adj[i].assign(tempAdj[i].begin(), tempAdj[i].end());
		}
	}

	// --- Step 3: Prepare Initial Sets for BK ---
	CliqueVec R;
	CliqueVec P(num_nodes); // All nodes [0, 1, ... N-1]
	for (int i = 0; i < num_nodes; ++i) P[i] = i;
	CliqueVec X;

	std::vector<CliqueVec> resultIDs;

	// --- Step 4: Run Recursive Algorithm ---
	runBronKerbosch(R, P, X, adj, resultIDs);

	// --- Step 5: Convert Integer IDs back to ColocationInstance ---
	std::vector<ColocationInstance> finalResult;
	finalResult.reserve(resultIDs.size());

	for (const auto& clique : resultIDs) {
		ColocationInstance col;
		col.reserve(clique.size());
		for (int id : clique) {
			col.push_back(internalToInstance[id]);
		}
		finalResult.push_back(col);
	}

	return finalResult;
}

// Build instance hashmap from maximal cliques (Remains unchanged logic)
std::map<Colocation, std::map<FeatureType, std::set<const SpatialInstance*>>> MaximalCliqueHashmap::buildInstanceHash(
	const std::vector<NeighborSet>& neighborSets) {

	std::vector<ColocationInstance> bk_result = MaximalCliqueHashmap::executeDivBK(neighborSets);
	std::map<Colocation, std::map<FeatureType, std::set<const SpatialInstance*>>> hashMap;

	// Process each clique
	for (const auto& clique : bk_result) {
		// Build colocation key
		Colocation colocationKey;
		colocationKey.reserve(clique.size());
		for (const auto& instancePtr : clique) {
			colocationKey.push_back(instancePtr->type);
		}

		std::sort(colocationKey.begin(), colocationKey.end());

		// Insert instances into hashmap
		for (const auto& instancePtr : clique) {
			hashMap[colocationKey][instancePtr->type].insert(instancePtr);
		}
	}

	return hashMap;
}

// Extract initial candidate colocations from hashmap (Remains unchanged logic)
std::priority_queue<Colocation, std::vector<Colocation>, ColocationPriorityComp> MaximalCliqueHashmap::extractInitialCandidates(
	const std::map<Colocation, std::map<FeatureType, std::set<const SpatialInstance*>>>& hashMap) {

	std::priority_queue<Colocation, std::vector<Colocation>, ColocationPriorityComp> candidateQueue;
	for (const auto& entry : hashMap) {
		const Colocation& maximalClique = entry.first;
		candidateQueue.push(maximalClique);
	}
	return candidateQueue;
}
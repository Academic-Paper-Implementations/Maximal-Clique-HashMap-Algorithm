/**
 * @file maximal_clique_hashmap.cpp
 * @brief Implementation: Maximal clique enumeration and hashmap construction
 */

#include "maximal_clique_hashmap.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <optional>
#include <map>
#include <set>
#include <vector>
#include <utility>

namespace {

    using NodeID = int; // Internal ID (0, 1, 2... N-1)
    using CliqueVec = std::vector<NodeID>;

    // Union of two sorted sets
    CliqueVec set_intersection_helper(const CliqueVec& A, const CliqueVec& B) {
        CliqueVec result;
        std::set_intersection(A.begin(), A.end(), B.begin(), B.end(), std::back_inserter(result));
        return result;
    }

    // Difference of two sorted sets (A \ B)
    CliqueVec set_difference_helper(const CliqueVec& A, const CliqueVec& B) {
        CliqueVec result;
        std::set_difference(A.begin(), A.end(), B.begin(), B.end(), std::back_inserter(result));
        return result;
    }

    // Union of two sorted sets
    CliqueVec set_union_helper(const CliqueVec& A, const CliqueVec& B) {
        CliqueVec result;
        std::set_union(A.begin(), A.end(), B.begin(), B.end(), std::back_inserter(result));
        return result;
    }
}

class BKGenerator {
public:
    int depth;
    CliqueVec R, P, X;
    const std::vector<std::vector<NodeID>>* adj;

    CliqueVec candidates;
    int current_candidate_idx;
    std::shared_ptr<BKGenerator> sub_generator;
    bool is_finished;

    BKGenerator(CliqueVec r, CliqueVec p, CliqueVec x, const std::vector<std::vector<NodeID>>* g, int d)
        : R(std::move(r)), P(std::move(p)), X(std::move(x)), adj(g), depth(d),
        current_candidate_idx(0), is_finished(false), sub_generator(nullptr)
    {
        // 1. Pivot Strategy
        if (P.empty() && X.empty()) return;

        CliqueVec PUX = set_union_helper(P, X);
        int u = -1;
        int max_inter = -1;

        for (int node : PUX) {
            const auto& neighbors = (*adj)[node];
            CliqueVec inter = set_intersection_helper(P, neighbors);
            if ((int)inter.size() > max_inter) {
                max_inter = inter.size();
                u = node;
            }
        }

        // 2. Candidates = P \ N(u)
        if (u != -1) {
            const auto& neighbors_u = (*adj)[u];
            candidates = set_difference_helper(P, neighbors_u);
        }
        else {
            candidates = P;
        }
    }

    std::optional<CliqueVec> nextClique() {
        if (P.empty() && X.empty()) {
            if (!is_finished) {
                is_finished = true;
                return R;
            }
            return std::nullopt;
        }

        while (current_candidate_idx < candidates.size() || sub_generator != nullptr) {
            if (sub_generator == nullptr) {
                sub_generator = createNextBranch();
                if (sub_generator == nullptr) break;
            }

            auto result = sub_generator->nextClique();
            if (result.has_value()) {
                return result;
            }
            else {
                sub_generator = nullptr;
            }
        }

        is_finished = true;
        return std::nullopt;
    }

    std::shared_ptr<BKGenerator> createNextBranch() {
        if (current_candidate_idx >= candidates.size()) return nullptr;

        int v = candidates[current_candidate_idx];
        CliqueVec R_new = R; R_new.push_back(v); std::sort(R_new.begin(), R_new.end());

        const auto& N_v = (*adj)[v];
        CliqueVec P_new = set_intersection_helper(P, N_v);
        CliqueVec X_new = set_intersection_helper(X, N_v);

        auto child = std::make_shared<BKGenerator>(R_new, P_new, X_new, adj, depth + 1);
        current_candidate_idx++;
        return child;
    }
};

// Execute Bron-Kerbosch algorithm to find all maximal cliques
std::vector<ColocationInstance> MaximalCliqueHashmap::executeDivBK(
	const std::vector<NeighborSet>& neighborSets) {
    // Key: <Type, ID>, Value int(0, 1, 2...)
    std::map<std::pair<FeatureType, InstanceID>, int> uniqueNodeMap;

    // Vector reversed map int -> Pointer
    std::vector<const SpatialInstance*> internalToInstance;

    int nextInternalID = 0;

    auto getInternalID = [&](const SpatialInstance* inst) {
		// create unique key
        std::pair<FeatureType, InstanceID> key = { inst->type, inst->id };

        if (uniqueNodeMap.find(key) == uniqueNodeMap.end()) {
            uniqueNodeMap[key] = nextInternalID++;
            internalToInstance.push_back(inst);
        }
        return uniqueNodeMap[key];
        };

    // Create adjacency graph
    std::vector<std::set<int>> tempAdj;

    for (const auto& ns : neighborSets) {
        int u = getInternalID(ns.center);

        if (tempAdj.size() <= u) tempAdj.resize(u + 1);

        for (const SpatialInstance* neighbor : ns.neighbors) {
            int v = getInternalID(neighbor);

            if (tempAdj.size() <= v) tempAdj.resize(v + 1);

            // Undirected Graph
            if (u != v) {
                tempAdj[u].insert(v);
                tempAdj[v].insert(u);
            }
        }
    }

    //Change data structure to vector<vector<int>> (Dense Graph Format)
    int num_nodes = internalToInstance.size();
    std::vector<std::vector<int>> adj(num_nodes);
    for (int i = 0; i < num_nodes; ++i) {
        if (i < tempAdj.size()) {
            adj[i].assign(tempAdj[i].begin(), tempAdj[i].end());
        }
    }

    //Execute BK Algorithm
    CliqueVec V(num_nodes);
    for (int i = 0; i < num_nodes; ++i) V[i] = i;

    const int DIV_BK_N = 10;
    std::list<std::shared_ptr<BKGenerator>> active_generators;
    active_generators.push_back(std::make_shared<BKGenerator>(CliqueVec{}, V, CliqueVec{}, &adj, 0));

    //std::vector<ColocationInstance> flatCliques;
    std::set<CliqueVec> uniqueCliques;

    while (!active_generators.empty()) {
        auto it = active_generators.begin();
        while (it != active_generators.end()) {
            auto g = *it;
            auto cliqueOpt = g->nextClique();

            if (cliqueOpt.has_value()) {
                // --- int -> ColocationInstance ---
                CliqueVec resultIDs = cliqueOpt.value();
                std::sort(resultIDs.begin(), resultIDs.end());
                uniqueCliques.insert(resultIDs);

                /*ColocationInstance colocationResult;
                colocationResult.reserve(resultIDs.size());*/

                /*for (int id : resultIDs) {
                    colocationResult.push_back(internalToInstance[id]);
                }

                flatCliques.push_back(colocationResult);*/
                ++it;
            }
            else {
                it = active_generators.erase(it);
            }
        }

        // Branching logic
        if (active_generators.size() < DIV_BK_N && !active_generators.empty()) {
            auto top_it = active_generators.begin();
            int min_depth = 999999;
            for (auto it_scan = active_generators.begin(); it_scan != active_generators.end(); ++it_scan) {
                if ((*it_scan)->depth < min_depth) {
                    min_depth = (*it_scan)->depth;
                    top_it = it_scan;
                }
            }
            auto new_gen = (*top_it)->createNextBranch();
            if (new_gen) active_generators.push_back(new_gen);
        }
    }

    // --- CHANGE 3: Chuyển từ Set (Unique) sang Vector Output ---
    std::vector<ColocationInstance> finalResult;
    finalResult.reserve(uniqueCliques.size());

    for (const auto& cliqueIDs : uniqueCliques) {
        ColocationInstance colocationResult;
        colocationResult.reserve(cliqueIDs.size());

        for (int id : cliqueIDs) {
            colocationResult.push_back(internalToInstance[id]);
        }
        finalResult.push_back(colocationResult);
    }

    return finalResult;
};

// Build instance hashmap from maximal cliques
std::map<Colocation, std::map<FeatureType, std::set<const SpatialInstance*>>> MaximalCliqueHashmap::buildInstanceHash(
	const std::vector<NeighborSet>& neighborSets) {
    std::vector<ColocationInstance> divBk_result = MaximalCliqueHashmap::executeDivBK(neighborSets);
	std::map<Colocation, std::map<FeatureType, std::set<const SpatialInstance*>>> hashMap;
    // Process each clique
    for (const auto& clique : divBk_result) {
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
};

// Extract initial candidate colocations from hashmap
std::priority_queue<Colocation> MaximalCliqueHashmap::extractInitialCandidates(
	const std::map<Colocation, std::map<FeatureType, std::set<const SpatialInstance*>>>& hashMap) {
	//////////////////////////////////////
	//////// TODO: Implement (6)//////////
	//////////////////////////////////////
    return {};


};
/**
 * @file main.cpp
 * @brief Simplified Entry point for Co-location Mining
 */

#include "config.h"
#include "data_loader.h"
#include "neighbor_graph.h"
#include "maximal_clique_hashmap.h"
#include "miner.h"
#include "types.h"
#include "utils.h"
#include <iostream>
#include <chrono>
#include <iomanip>

int main(int argc, char* argv[]) {
    auto programStart = std::chrono::high_resolution_clock::now();


    // --- Step 1: Config & Load Data ---
    std::cout << "[1/3] Loading Configuration and Data...\n";
    std::string config_path = (argc > 1) ? argv[1] : "./config/config.txt";
    AppConfig config = ConfigLoader::load(config_path);

    auto instances = DataLoader::load_csv(config.datasetPath);
    std::cout << "      Dataset: " << config.datasetPath << " | Size: " << instances.size() << " instances\n";


    // --- Step 2: Pre-processing (Indexing & Structures) ---
	std::cout << "[2/3] Building Graph Structures and Hashmap...\n";

    // 1. Feature Counting & Sorting
    auto featureCount = countAndSortFeatures(instances);

	// 2. Delta Calculation
	double delta = calculateDirpersion(featureCount);

	// 3. Neighbor Graph Building
    NeighborGraph neighborGraph;
    auto graph = neighborGraph.buildNeighborGraph(instances, config.neighborDistance);

	// 4. Build Instance Hashmap from Maximal Cliques
	MaximalCliqueHashmap mcHashmap;
    auto hashMap = mcHashmap.buildInstanceHash(graph);

	// 5. Get Candidate Colocations
	auto candidateQueue = mcHashmap.extractInitialCandidates(hashMap);

    // --- Step 3: Mining Prevalent Co-location Patterns ---
    std::cout << "[3/3] Mining Patterns (MinPrev: " << config.minPrev << ", Dist: " << config.neighborDistance << ")...\n";

    Miner miner;
    auto colocations = miner.minePCPs(
        candidateQueue,
        hashMap,
        featureCount,
        delta,
        config.minPrev
	);

    // --- Final Report ---
    auto programEnd = std::chrono::high_resolution_clock::now();
    double totalTime = std::chrono::duration<double>(programEnd - programStart).count();;

    std::cout << "\n" << std::string(40, '=') << "\n";
    std::cout << "SUMMARY REPORT\n";
    std::cout << "Time:   " << std::fixed << std::setprecision(3) << totalTime << " s\n";
    std::cout << "Found:  " << colocations.size() << " patterns\n";
    std::cout << std::string(40, '=') << "\n";

    if (!colocations.empty()) {
        int idx = 1;
        for (const auto& col : colocations) {
            std::cout << "[" << idx++ << "] {";
            for (size_t i = 0; i < col.size(); ++i) {
                std::cout << (i > 0 ? ", " : "") << col[i];
            }
            std::cout << "}\n";
        }
    }
    else {
        std::cout << "No patterns found.\n";
    }

    return 0;
}
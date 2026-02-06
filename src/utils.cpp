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
std::map<FeatureType, int> countFeatures(
	const std::vector<SpatialInstance>& instances) {
	//////// TODO: Implement (1)//////////
	std::map<FeatureType, int> counts;
	for (const auto& instance : instances) {
		counts[instance.type]++;
	}
	return counts;
};

// Calculate dispersion (delta) from feature distribution
double calculateDispersion(const std::map<FeatureType, int>& featureCount) {
	//////// TODO: Implement (2)//////////
	if (featureCount.empty()) return 0.0;
	if (featureCount.size() == 1) return 0.0;

	// --- Bước 1: Trích xuất và Sắp xếp (Sort) ---
	// Chuyển tần suất từ Map sang Vector để sắp xếp
	std::vector<double> frequencies;
	frequencies.reserve(featureCount.size());

	for (const auto& pair : featureCount) {
		frequencies.push_back(static_cast<double>(pair.second));
	}

	// Sắp xếp tăng dần (Ascending) theo yêu cầu
	std::sort(frequencies.begin(), frequencies.end());

	// --- Bước 2: Tính Logarit tự nhiên (ln) cho từng phần tử ---
	// Tính trước log để tránh gọi hàm log() nhiều lần trong vòng lặp lồng nhau
	std::vector<double> logFrequencies;
	logFrequencies.reserve(frequencies.size());
	for (double f : frequencies) {
		logFrequencies.push_back(std::log(f)); // ln(N(fi))
	}

	// --- Bước 3: Tính tổng bình phương hiệu các cặp (Sum of squared pairwise differences) ---
	// Công thức: sum_{i<j} (ln(Nj) - ln(Ni))^2
	double sumSqDiff = 0.0;
	size_t m = logFrequencies.size();

	for (size_t i = 0; i < m; ++i) {
		for (size_t j = i + 1; j < m; ++j) {
			// ln(Nj / Ni) = ln(Nj) - ln(Ni)
			// Vì đã sort tăng dần và j > i, nên logFrequencies[j] >= logFrequencies[i]
			double diff = logFrequencies[j] - logFrequencies[i];
			sumSqDiff += diff * diff;
		}
	}

	// --- Bước 4: Áp dụng hệ số tỉ lệ và Căn bậc hai ---
	// Hệ số: 2 / (m * (m - 1))
	double factor = 2.0 / (static_cast<double>(m) * (static_cast<double>(m) - 1.0));

	double sigma_log = std::sqrt(factor * sumSqDiff);

	return sigma_log;
};

// Calculate rare intensity for each instance in a colocation
std::unordered_map<FeatureType, double> calcRareIntensity(
	Colocation c,
	const std::map<FeatureType, int>& featureCounts,
	double delta) {
		//////// TODO: Implement (11)//////////
	std::unordered_map<FeatureType, double> intensityMap;
	if (c.empty()) return intensityMap;

	// 1. Find N(f_min)
	// Initialize with first feature's count
	int minCount = -1;
	
	for (const auto& f : c) {
		if (featureCounts.find(f) != featureCounts.end()) {
			int count = featureCounts.at(f);
			if (minCount == -1 || count < minCount) {
				minCount = count;
			}
		}
	}

	if (minCount <= 0) return intensityMap; 

	// 2. Calculate Rare Intensity (RI) and combined weight factor
	double sigmaSq2 = 2.0 * delta * delta;
	if (sigmaSq2 == 0) sigmaSq2 = 1e-9; 

	double logMin = std::log(static_cast<double>(minCount));

	for (const auto& f : c) {
		if (featureCounts.find(f) != featureCounts.end()) {
			int count = featureCounts.at(f);
			if (count > 0) {
				// Step 2: Calculate Delta_log
				double logCount = std::log(static_cast<double>(count));
				double deltaLog = logCount - logMin;

				// Step 3: Calculate RI
				double ri = std::exp(-(deltaLog * deltaLog) / sigmaSq2);

				// Store result (Mapping FeatureType -> RI)
				intensityMap[f] = ri;
			}
		}
	}

	return intensityMap;
};
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
#include <iomanip>

namespace {

	using Node = const SpatialInstance*; // Internal ID (0, 1, 2... N-1)
	using CliqueVec = std::vector<Node>;

	using AdjMap = std::unordered_map<Node, CliqueVec>;

	// --- HELPER FUNCTIONS (Pointer Arithmetic) ---

	// Helper: Đếm số phần tử chung (Intersection Size) để chọn Pivot
	// Logic giống hệt phiên bản int, nhưng so sánh địa chỉ pointer
	int count_intersection(const CliqueVec& A, const CliqueVec& B) {
		int count = 0;
		auto it1 = A.begin();
		auto it2 = B.begin();

		while (it1 != A.end() && it2 != B.end()) {
			if (*it1 < *it2) {
				++it1;
			}
			else if (*it2 < *it1) {
				++it2;
			}
			else { // *it1 == *it2
				++count;
				++it1;
				++it2;
			}
		}
		return count;
	}

	// Helper: P \ N(u)
	CliqueVec set_difference_helper(const CliqueVec& A, const CliqueVec& B) {
		CliqueVec result;
		result.reserve(A.size());
		std::set_difference(A.begin(), A.end(), B.begin(), B.end(), std::back_inserter(result));
		return result;
	}

	// Helper: P intersection N(u)
	CliqueVec set_intersection_helper(const CliqueVec& A, const CliqueVec& B) {
		CliqueVec result;
		result.reserve(std::min(A.size(), B.size()));
		std::set_intersection(A.begin(), A.end(), B.begin(), B.end(), std::back_inserter(result));
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

		 // --- CORE RECURSIVE ALGORITHM ---

	void runBronKerbosch(
		CliqueVec R,
		CliqueVec P,
		CliqueVec X,
		const AdjMap& adj,           // Map tra cứu nhanh neighbors
		std::vector<ColocationInstance>& final_cliques,
		int depth = 0)
	{
		if (P.empty() && X.empty()) {
			if (R.size() >= 2) {
				// R chính là ColocationInstance (vector<const SpatialInstance*>)
				// Không cần convert gì cả, đẩy thẳng vào kết quả!
				final_cliques.push_back(R);
			}
			return;
		}

		if (P.empty()) return;

		// --- 1. Pivot Selection ---
		// Chọn u thuộc P U X sao cho |P giao N(u)| lớn nhất
		Node u_pivot = nullptr;
		int max_inter = -1;

		auto check_pivot = [&](Node candidate) {
			auto it = adj.find(candidate);
			if (it != adj.end()) {
				// Đếm giao của P và neighbors của candidate
				int inter_size = count_intersection(P, it->second);
				if (inter_size > max_inter) {
					max_inter = inter_size;
					u_pivot = candidate;
				}
			}
			};

		for (Node node : P) check_pivot(node);
		for (Node node : X) check_pivot(node);

		// --- 2. Candidates Calculation: P \ N(pivot) ---
		CliqueVec candidates;
		if (u_pivot != nullptr) {
			auto it = adj.find(u_pivot);
			if (it != adj.end()) {
				candidates = set_difference_helper(P, it->second);
			}
			else {
				candidates = P; // Trường hợp hiếm: Pivot không có hàng xóm
			}
		}
		else {
			candidates = P;
		}

		// --- LOGIC HIỂN THỊ TIẾN ĐỘ (Chỉ chạy ở Depth 0) ---
		int total_candidates = 0;
		int processed_count = 0;

		if (depth == 0) {
			total_candidates = candidates.size();
			std::cout << ">>> Start Bron-Kerbosch. Root candidates: " << total_candidates
				<< " (Optimized from " << P.size() << " nodes by Pivot)" << std::endl;
		}

		// --- 3. Recursive Step ---
		for (Node v : candidates) {
			// Tạo R mới: R U {v}
			CliqueVec newR = R;
			newR.push_back(v);

			// Lấy danh sách hàng xóm của v để tính giao
			CliqueVec neighbors_v;
			auto it = adj.find(v);
			if (it != adj.end()) {
				neighbors_v = it->second;
			}

			// P mới = P giao N(v)
			CliqueVec newP = set_intersection_helper(P, neighbors_v);

			// X mới = X giao N(v)
			CliqueVec newX = set_intersection_helper(X, neighbors_v);

			// Đệ quy
			runBronKerbosch(newR, newP, newX, adj, final_cliques, depth + 1);

			// --- Backtracking Step ---
			// Sau khi xét v xong, chuyển v từ P sang X
			// Vì ta đang iter qua 'candidates' (là bản sao), ta cần sửa P và X thực tế
			// để dùng cho vòng lặp tiếp theo của tầng này.

			// Xóa v khỏi P (P đang sorted)
			// Vì P là sorted vector, ta dùng lower_bound để tìm nhanh
			auto itP = std::lower_bound(P.begin(), P.end(), v);
			if (itP != P.end() && *itP == v) {
				P.erase(itP);
			}

			// Thêm v vào X (X cần giữ sorted)
			auto itX = std::lower_bound(X.begin(), X.end(), v);
			X.insert(itX, v);

			// --- CẬP NHẬT TIẾN ĐỘ ---
			if (depth == 0) {
				processed_count++;
				// In ra mỗi 10 node hoặc 1% để đỡ lag console
				if (processed_count % 10 == 0 || processed_count == total_candidates) {
					float percent = (float)processed_count / total_candidates * 100.0f;
					std::cout << "\r[Progress]: " << processed_count << "/" << total_candidates
						<< " root branches (" << std::fixed << std::setprecision(1) << percent << "%)" << std::flush;
				}
			}
		}
		// Xuống dòng khi chạy xong depth 0
		if (depth == 0) {
			std::cout << "\r[Progress]: Done! Found " << final_cliques.size() << " maximal cliques.           " << std::endl;
		}
	}
}

// ============================================================================
// PUBLIC METHODS IMPLEMENTATION
// ============================================================================

std::vector<ColocationInstance> MaximalCliqueHashmap::executeDivBK(
	const std::vector<NeighborSet>& neighborSets) {

	// --- Step 1: Build Adjacency Map Directly (Pointer-based) ---
	// Ta dùng unordered_map<Node, vector<Node>> làm Adjacency List.
	// KEY: con trỏ center. VALUE: vector các con trỏ neighbors.
	std::cout << "[MaximalCliqueHashmap] Building Adjacency Map for Bron-Kerbosch...\n";
	AdjMap adj;
	// Dự trù bộ nhớ để tránh rehash
	adj.reserve(neighborSets.size());

	CliqueVec P;
	P.reserve(neighborSets.size());

	for (const auto& ns : neighborSets) {
		Node u = ns.center;
		P.push_back(u);

		// CRITICAL STEP: Copy neighbors và SORT theo địa chỉ pointer.
		// Thuật toán Bron-Kerbosch dùng set_intersection, bắt buộc vector phải sorted.
		CliqueVec sorted_neighbors = ns.neighbors;
		std::sort(sorted_neighbors.begin(), sorted_neighbors.end());

		// Move vector đã sort vào map
		adj[u] = std::move(sorted_neighbors);
	}
	std::cout << "Done building Adjacency Map. Total Nodes: " << adj.size() << "\n";
	// P (tập đỉnh ban đầu) cũng phải được sort
	std::sort(P.begin(), P.end());

	// --- Step 2: Prepare BK variables ---
	CliqueVec R; // Tập clique đang xây dựng (rỗng)
	CliqueVec X; // Tập đỉnh đã xét (rỗng)

	std::vector<ColocationInstance> finalResult;
	std::cout << "[MaximalCliqueHashmap] Running Bron-Kerbosch Algorithm...\n";
	// --- Step 3: Run BK Algorithm ---
	runBronKerbosch(R, P, X, adj, finalResult, 0);
	std::cout << "Bron-Kerbosch completed. Total Maximal Cliques found: " << finalResult.size() << "\n";
	return finalResult;
}

// Build instance hashmap from maximal cliques (Remains unchanged logic)
std::map<Colocation, std::unordered_map<FeatureType, std::set<const SpatialInstance*>>> MaximalCliqueHashmap::buildInstanceHash(
	const std::vector<NeighborSet>& neighborSets) {
	std::cout << "[BuildHash] Executing Bron-Kerbosch..." << std::endl;
	std::vector<ColocationInstance> bk_result = MaximalCliqueHashmap::executeDivBK(neighborSets);

	std::cout << "[BuildHash] Constructing HashMap from " << bk_result.size() << " cliques..." << std::endl;
	std::map<Colocation, std::unordered_map<FeatureType, std::set<const SpatialInstance*>>> hashMap;

	Colocation colocationKey;
	colocationKey.reserve(16);


	// Process each clique
	for (const auto& clique : bk_result) {
		// Build colocation key
		colocationKey.clear();

		for (const auto& instancePtr : clique) {
			colocationKey.push_back(instancePtr->type);
		}

		std::sort(colocationKey.begin(), colocationKey.end());

		auto& innerMap = hashMap[colocationKey];

		// Insert instances into hashmap
		for (const auto& instancePtr : clique) {
			innerMap[instancePtr->type].insert(instancePtr);
		}
	}
	std::cout << "[BuildHash] Done." << std::endl;
	return hashMap;
}

// Extract initial candidate colocations from hashmap (Remains unchanged logic)
std::priority_queue<Colocation, std::vector<Colocation>, ColocationPriorityComp> MaximalCliqueHashmap::extractInitialCandidates(
	const std::map<Colocation, std::unordered_map<FeatureType, std::set<const SpatialInstance*>>>& hashMap) {

	std::priority_queue<Colocation, std::vector<Colocation>, ColocationPriorityComp> candidateQueue;
	for (const auto& entry : hashMap) {
		const Colocation& maximalClique = entry.first;
		candidateQueue.push(maximalClique);
	}
	return candidateQueue;
}
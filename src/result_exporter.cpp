#include "result_exporter.h"
#include "utils.h"
#include <sqlite3.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

static void execSQL(sqlite3* db, const std::string& sql) {
    char* errMsg = nullptr;

    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        std::string error = errMsg ? errMsg : "Unknown SQLite error";
        sqlite3_free(errMsg);
        throw std::runtime_error(error);
    }
}

static std::string readFileToString(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open schema file: " + path);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static std::string makeFeatureKey(Colocation colocation) {
    std::sort(colocation.begin(), colocation.end());

    std::ostringstream oss;
    for (size_t i = 0; i < colocation.size(); ++i) {
        if (i > 0) oss << ",";
        oss << colocation[i];
    }

    return oss.str();
}


void ResultExporter::exportToSQLite(
    const std::string& dbPath,
    const std::string& schemaPath,
    const std::vector<SpatialInstance>& instances,
    const ColocationResult& results
) {
    sqlite3* db = nullptr;

    int rc = sqlite3_open(dbPath.c_str(), &db);

    if (rc != SQLITE_OK) {
        std::string error = sqlite3_errmsg(db);
        sqlite3_close(db);
        throw std::runtime_error("Cannot open SQLite DB: " + error);
    }

    try {
        execSQL(db, "PRAGMA journal_mode = WAL;");
        execSQL(db, "PRAGMA synchronous = NORMAL;");
        execSQL(db, "PRAGMA temp_store = MEMORY;");

        execSQL(db, "BEGIN TRANSACTION;");

        const std::string schemaSql = readFileToString(schemaPath);
        execSQL(db, schemaSql);

        sqlite3_stmt* insertInstanceStmt = nullptr;
        sqlite3_prepare_v2(
            db,
            R"SQL(
                INSERT INTO instances (
                    instance_idx, feature, instance_id, x, y, checkin
                ) VALUES (?, ?, ?, ?, ?, ?);
            )SQL",
            -1,
            &insertInstanceStmt,
            nullptr
        );

        for (const auto& instance : instances) {
            sqlite3_bind_int(insertInstanceStmt, 1, instance.idx);
            sqlite3_bind_text(insertInstanceStmt, 2, instance.type.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(insertInstanceStmt, 3, instance.id.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(insertInstanceStmt, 4, instance.x);
            sqlite3_bind_double(insertInstanceStmt, 5, instance.y);
            sqlite3_bind_int(insertInstanceStmt, 6, instance.checkin);

            if (sqlite3_step(insertInstanceStmt) != SQLITE_DONE) {
                const char* err = sqlite3_errmsg(db);
                throw std::runtime_error(std::string("Failed to insert instance: ") + (err ? err : "unknown"));
            }

            sqlite3_reset(insertInstanceStmt);
        }

        sqlite3_finalize(insertInstanceStmt);

        auto ptrToIndex = buildPointerToIndexMap(instances);

        sqlite3_stmt* insertColocationStmt = nullptr;
        sqlite3_prepare_v2(
            db,
            R"SQL(
                INSERT INTO colocations (
                    colocation_id, size, feature_key, instance_count
                ) VALUES (?, ?, ?, ?);
            )SQL",
            -1,
            &insertColocationStmt,
            nullptr
        );

        sqlite3_stmt* insertFeatureStmt = nullptr;
        sqlite3_prepare_v2(
            db,
            R"SQL(
                INSERT INTO colocation_features (
                    colocation_id, feature, feature_order
                ) VALUES (?, ?, ?);
            )SQL",
            -1,
            &insertFeatureStmt,
            nullptr
        );

        sqlite3_stmt* insertMemberStmt = nullptr;
        sqlite3_prepare_v2(
            db,
            R"SQL(
                INSERT INTO colocation_members (
                    colocation_id, feature, instance_idx
                ) VALUES (?, ?, ?);
            )SQL",
            -1,
            &insertMemberStmt,
            nullptr
        );

        int colocationId = 1;

        for (const auto& [colocation, featureMap] : results) {
            std::string featureKey = makeFeatureKey(colocation);

            int instanceCount = 0;
            for (const auto& [feature, instanceSet] : featureMap) {
                instanceCount += static_cast<int>(instanceSet.size());
            }

            sqlite3_bind_int(insertColocationStmt, 1, colocationId);
            sqlite3_bind_int(insertColocationStmt, 2, static_cast<int>(colocation.size()));
            sqlite3_bind_text(insertColocationStmt, 3, featureKey.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(insertColocationStmt, 4, instanceCount);

            if (sqlite3_step(insertColocationStmt) != SQLITE_DONE) {
                throw std::runtime_error("Failed to insert colocation");
            }

            sqlite3_reset(insertColocationStmt);

            for (size_t order = 0; order < colocation.size(); ++order) {
                const auto& feature = colocation[order];

                sqlite3_bind_int(insertFeatureStmt, 1, colocationId);
                sqlite3_bind_text(insertFeatureStmt, 2, feature.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(insertFeatureStmt, 3, static_cast<int>(order));

                if (sqlite3_step(insertFeatureStmt) != SQLITE_DONE) {
                    throw std::runtime_error("Failed to insert colocation feature");
                }

                sqlite3_reset(insertFeatureStmt);
            }

            for (const auto& [feature, instanceSet] : featureMap) {
                for (const SpatialInstance* instancePtr : instanceSet) {
                    auto it = ptrToIndex.find(instancePtr);

                    if (it == ptrToIndex.end()) {
                        throw std::runtime_error("Instance pointer not found in ptrToIndex map");
                    }

                    int instanceIdx = it->second;

                    sqlite3_bind_int(insertMemberStmt, 1, colocationId);
                    sqlite3_bind_text(insertMemberStmt, 2, feature.c_str(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_int(insertMemberStmt, 3, instanceIdx);

                    if (sqlite3_step(insertMemberStmt) != SQLITE_DONE) {
                        throw std::runtime_error("Failed to insert colocation member");
                    }

                    sqlite3_reset(insertMemberStmt);
                }
            }

            colocationId++;
        }

        sqlite3_finalize(insertColocationStmt);
        sqlite3_finalize(insertFeatureStmt);
        sqlite3_finalize(insertMemberStmt);

        execSQL(db, "COMMIT;");

        sqlite3_close(db);

        std::cout << "Exported SQLite result to: " << dbPath << std::endl;
    } catch (...) {
        execSQL(db, "ROLLBACK;");
        sqlite3_close(db);
        throw;
    }
}

void ResultExporter::exportSummaryJson(
    const std::string& jsonPath,
    const ColocationResult& results
) {
    std::map<int, int> sizeDistribution;

    for (const auto& entry : results) {
        int size = static_cast<int>(entry.first.size());
        sizeDistribution[size]++;
    }

    std::ofstream out(jsonPath);
    if (!out.is_open()) {
        throw std::runtime_error("Cannot open summary JSON path: " + jsonPath);
    }

    out << "{\n";
    out << "  \"algorithm\": \"maximal_clique_hashmap\",\n";
    out << "  \"total_colocations\": " << results.size() << ",\n";
    out << "  \"size_distribution\": [\n";

    int i = 0;
    for (const auto& entry : sizeDistribution) {
        out << "    { \"size\": " << entry.first
            << ", \"count\": " << entry.second << " }";

        if (i + 1 < static_cast<int>(sizeDistribution.size())) {
            out << ",";
        }

        out << "\n";
        ++i;
    }

    out << "  ]\n";
    out << "}\n";

    out.close();

    std::cout << "Exported summary JSON to: " << jsonPath << std::endl;
}
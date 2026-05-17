#pragma once

#include <map>
#include <set>
#include <vector>
#include <string>
#include <unordered_map>
#include "types.h"

using ColocationResult = std::map<
    Colocation,
    std::unordered_map<FeatureType, std::set<const SpatialInstance*>>
>;

class ResultExporter {
public:
    static void exportToSQLite(
        const std::string& dbPath,
        const std::string& schemaPath,
        const std::vector<SpatialInstance>& instances,
        const ColocationResult& results
    );

    static void exportSummaryJson(
        const std::string& jsonPath,
        const ColocationResult& results
    );
};
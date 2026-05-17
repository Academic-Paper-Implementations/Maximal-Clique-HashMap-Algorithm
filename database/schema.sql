PRAGMA foreign_keys = ON;

DROP TABLE IF EXISTS colocation_members;
DROP TABLE IF EXISTS colocation_features;
DROP TABLE IF EXISTS colocations;
DROP TABLE IF EXISTS instances;

CREATE TABLE instances (
    instance_idx INTEGER PRIMARY KEY,
    feature TEXT NOT NULL,
    instance_id TEXT NOT NULL,
    x REAL NOT NULL,
    y REAL NOT NULL,
    checkin INTEGER
);

CREATE TABLE colocations (
    colocation_id INTEGER PRIMARY KEY,
    size INTEGER NOT NULL,
    feature_key TEXT NOT NULL,
    instance_count INTEGER NOT NULL
);

CREATE TABLE colocation_features (
    colocation_id INTEGER NOT NULL,
    feature TEXT NOT NULL,
    feature_order INTEGER NOT NULL,

    PRIMARY KEY (colocation_id, feature),

    FOREIGN KEY (colocation_id)
        REFERENCES colocations(colocation_id)
        ON DELETE CASCADE
);

CREATE TABLE colocation_members (
    colocation_id INTEGER NOT NULL,
    feature TEXT NOT NULL,
    instance_idx INTEGER NOT NULL,

    PRIMARY KEY (colocation_id, feature, instance_idx),

    FOREIGN KEY (colocation_id)
        REFERENCES colocations(colocation_id)
        ON DELETE CASCADE,

    FOREIGN KEY (instance_idx)
        REFERENCES instances(instance_idx)
        ON DELETE CASCADE
);

CREATE INDEX idx_instances_feature
ON instances(feature);

CREATE INDEX idx_instances_xy
ON instances(x, y);

CREATE INDEX idx_colocations_size
ON colocations(size);

CREATE INDEX idx_colocations_feature_key
ON colocations(feature_key);

CREATE INDEX idx_members_colocation
ON colocation_members(colocation_id);

CREATE INDEX idx_members_feature
ON colocation_members(feature);

CREATE INDEX idx_members_instance
ON colocation_members(instance_idx);


-- ============================================================================
-- Prevalent Co-location Patterns (PCP) Tables
-- ============================================================================

CREATE TABLE prevalent_colocations (
    prevalent_id INTEGER PRIMARY KEY,
    feature_key TEXT NOT NULL UNIQUE,
    size INTEGER NOT NULL,

    is_deduced INTEGER NOT NULL DEFAULT 0,
    min_prev REAL NOT NULL
);

CREATE TABLE prevalent_colocation_features (
    prevalent_id INTEGER NOT NULL,
    feature TEXT NOT NULL,
    feature_order INTEGER NOT NULL,

    PRIMARY KEY (prevalent_id, feature),

    FOREIGN KEY (prevalent_id)
        REFERENCES prevalent_colocations(prevalent_id)
        ON DELETE CASCADE
);

CREATE INDEX idx_pc_feature_key
ON prevalent_colocations(feature_key);

CREATE INDEX idx_pc_size
ON prevalent_colocations(size);

CREATE INDEX idx_pcf_prevalent_feature
ON prevalent_colocation_features(prevalent_id, feature);

CREATE INDEX idx_pcf_feature_prevalent
ON prevalent_colocation_features(feature, prevalent_id);


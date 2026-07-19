/*
 * Unit tests for the v6 output stage (intensity + per-axis gain/invert)
 * added to the shared MotionCueingConfig. Self-contained (no framework):
 * builds on any host, returns non-zero on first failure.
 */
#include "MotionCueing.h"
#include <cmath>
#include <cstdio>
#include <cstring>

static int g_fail = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { std::printf("FAIL: %s (line %d)\n", msg, __LINE__); g_fail++; } \
    else         { std::printf("ok:   %s\n", msg); } \
} while (0)

static bool near(float a, float b, float eps = 1e-5f) { return std::fabs(a - b) <= eps; }

int main() {
    /* 1. Schema version bumped for the new layout */
    CHECK(MCA_SCHEMA_VERSION == 6, "schema version is 6");

    /* 2. Defaults are unity/no-op after init */
    MotionCueingConfig cfg;
    initMotionCueing(&cfg, 50.0f);
    CHECK(near(cfg.intensity, 1.0f), "default intensity == 1.0");
    bool gains_unity = true, inverts_clear = true;
    for (int i = 0; i < 6; i++) {
        if (!near(cfg.axis_gain[i], 1.0f)) gains_unity = false;
        if (cfg.axis_invert[i] != 0)       inverts_clear = false;
    }
    CHECK(gains_unity, "default axis_gain all 1.0");
    CHECK(inverts_clear, "default axis_invert all 0");

    /* 3. Output stage with defaults is a pass-through */
    {
        float in[6]  = {1.0f, -2.0f, 3.5f, 0.1f, -0.2f, 0.05f};
        float out[6] = {0};
        mcaApplyOutputStage(&cfg, in, out);
        bool pass = true;
        for (int i = 0; i < 6; i++) if (!near(out[i], in[i])) pass = false;
        CHECK(pass, "default output stage is identity");
    }

    /* 4. Global intensity scales every axis */
    {
        mcaSetIntensity(&cfg, 0.5f);
        CHECK(near(mcaGetIntensity(&cfg), 0.5f), "intensity setter/getter");
        float in[6]  = {2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f};
        float out[6] = {0};
        mcaApplyOutputStage(&cfg, in, out);
        bool pass = true;
        for (int i = 0; i < 6; i++) if (!near(out[i], in[i] * 0.5f)) pass = false;
        CHECK(pass, "intensity 0.5 halves all axes");
    }

    /* 5. Per-axis gain and invert compose with intensity */
    {
        initMotionCueing(&cfg, 50.0f);
        mcaSetIntensity(&cfg, 2.0f);
        mcaSetAxisGain(&cfg, 0, 3.0f);
        mcaSetAxisInvert(&cfg, 1, 1);
        CHECK(near(mcaGetAxisGain(&cfg, 0), 3.0f), "axis gain setter/getter");
        CHECK(mcaGetAxisInvert(&cfg, 1) == 1, "axis invert setter/getter");
        float in[6]  = {1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        float out[6] = {0};
        mcaApplyOutputStage(&cfg, in, out);
        CHECK(near(out[0], 1.0f * 2.0f * 3.0f), "axis0: intensity*gain = 6");
        CHECK(near(out[1], -(1.0f * 2.0f)),     "axis1: inverted = -2");
        CHECK(near(out[2], 1.0f * 2.0f),        "axis2: intensity only = 2");
    }

    /* 6. Bounds-checked setters ignore out-of-range axes (no crash/write) */
    {
        mcaSetAxisGain(&cfg, -1, 9.0f);
        mcaSetAxisGain(&cfg, 6, 9.0f);
        mcaSetAxisInvert(&cfg, 99, 1);
        CHECK(near(mcaGetAxisGain(&cfg, 0), 3.0f), "out-of-range setters are no-ops");
        CHECK(near(mcaGetAxisGain(&cfg, 42), 1.0f), "out-of-range getter returns 1.0");
    }

    /* 7. Validation: defaults valid; NaN/absurd output-stage values rejected */
    {
        MotionCueingConfig v;
        initMotionCueing(&v, 50.0f);
        setMotionCueingPreset(&v, MCA_MODERATE);
        CHECK(mcaValidateConfig(&v) == 1, "default config validates");
        MotionCueingConfig bad = v;
        bad.intensity = std::nanf("");
        CHECK(mcaValidateConfig(&bad) == 0, "NaN intensity rejected");
        bad = v;
        bad.axis_gain[3] = 1e9f;
        CHECK(mcaValidateConfig(&bad) == 0, "absurd axis_gain rejected");
    }

    /* 8. Back-compat: a config from an older schema is rejected by validate,
     *    forcing a fall-back to defaults (protects against stale NVS blobs). */
    {
        MotionCueingConfig old;
        initMotionCueing(&old, 50.0f);
        old.schema_version = 5;   /* pretend it's the previous layout */
        CHECK(mcaValidateConfig(&old) == 0, "old schema version rejected");
    }

    if (g_fail == 0) { std::printf("\nALL PASSED\n"); return 0; }
    std::printf("\n%d FAILURE(S)\n", g_fail);
    return 1;
}

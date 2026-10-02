#pragma once
#include <array>

// 25 presets d'usine.
// rate (Hz), width 0..1, center -1..1, feedback -1..1, depth (notch) 0..1,
// stereo (dephasage LFO L/R en degres), delay (ms, 0 = off), dlyfb 0..1, mix 0..1
namespace EarthPresets
{
    struct Preset { const char* name; float rate, width, center, fb, depth, stereo, delay, dlyfb, mix; };

    inline const std::array<Preset, 25>& getFactory()
    {
        static const std::array<Preset, 25> p {{
            { "Funk Sweep DDL",      0.32f, 0.62f,  0.16f,  0.79f, 0.99f,   0.0f, 738.0f, 0.14f, 0.99f },
            { "Classic 12",          0.77f, 0.62f,  0.00f,  0.40f, 0.99f,  90.0f,   0.0f, 0.00f, 1.00f },
            { "Slow Bloom",          0.15f, 0.85f,  0.00f,  0.55f, 0.95f,  90.0f,   0.0f, 0.00f, 0.90f },
            { "Mu-Tron Style",       0.45f, 0.70f, -0.10f,  0.65f, 0.90f,   0.0f,   0.0f, 0.00f, 1.00f },
            { "Jet Sweep",           0.20f, 1.00f,  0.10f,  0.85f, 1.00f,   0.0f,   0.0f, 0.00f, 1.00f },
            { "Subtle Motion",       0.30f, 0.35f,  0.00f,  0.20f, 0.60f,  90.0f,   0.0f, 0.00f, 0.60f },
            { "Stereo Swirl",        0.55f, 0.75f,  0.00f,  0.45f, 0.95f, 180.0f,   0.0f, 0.00f, 1.00f },
            { "Quadrature Wide",     1.20f, 0.60f,  0.10f,  0.30f, 0.90f,  90.0f,   0.0f, 0.00f, 0.90f },
            { "Ping-Pong Phase",     0.40f, 0.60f,  0.10f,  0.50f, 0.95f,  90.0f, 500.0f, 0.35f, 1.00f },
            { "Dub Phaser",          0.18f, 0.70f, -0.20f,  0.60f, 0.95f,  60.0f, 750.0f, 0.50f, 0.90f },
            { "Rhodes Glow",         0.60f, 0.45f,  0.15f,  0.25f, 0.70f,  90.0f,   0.0f, 0.00f, 0.70f },
            { "Guitar Whoosh",       0.90f, 0.65f,  0.20f,  0.55f, 0.95f,  90.0f,   0.0f, 0.00f, 0.90f },
            { "Pad Drift",           0.10f, 0.90f, -0.10f,  0.30f, 0.85f, 120.0f,   0.0f, 0.00f, 0.80f },
            { "High Sweep",          0.50f, 0.50f,  0.60f,  0.50f, 0.95f,   0.0f,   0.0f, 0.00f, 1.00f },
            { "Low Sweep",           0.50f, 0.60f, -0.60f,  0.60f, 0.95f,   0.0f,   0.0f, 0.00f, 1.00f },
            { "Resonant Whistle",    0.35f, 0.40f,  0.30f,  0.92f, 1.00f,  90.0f,   0.0f, 0.00f, 1.00f },
            { "Negative Feedback",   0.45f, 0.70f,  0.00f, -0.70f, 0.99f,  90.0f,   0.0f, 0.00f, 1.00f },
            { "Vibe Throb",          4.50f, 0.35f,  0.00f,  0.20f, 0.80f,   0.0f,   0.0f, 0.00f, 0.85f },
            { "Rotary Phase",        6.50f, 0.25f,  0.10f,  0.30f, 0.70f,  90.0f,   0.0f, 0.00f, 0.80f },
            { "Echo Phaze",          0.25f, 0.65f,  0.00f,  0.45f, 0.90f,  90.0f, 600.0f, 0.45f, 0.95f },
            { "Fast Echo Phaze",     0.50f, 0.55f,  0.10f,  0.40f, 0.90f,  45.0f, 250.0f, 0.40f, 0.95f },
            { "Space Sweep",         0.12f, 1.00f,  0.05f,  0.70f, 1.00f, 150.0f, 900.0f, 0.50f, 1.00f },
            { "Ambient Halo",        0.08f, 0.80f, -0.05f,  0.50f, 0.90f, 180.0f, 850.0f, 0.60f, 0.90f },
            { "Pump Phaser",         2.20f, 0.90f,  0.20f,  0.50f, 1.00f,   0.0f,   0.0f, 0.00f, 1.00f },
            { "Total Meltdown",      0.30f, 1.00f,  0.00f,  0.90f, 1.00f,  90.0f, 700.0f, 0.60f, 1.00f },
        }};
        return p;
    }
}

#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Presets.h"

class EarthPhaserProcessor : public juce::AudioProcessor
{
public:
    EarthPhaserProcessor();
    ~EarthPhaserProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Earth Phaser"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ---- Presets ----
    juce::String getCurrentPresetName() const;
    void loadFactoryPreset (int index);
    void loadUserPreset (const juce::File& file);
    bool saveUserPreset (const juce::String& name);
    juce::Array<juce::File> getUserPresetFiles() const;
    static juce::File getUserPresetDir();

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> levelL { 0.0f }, levelR { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void setParamReal (const juce::String& id, float realValue);

    struct DelayLine
    {
        std::vector<float> buf;
        int w = 0, n = 0;
        void prepare (int size) { n = size; buf.assign ((size_t) n, 0.0f); w = 0; }
        void push (float x) { buf[(size_t) w] = x; if (++w >= n) w = 0; }
        float read (float d) const
        {
            d = juce::jlimit (1.0f, (float) (n - 2), d);
            float rp = (float) w - d;
            while (rp < 0.0f) rp += (float) n;
            int i0 = (int) rp;
            const float f = rp - (float) i0;
            if (i0 >= n) i0 -= n;
            int i1 = i0 + 1; if (i1 >= n) i1 = 0;
            return buf[(size_t) i0] + (buf[(size_t) i1] - buf[(size_t) i0]) * f;
        }
    };

    // Allpass du 1er ordre (un "pole" du phaser)
    struct AP1
    {
        float s = 0.0f;
        inline float process (float x, float a) { const float y = a * x + s; s = x - a * y; return y; }
    };

    static constexpr int kStages = 12;   // comme le Phaser-DDL du DP/4 : phaser stereo a 12 poles

    std::atomic<float> *pRate {}, *pWidth {}, *pCenter {}, *pFb {}, *pDepth {}, *pStereo {},
                       *pDelay {}, *pDlyFb {}, *pMix {}, *pPower {};

    juce::SmoothedValue<float> widthS, centerS, fbS, depthS, delayS, dlyFbS, mixS, powerS;

    double sr = 44100.0;
    double phase = 0.0;
    AP1 stage[2][kStages];
    DelayLine dly[2];
    float fbState[2] { 0.0f, 0.0f };
    float dcX[2] { 0.0f, 0.0f }, dcY[2] { 0.0f, 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EarthPhaserProcessor)
};

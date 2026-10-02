#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
    const char* kIds[9] = { "rate", "width", "center", "feedback", "depth", "stereo", "delay", "dlyfb", "drymix" };
}

juce::AudioProcessorValueTreeState::ParameterLayout EarthPhaserProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;
    // Rate : courbe exponentielle proche de l'echelle 0-99 du DP/4 (milieu ~ 0.77 Hz, max ~ 8.5 Hz)
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "rate", 1 },     "Rate",     NormalisableRange<float> (0.06f, 8.5f, 0.001f, 0.28f), 0.77f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "width", 1 },    "Width",    NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.62f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "center", 1 },   "Center",   NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "feedback", 1 }, "Feedback", NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.4f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "depth", 1 },    "Notch Depth", NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.95f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "stereo", 1 },   "L/R LFO",  NormalisableRange<float> (0.0f, 180.0f, 1.0f), 90.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "delay", 1 },    "Delay Time", NormalisableRange<float> (0.0f, 1000.0f, 1.0f), 375.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "dlyfb", 1 },    "Delay Feedback", NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "drymix", 1 },   "Dry/Wet",  NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.9f));
    l.add (std::make_unique<AudioParameterBool>  (ParameterID { "power", 1 },    "Power", true));
    return l;
}

EarthPhaserProcessor::EarthPhaserProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    pRate   = apvts.getRawParameterValue ("rate");
    pWidth  = apvts.getRawParameterValue ("width");
    pCenter = apvts.getRawParameterValue ("center");
    pFb     = apvts.getRawParameterValue ("feedback");
    pDepth  = apvts.getRawParameterValue ("depth");
    pStereo = apvts.getRawParameterValue ("stereo");
    pDelay  = apvts.getRawParameterValue ("delay");
    pDlyFb  = apvts.getRawParameterValue ("dlyfb");
    pMix    = apvts.getRawParameterValue ("drymix");
    pPower  = apvts.getRawParameterValue ("power");
    apvts.state.setProperty ("presetName", "Default", nullptr);
}

bool EarthPhaserProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && l.getMainInputChannelSet()  == l.getMainOutputChannelSet();
}

void EarthPhaserProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    for (int c = 0; c < 2; ++c)
    {
        dly[c].prepare ((int) (1.1 * sr) + 16);
        for (auto& s : stage[c]) s.s = 0.0f;
        fbState[c] = dcX[c] = dcY[c] = 0.0f;
    }
    phase = 0.0;

    widthS.reset (sr, 0.05); centerS.reset (sr, 0.05); fbS.reset (sr, 0.05); depthS.reset (sr, 0.05);
    dlyFbS.reset (sr, 0.05); mixS.reset (sr, 0.05); powerS.reset (sr, 0.03);
    delayS.reset (sr, 0.25);

    widthS.setCurrentAndTargetValue (pWidth->load());
    centerS.setCurrentAndTargetValue (pCenter->load());
    fbS.setCurrentAndTargetValue (pFb->load());
    depthS.setCurrentAndTargetValue (pDepth->load());
    delayS.setCurrentAndTargetValue (pDelay->load());
    dlyFbS.setCurrentAndTargetValue (pDlyFb->load());
    mixS.setCurrentAndTargetValue (pMix->load());
    powerS.setCurrentAndTargetValue (pPower->load() > 0.5f ? 1.0f : 0.0f);
}

void EarthPhaserProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    if (buffer.getNumChannels() < 2) return;

    widthS.setTargetValue (pWidth->load());
    centerS.setTargetValue (pCenter->load());
    fbS.setTargetValue (pFb->load());
    depthS.setTargetValue (pDepth->load());
    delayS.setTargetValue (pDelay->load());
    dlyFbS.setTargetValue (pDlyFb->load());
    mixS.setTargetValue (pMix->load());
    powerS.setTargetValue (pPower->load() > 0.5f ? 1.0f : 0.0f);

    const double inc = (double) pRate->load() / sr;
    const double stereoFrac = (double) pStereo->load() / 360.0;
    const float msToSmp = (float) (sr / 1000.0);
    const double twoPi = juce::MathConstants<double>::twoPi;
    const float piF = juce::MathConstants<float>::pi;
    const float maxHz = 0.45f * (float) sr;

    float* d0 = buffer.getWritePointer (0);
    float* d1 = buffer.getWritePointer (1);
    float pkL = 0.0f, pkR = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        const float wdt = widthS.getNextValue();
        const float cen = centerS.getNextValue();
        const float fbv = fbS.getNextValue();
        const float dep = depthS.getNextValue();
        const float dms = delayS.getNextValue();
        const float dfb = dlyFbS.getNextValue();
        const float mx  = mixS.getNextValue();
        const float pw  = powerS.getNextValue();

        phase += inc; phase -= std::floor (phase);

        // Frequence centrale (echelle log) : 650 Hz * 4^center, balayage +/- 2.5 octaves * width
        const float f0 = 650.0f * std::pow (4.0f, cen);

        // Delay ping-pong dans la boucle : L = t, R = t/2 (rapport 2:1 comme dans les presets DP/4)
        float dOut[2] = { 0.0f, 0.0f };
        if (dms > 2.0f)
        {
            dOut[0] = dly[0].read (dms * msToSmp);
            dOut[1] = dly[1].read (dms * 0.5f * msToSmp);
        }

        const float fbEff  = fbv * 0.95f;
        const float dfbEff = dfb * 0.9f * (1.0f - 0.5f * std::abs (fbv));   // evite l'emballement de la boucle

        const float in[2] = { d0[i], d1[i] };
        float out[2];

        for (int c = 0; c < 2; ++c)
        {
            const double lp = phase + (c == 0 ? 0.0 : stereoFrac);
            const float lfo = (float) std::sin (twoPi * lp);
            const float fc = juce::jlimit (30.0f, maxHz, f0 * std::exp2 (wdt * 2.5f * lfo));
            const float t = std::tan (piF * fc / (float) sr);
            const float a = (t - 1.0f) / (t + 1.0f);

            // Feedback du phaser + retour du delay en croix (ping-pong) dans l'entree du phaser
            const float fbTerm = std::tanh (fbEff * fbState[c] + dfbEff * dOut[1 - c]);
            const float x = in[c] + fbTerm;

            float y = x;
            for (int k = 0; k < kStages; ++k) y = stage[c][k].process (y, a);
            fbState[c] = y;

            // Profondeur des notchs : 0 = signal sec, 1 = mix 50/50 (notchs complets)
            const float wet = x * (1.0f - 0.5f * dep) + y * 0.5f * dep;
            dly[c].push (wet);

            // Filtre passe-haut tres bas (bloque le DC cree par le feedback)
            const float dc = wet - dcX[c] + 0.9995f * dcY[c];
            dcX[c] = wet; dcY[c] = dc;
            out[c] = dc;
        }

        const float g = mx * pw;
        const float o0 = in[0] + (out[0] - in[0]) * g;
        const float o1 = in[1] + (out[1] - in[1]) * g;
        d0[i] = o0; d1[i] = o1;
        pkL = juce::jmax (pkL, std::abs (o0));
        pkR = juce::jmax (pkR, std::abs (o1));
    }

    levelL.store (pkL);
    levelR.store (pkR);
}

juce::AudioProcessorEditor* EarthPhaserProcessor::createEditor() { return new EarthPhaserEditor (*this); }

void EarthPhaserProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, destData);
}

void EarthPhaserProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// ------------------------------------------------------------------ Presets
juce::String EarthPhaserProcessor::getCurrentPresetName() const
{
    return apvts.state.getProperty ("presetName", "Default").toString();
}

void EarthPhaserProcessor::setParamReal (const juce::String& id, float v)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (v));
}

void EarthPhaserProcessor::loadFactoryPreset (int index)
{
    const auto& f = EarthPresets::getFactory();
    if (index < 0 || index >= (int) f.size()) return;
    const auto& p = f[(size_t) index];
    setParamReal ("rate", p.rate);
    setParamReal ("width", p.width);
    setParamReal ("center", p.center);
    setParamReal ("feedback", p.fb);
    setParamReal ("depth", p.depth);
    setParamReal ("stereo", p.stereo);
    setParamReal ("delay", p.delay);
    setParamReal ("dlyfb", p.dlyfb);
    setParamReal ("drymix", p.mix);
    apvts.state.setProperty ("presetName", p.name, nullptr);
}

juce::File EarthPhaserProcessor::getUserPresetDir()
{
    auto d = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
   #if JUCE_MAC
    d = d.getChildFile ("Application Support");
   #endif
    return d.getChildFile ("NACHE").getChildFile ("Earth Phaser").getChildFile ("Presets");
}

juce::Array<juce::File> EarthPhaserProcessor::getUserPresetFiles() const
{
    auto files = getUserPresetDir().findChildFiles (juce::File::findFiles, false, "*.xml");
    std::sort (files.begin(), files.end(), [] (const juce::File& a, const juce::File& b)
               { return a.getFileName().compareIgnoreCase (b.getFileName()) < 0; });
    return files;
}

bool EarthPhaserProcessor::saveUserPreset (const juce::String& name)
{
    const auto clean = juce::File::createLegalFileName (name.trim());
    if (clean.isEmpty()) return false;
    auto dir = getUserPresetDir();
    if (! dir.createDirectory()) return false;

    juce::XmlElement x ("EarthPhaserPreset");
    x.setAttribute ("name", clean);
    for (auto* id : kIds) x.setAttribute (id, (double) apvts.getRawParameterValue (id)->load());

    const bool ok = x.writeTo (dir.getChildFile (clean + ".xml"));
    if (ok) apvts.state.setProperty ("presetName", clean, nullptr);
    return ok;
}

void EarthPhaserProcessor::loadUserPreset (const juce::File& file)
{
    auto xml = juce::parseXML (file);
    if (xml == nullptr || ! xml->hasTagName ("EarthPhaserPreset")) return;
    for (auto* id : kIds)
        if (xml->hasAttribute (id)) setParamReal (id, (float) xml->getDoubleAttribute (id));
    apvts.state.setProperty ("presetName", file.getFileNameWithoutExtension(), nullptr);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new EarthPhaserProcessor(); }

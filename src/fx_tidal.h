// πMorpH 0.2.0 — Tidal Verb (Fullban, MIT) intégrée comme moteur de reverb.
// Le DSP est repris tel quel (src/fx/tidal/TidalVerb.inc, commit e83bf20 de github.com/fullbann/tidal-verb) ;
// cette couche remplace le framework mpc_plugin.h par le strict nécessaire (paramètres en valeurs réelles,
// pas de VST, pas de programmes). Voir THIRD-PARTY.md.
#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include "fx/tidal/dsp/math.h"
#include "fx/tidal/dsp/delay_line.h"
#include "fx/tidal/dsp/onepole.h"
#include "fx/tidal/dsp/smoother.h"

#ifndef MPC_FOURCC
#define MPC_FOURCC(a, b, c, d) ((uint32_t)(((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(c) << 8) | (uint32_t)(d)))
#endif

namespace mpc {
template <typename T> inline T clamp(T v, T lo, T hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline void log(const char*, ...) {}
struct Param { float min = 0, max = 1, def = 0; bool automatable = true; };
inline Param floatParam(const char*, const char*, float min, float max, float def, const char* = "", int = 2) { return Param{min, max, def}; }
inline Param boolParam(const char*, const char*, bool def) { return Param{0, 1, def ? 1.f : 0.f}; }
inline Param choiceParam(const char*, const char*, std::vector<std::string> c, int def = 0) { return Param{0, (float)c.size() - 1, (float)def}; }
struct PluginInfo { const char* name = ""; const char* vendor = ""; uint32_t uniqueId = 0; int version = 0; bool isInstrument = false; int numInputs = 2, numOutputs = 2, tailSamples = 0; };
struct ProcessContext { const float* const* inputs = nullptr; float* const* outputs = nullptr; int numInputs = 2, numOutputs = 2, numFrames = 0; double sampleRate = 44100.0; };
class Plugin {
public:
    explicit Plugin(const PluginInfo&) {}
    virtual ~Plugin() {}
    virtual void prepare(double, int) {}
    virtual void reset() {}
    virtual void process(ProcessContext&) = 0;
    virtual bool formatValue(int, float, std::string&) { return false; }
    void addParam(const Param& p) { params_.push_back(p); values_.push_back(p.def); }
    void addProgram(const char*, std::vector<float>) {}
    int numParams() const { return (int)params_.size(); }
    float get(int i) const { return values_[(size_t)i]; }
    int getInt(int i) const { return (int)std::lround(values_[(size_t)i]); }
    bool getBool(int i) const { return values_[(size_t)i] >= 0.5f; }
    void set(int i, float v) { const Param& p = params_[(size_t)i]; values_[(size_t)i] = clamp(v, p.min, p.max); }
    void setNormalised(int, float) {}
    double sampleRate() const { return sampleRate_; }
    void setSampleRate(double sr) { sampleRate_ = sr; }
private:
    std::vector<Param> params_;
    std::vector<float> values_;
    double sampleRate_ = 44100.0;
};
} // namespace mpc

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsubobject-linkage"   // classes internes de TidalVerb.inc (inclus une seule fois)
namespace tidalfx {
#include "fx/tidal/TidalVerb.inc"
// Indices publics (ordre du DSP d'origine).
enum { Level = kLevel, Type = kType, Decay = kDecay, Liveliness = kLiveliness, Tails = kTails, Swirl = kSwirl,
       SwirlRate = kSwirlRate, Warmth = kWarmth, Air = kAir, TypeCount = kNumTypes };
} // namespace tidalfx
#pragma GCC diagnostic pop

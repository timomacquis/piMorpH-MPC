// πMorpH 0.2.0 — delay à bande « DELAY A » de Dub Station (nachtaktiv303, MIT), commit 3752cc8 de
// github.com/nachtaktiv303/mpc-vst-dub-station, lui-même bâti sur le DSP CHOMPI TAPE 2.0 (MIT) et DaisySP (MIT).
// Repris de src/engine.cpp de Dub Station : couleur bande (DRIVE + WARBLE) sur l'entrée du delay, ligne interpolée
// ping-pong, filtre DJ résonant compensé et passe-bas dans la boucle, réinjection jusqu'à 120 % bornée par tanh,
// largeur stéréo + Haas. Seule la ligne A est reprise (pas de ligne B, flanger, phaser, bruit, ducking, Galactic).
// Différence voulue : le son direct de πMorpH n'est pas coloré ; seuls les échos sont ajoutés (LEVEL 0 = son inchangé).
// Réglages exposés : TIME, SYNC, FEEDBACK, FILTER, LEVEL. Fixés aux valeurs par défaut de Dub Station :
// RESO 20 %, DRIVE 20 %, WARBLE 15 %, WIDTH 100 %, SAT 0.
#pragma once
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "fx/daisysp/dsp.h"
#include "fx/daisysp/delayline.h"
#include "fx/chompi/InterpolatedDelayLine.h"
#include "fx/chompi/Warble.h"
#include "fx/chompi/DJFilter.h"

namespace dubfx {
constexpr int kDivCount = 7;
static const float kDivBeats[kDivCount] = {0.25f, 0.5f, 0.75f, 1.f, 1.5f, 2.f, 4.f};
static const char* const kDivLabels[kDivCount] = {"1/16", "1/8", "1/8.", "1/4", "1/4.", "1/2", "1"};
// Sync : la plage brute du bouton TIME (10..2000 ms) est répartie sur les 7 divisions (comme Dub Station).
inline int divIndexFor(float timeMs) { int d = (int)((timeMs - 10.f) / 1990.f * 7.f); return d < 0 ? 0 : d > 6 ? 6 : d; }

inline float s162f(int32_t x) { return (float)x * 3.0518509475997192e-05f; }
inline int32_t f2s16(float x) { x = x <= -0.999985f ? -0.999985f : x; x = x >= 0.999985f ? 0.999985f : x; return (int32_t)(x * 32767.0f); }
inline void widthStereo(float& l, float& r, float w) { const float mid = 0.5f * (l + r), side = 0.5f * (l - r) * w; l = mid + side; r = mid - side; }
constexpr int kHaasMax = 160;
inline float haasTap(float* buf, int& idx, float x, int off) {
    buf[idx] = x; off = off < 0 ? 0 : off > kHaasMax - 1 ? kHaasMax - 1 : off;
    int r = idx - off; if (r < 0) r += kHaasMax; const float out = buf[r]; if (++idx >= kHaasMax) idx = 0; return out;
}
struct DcBlock { float in_ = 0, out_ = 0; float process(float in) { const float o = in - in_ + 0.999f * out_; out_ = o; in_ = in; return o; } };

struct DubDelay {
    // Réglages (unités de Dub Station).
    float time = 375.f, feedback = 45.f, filter = 0.f, level = 0.f; bool sync = true; float bpm = 120.f;
    static constexpr float kReso = 20.f, kDrive = 20.f, kWarble = 15.f, kWidth = 100.f;

    DubDelay() { mem_ = (chompi::InterpolatedDelayLine::AudioSample*)calloc(kMax, sizeof *mem_); }
    ~DubDelay() { free(mem_); }
    DubDelay(const DubDelay&) = delete; DubDelay& operator=(const DubDelay&) = delete;

    void prepare(double sr) {
        sr_ = (float)sr; maxFrames_ = (float)(kMax - 2); minFrames_ = 413.f * sr_ / 44100.f;
        if (mem_) del_.Init(mem_, kMax);
        warble_.Init(sr_); filt_.Init(sr_); memset(haas_, 0, sizeof haas_); haasI_ = 0;
        dcL_ = dcR_ = DcBlock{}; fbL_ = fbR_ = 0; wasOn_ = false;
        update(); snap();
    }
    float delayMs() const { return sync ? kDivBeats[divIndexFor(time)] * 60000.f / (bpm > 1.f ? bpm : 120.f) : time; }
    void update() {
        timeT_ = daisysp::fclamp(delayMs() * 0.001f * sr_, minFrames_, maxFrames_);
        regenT_ = feedback * .01f; sendT_ = level * .01f; widthT_ = kWidth * .01f;
        satT_ = logf(1.7f * (kDrive * .01f) + 1.f) * 13.f + 1.f;
        warble_.SetFreq(kWarble * .01f);
        filtCtrl_ = 0.5f + (filter * .01f) * 0.35f;
        filtRes_ = kReso * .01f * 0.72f; const float rx = filtRes_ - 0.4f; resComp_ = rx > 0.f ? 1.f / (1.f + rx) : 1.f;
    }
    void snap() { time_ = timeT_; regen_ = regenT_; send_ = sendT_; width_ = widthT_; sat_ = satT_; }
    bool active() const { return mem_ && (sendT_ > 0.f || send_ > 0.0005f); }

    // En place : ajoute les échos au signal. Le signal direct reste intact.
    void process(float* l, float* r, int n) {
        if (!active()) { wasOn_ = false; return; }
        if (!wasOn_) { del_.FullClear(); fbL_ = fbR_ = 0; wasOn_ = true; }   // pas d'anciens échos au retour
        filt_.SetControl(filtCtrl_); filt_.SetRes(filtRes_);
        const float kFbLp = 0.66f, kCeil = 0.75f;
        for (int i = 0; i < n; i++) {
            daisysp::fonepole(sat_, satT_, .001f); daisysp::fonepole(regen_, regenT_, .001f);
            daisysp::fonepole(width_, widthT_, .001f); daisysp::fonepole(time_, timeT_, .001f);
            daisysp::fonepole(send_, sendT_, .001f);
            float wl = daisysp::SoftClip(sat_ * l[i]), wr = daisysp::SoftClip(sat_ * r[i]);
            const float g = 1.f - daisysp::SoftClip(.4f * (sat_ - 1.f)) * .7f; wl *= g; wr *= g;
            warble_.Process(wl, wr, &wl, &wr);
            del_.SetDelay(time_);
            const auto rd = del_.Read();
            float fl = s162f(rd.l), fr = s162f(rd.r);
            filt_.Process(fl, fr, &fl, &fr);
            fl = dcL_.process(fl); fr = dcR_.process(fr);
            fbL_ += kFbLp * (fl - fbL_); fbR_ += kFbLp * (fr - fbR_); fl = fbL_; fr = fbR_;
            const float inL = kCeil * tanhf((wl + fr * regen_ * resComp_) / kCeil);
            const float inR = kCeil * tanhf((wr + fl * regen_ * resComp_) / kCeil);
            del_.Write({int16_t(f2s16(inL)), int16_t(f2s16(inR))});
            float el = fl, er = fr; widthStereo(el, er, width_);
            er = haasTap(haas_, haasI_, er, (int)(width_ * 120.f));
            l[i] += el * send_; r[i] += er * send_;
        }
    }
private:
    static constexpr size_t kMax = 88435;   // ~2 s à 44,1 kHz, 16 bits stéréo (~354 Ko)
    chompi::InterpolatedDelayLine::AudioSample* mem_ = nullptr;
    chompi::InterpolatedDelayLine del_;
    chompi::Warble warble_;
    DjFilter filt_;
    DcBlock dcL_, dcR_;
    float haas_[kHaasMax] = {}; int haasI_ = 0;
    float sr_ = 44100.f, maxFrames_ = kMax - 2, minFrames_ = 413.f;
    float time_ = 0, timeT_ = 0, regen_ = 0, regenT_ = 0, send_ = 0, sendT_ = 0, width_ = 1, widthT_ = 1, sat_ = 1, satT_ = 1;
    float filtCtrl_ = .5f, filtRes_ = 0, resComp_ = 1, fbL_ = 0, fbR_ = 0;
    bool wasOn_ = false;
};
} // namespace dubfx

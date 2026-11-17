// πMorpH 0.2.0 (candidat 6) — effets en série sur la sortie : synthé -> DELAY (Dub Station) -> REVERB (Tidal Verb).
// La reverb reçoit le son direct et les échos. LEVEL 0 sur les deux = son de πMorpH inchangé, aucun calcul.
#pragma once
#include "fx_dub.h"
#include "fx_tidal.h"

struct FxChain {
    dubfx::DubDelay delay;
    tidalfx::TidalVerb reverb;
    // Réglages exposés de la reverb ; les autres sont fixés (TAILS ON, SWIRL 60, SWIRL RATE 40, AIR 45 : défauts de Tidal Verb).
    float rvType = 1, rvDecay = 55, rvLive = 60, rvWarmth = 35, rvLevel = 0;

    void prepare(double sr) {
        delay.prepare(sr);
        reverb.setSampleRate(sr);
        reverb.set(tidalfx::Tails, 1); reverb.set(tidalfx::Swirl, 60); reverb.set(tidalfx::SwirlRate, 40); reverb.set(tidalfx::Air, 45);
        apply();
        reverb.prepare(sr, 128); reverb.reset();
        sr_ = sr; rvOn_ = false;
    }
    void apply() {
        delay.update();
        reverb.set(tidalfx::Type, rvType); reverb.set(tidalfx::Decay, rvDecay); reverb.set(tidalfx::Liveliness, rvLive);
        reverb.set(tidalfx::Warmth, rvWarmth); reverb.set(tidalfx::Level, rvLevel);
    }
    // buffer : stéréo entrelacé, n <= 128 trames.
    void process(float* buffer, int n) {
        const bool dl = delay.active();
        // Reverb : active tant que LEVEL > 0, puis encore 100 ms (le LEVEL lissé de Tidal Verb redescend à 0).
        if (rvLevel > 0.f) rvIdle_ = 0; else if (rvOn_) rvIdle_ += n;
        if (rvOn_ && rvLevel <= 0.f && rvIdle_ > (long)(0.1 * sr_)) rvOn_ = false;
        if (!rvOn_ && rvLevel > 0.f) { reverb.reset(); rvOn_ = true; }   // reprise sans ancienne queue
        if (!dl && !rvOn_) return;
        float l[128], r[128];
        for (int i = 0; i < n; i++) { l[i] = buffer[2 * i]; r[i] = buffer[2 * i + 1]; }
        if (dl) delay.process(l, r, n);
        if (rvOn_) {
            const float* in[2] = {l, r}; float* out[2] = {l, r};
            mpc::ProcessContext ctx; ctx.inputs = in; ctx.outputs = out; ctx.numFrames = n; ctx.sampleRate = sr_;
            reverb.process(ctx);
        }
        for (int i = 0; i < n; i++) { buffer[2 * i] = l[i]; buffer[2 * i + 1] = r[i]; }
    }
private:
    double sr_ = 44100.0; bool rvOn_ = false; long rvIdle_ = 0;
};

#!/usr/bin/env python3
"""Procedural audio for Eight Wings: synthesised SFX and algorithmically composed chiptune loops.

Pure standard library. Output: assets/audio/sfx_<name>.wav and mus_<name>.wav, 22050 Hz, mono, 16-bit.
Deterministic (seeded). Names must match the Sfx/Music enums in src/game/types.h (see src/platform/audio.c).
"""
import math
import os
import random
import struct
import wave

SR = 22050
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "assets", "audio")


def midi(n):
    return 440.0 * 2 ** ((n - 69) / 12.0)


# ---------------------------------------------------------------- oscillators
def osc(kind, phase, duty=0.5):
    p = phase % 1.0
    if kind == "sq":
        return 1.0 if p < duty else -1.0
    if kind == "tri":
        return 4 * abs(p - 0.5) - 1
    if kind == "saw":
        return 2 * p - 1
    return 0.0


def tone(kind, f0, f1, dur, vol=0.5, decay=4.0, duty=0.5, vib=0.0, attack=0.002):
    """Frequency sweep f0->f1 over dur seconds with exponential decay."""
    n = int(dur * SR)
    out = [0.0] * n
    ph = 0.0
    for i in range(n):
        t = i / SR
        u = i / max(1, n - 1)
        f = f0 + (f1 - f0) * u
        if vib:
            f *= 1 + vib * math.sin(2 * math.pi * 6 * t)
        ph += f / SR
        env = math.exp(-decay * u) * min(1.0, t / attack if attack else 1.0)
        out[i] = osc(kind, ph, duty) * vol * env
    return out


def noise(dur, vol=0.5, decay=4.0, cutoff=1.0, seed=1, sweep=0.0):
    """White noise through a one-pole lowpass (cutoff 0..1, sweep changes it over time)."""
    rnd = random.Random(seed)
    n = int(dur * SR)
    out = [0.0] * n
    y = 0.0
    for i in range(n):
        u = i / max(1, n - 1)
        c = max(0.01, min(1.0, cutoff + sweep * u))
        y += c * (rnd.uniform(-1, 1) - y)
        out[i] = y * vol * math.exp(-decay * u)
    return out


def mix(*tracks, offsets=None):
    n = max(len(t) + (offsets[k] if offsets else 0) for k, t in enumerate(tracks))
    out = [0.0] * n
    for k, t in enumerate(tracks):
        o = offsets[k] if offsets else 0
        for i, v in enumerate(t):
            out[o + i] += v
    return out


def seq(*parts):
    out = []
    for p in parts:
        out += p
    return out


def write_wav(name, samples, peak=0.9):
    m = max(1e-9, max(abs(s) for s in samples))
    g = min(1.0, peak / m) if m > peak else 1.0
    data = b"".join(struct.pack("<h", int(max(-1, min(1, s * g)) * 32767)) for s in samples)
    os.makedirs(OUT, exist_ok=True)
    with wave.open(os.path.join(OUT, name + ".wav"), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes(data)


# ---------------------------------------------------------------- sound effects
def sfx():
    S = {}
    S["shot"] = tone("sq", 1500, 500, 0.07, 0.30, decay=5, duty=0.25)
    S["shot_heavy"] = mix(tone("sq", 900, 220, 0.12, 0.34, decay=4, duty=0.5), tone("saw", 500, 120, 0.12, 0.2, decay=4))
    S["hit"] = mix(tone("sq", 300, 180, 0.04, 0.25, decay=3), noise(0.04, 0.2, 3, 0.6))
    S["enemy_die"] = mix(noise(0.22, 0.5, 4, 0.5, seed=2, sweep=-0.35), tone("sq", 700, 90, 0.2, 0.28, decay=4))
    S["explode_big"] = mix(noise(0.7, 0.8, 3, 0.35, seed=3, sweep=-0.3), tone("tri", 140, 35, 0.7, 0.6, decay=3))
    S["ground_die"] = mix(noise(0.35, 0.65, 4, 0.25, seed=4), tone("tri", 110, 40, 0.3, 0.5, decay=4))
    S["pod_appear"] = seq(tone("sq", 660, 660, 0.06, 0.28, decay=1.5), tone("sq", 880, 880, 0.06, 0.28, decay=1.5), tone("sq", 1320, 1320, 0.1, 0.28, decay=3))
    S["dock"] = seq(*[tone("sq", midi(n), midi(n), 0.07, 0.3, decay=1.2, duty=0.25) for n in (72, 76, 79, 84)], tone("sq", midi(88), midi(88), 0.2, 0.3, decay=4, duty=0.25))
    S["component_lost"] = mix(noise(0.3, 0.55, 3, 0.4, seed=5), tone("saw", 600, 80, 0.3, 0.35, decay=3))
    S["formation_start"] = mix(tone("saw", 150, 1200, 0.7, 0.3, decay=0.6, vib=0.02), tone("sq", 300, 2400, 0.7, 0.18, decay=0.6, duty=0.125), noise(0.6, 0.15, 2, 0.8, seed=6, sweep=-0.5))
    S["formation_end"] = mix(tone("saw", 1200, 150, 0.5, 0.3, decay=1.2), tone("sq", 2000, 300, 0.5, 0.15, decay=1.5, duty=0.125))
    S["player_die"] = mix(noise(1.0, 0.75, 2.5, 0.5, seed=7, sweep=-0.45), tone("saw", 700, 40, 1.0, 0.4, decay=2.2, vib=0.04))
    S["menu_move"] = tone("sq", 880, 880, 0.04, 0.22, decay=2, duty=0.5)
    S["menu_select"] = seq(tone("sq", 660, 660, 0.05, 0.25, decay=1), tone("sq", 990, 990, 0.09, 0.25, decay=3))
    S["boss_warning"] = seq(*[tone("sq", f, f, 0.14, 0.3, decay=1.0, duty=0.5) for f in (440, 330, 440, 330, 440, 330)])
    S["boss_hit"] = mix(tone("sq", 220, 120, 0.05, 0.3, decay=3), noise(0.05, 0.3, 3, 0.4, seed=8))
    S["boss_die"] = mix(noise(1.8, 0.9, 1.6, 0.4, seed=9, sweep=-0.35), tone("tri", 180, 25, 1.8, 0.8, decay=1.6), tone("saw", 400, 30, 1.5, 0.3, decay=2, vib=0.05))
    S["one_up"] = seq(*[tone("sq", midi(n), midi(n), 0.08, 0.28, decay=1.0, duty=0.25) for n in (67, 71, 74, 79, 83, 86)], tone("sq", midi(91), midi(91), 0.25, 0.28, decay=3, duty=0.25))
    S["medal"] = seq(tone("sq", 1600, 1600, 0.04, 0.25, decay=1), tone("sq", 2400, 2400, 0.1, 0.25, decay=4))
    S["enemy_shot"] = tone("sq", 600, 300, 0.09, 0.18, decay=3, duty=0.125)
    S["beam"] = mix(tone("saw", 90, 70, 0.8, 0.35, decay=0.8, vib=0.03), noise(0.8, 0.25, 1.0, 0.3, seed=10))
    S["stage_clear"] = seq(*[tone("sq", midi(n), midi(n), 0.11, 0.28, decay=1.0, duty=0.25) for n in (60, 64, 67, 72, 67, 72, 76)], tone("sq", midi(79), midi(79), 0.5, 0.28, decay=2.5, duty=0.25, vib=0.01))
    S["dino_die"] = mix(tone("saw", 260, 55, 0.7, 0.45, decay=2.2, vib=0.06), tone("sq", 180, 60, 0.6, 0.2, decay=2.5, duty=0.25, vib=0.08), noise(0.5, 0.3, 3, 0.3, seed=13))
    S["dino_spit"] = mix(tone("saw", 400, 120, 0.25, 0.3, decay=3), noise(0.25, 0.3, 4, 0.7, seed=14, sweep=-0.4))
    S["pause"] = seq(tone("sq", 520, 520, 0.06, 0.22, decay=1), tone("sq", 390, 390, 0.08, 0.22, decay=3))
    for k, v in S.items():
        write_wav("sfx_" + k, v)


# ---------------------------------------------------------------- music (algorithmic composition)
SCALES = {"minor": [0, 2, 3, 5, 7, 8, 10], "phrygian": [0, 1, 3, 5, 7, 8, 10], "dorian": [0, 2, 3, 5, 7, 9, 10], "harmonic": [0, 2, 3, 5, 7, 8, 11]}


def degree(root, scale, d):
    sc = SCALES[scale]
    return root + sc[d % 7] + 12 * (d // 7)


def compose(name, root, scale, bpm, prog, seed, bars=16, lead_density=0.6, arp=True, drums="rock", lead_wave="sq", intensity=1.0):
    rnd = random.Random(seed)
    step = 60.0 / bpm / 2  # eighth note
    steps_per_bar = 8
    total = bars * steps_per_bar
    n = int(total * step * SR)
    buf = [0.0] * n

    def put(samples, at_step, gain=1.0):
        o = int(at_step * step * SR)
        for i, v in enumerate(samples):
            if o + i < n:
                buf[o + i] += v * gain

    # motif: 8 eighth notes of scale degrees, then variations
    motif = [rnd.choice([0, 2, 4, 4, 5, 7, 2, 1]) if rnd.random() < lead_density else None for _ in range(8)]
    motif2 = [(m + rnd.choice([0, 1, -1, 2]) if m is not None else None) for m in motif]
    for bar in range(bars):
        d = prog[bar % len(prog)]
        chord = [degree(root, scale, d + k) for k in (0, 2, 4)]
        base = bar * steps_per_bar
        # bass: driving eighths on the triangle, octave jumps
        for s in range(8):
            note = chord[0] - 24 + (12 if s % 4 == 3 else 0)
            put(tone("tri", midi(note), midi(note), step * 0.9, 0.42, decay=1.8), base + s)
        # arpeggio on a thin pulse
        if arp:
            for s in range(8):
                note = chord[(s * 2 + bar // 4) % 3] + 12
                put(tone("sq", midi(note), midi(note), step * 0.8, 0.10 * intensity, decay=3.0, duty=0.125), base + s)
        # lead: motif A in bars 0-1 of each 4, B in 2, silence-ish in 3 for breath
        phase = bar % 4
        if bar >= 2:
            pat = motif if phase in (0, 1) else motif2 if phase == 2 else [motif[0], None, motif[2], None, motif2[4], None, motif[6], None]
            for s, m in enumerate(pat):
                if m is None:
                    continue
                note = degree(root, scale, d + m) + 12
                put(tone(lead_wave, midi(note), midi(note), step * 1.7, 0.22 * intensity, decay=2.0, duty=0.25, vib=0.004, attack=0.004), base + s)
        # drums
        for s in range(8):
            if drums == "rock":
                if s % 4 == 0:
                    put(mix(tone("tri", 160, 45, 0.12, 0.7, decay=5), noise(0.03, 0.2, 5, 0.3, seed=11)), base + s)
                if s % 4 == 2:
                    put(noise(0.14, 0.38, 6, 0.9, seed=12 + bar), base + s)
                put(noise(0.025, 0.14, 8, 1.0, seed=20 + s), base + s + 0.5)
            elif drums == "driving":
                if s % 2 == 0:
                    put(mix(tone("tri", 160, 45, 0.1, 0.6, decay=5), noise(0.03, 0.2, 5, 0.3, seed=11)), base + s)
                if s % 4 == 2:
                    put(noise(0.12, 0.4, 6, 0.9, seed=30 + bar), base + s)
                put(noise(0.02, 0.12, 8, 1.0, seed=40 + s), base + s + 0.5)
            elif drums == "slow":
                if s % 4 == 0:
                    put(mix(tone("tri", 140, 40, 0.15, 0.6, decay=5)), base + s)
                if s == 4:
                    put(noise(0.2, 0.3, 5, 0.7, seed=50), base + s)
    write_wav("mus_" + name, buf, peak=0.8)


def music():
    compose("title", 57, "minor", 118, [0, 5, 3, 4], 101, bars=16, lead_density=0.7, drums="slow", intensity=1.1)
    compose("stage1", 52, "dorian", 150, [0, 0, 5, 4, 0, 3, 5, 4], 202, drums="rock")
    compose("stage2", 55, "minor", 156, [0, 5, 3, 6, 0, 5, 4, 4], 303, drums="driving", lead_wave="tri", intensity=1.3)
    compose("stage3", 50, "phrygian", 164, [0, 1, 0, 5, 0, 1, 4, 3], 404, drums="driving", intensity=1.2)
    compose("boss", 47, "harmonic", 178, [0, 5, 1, 0, 0, 5, 4, 1], 505, bars=16, lead_density=0.85, drums="driving", intensity=1.4)
    compose("gameover", 50, "minor", 70, [0, 5, 3, 4], 606, bars=4, lead_density=0.5, drums="slow", arp=False)
    compose("clear", 60, "dorian", 140, [0, 3, 4, 0], 707, bars=4, lead_density=0.9, drums="rock")


if __name__ == "__main__":
    sfx()
    music()
    print("audio written to", os.path.normpath(OUT))

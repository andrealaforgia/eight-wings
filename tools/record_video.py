#!/usr/bin/env python3
"""Records a full playthrough of Eight Wings to an mp4 with sound.

The game (`--record=DIR`) plays itself with the demo pilot and dumps a BMP every other tick (30 fps) plus a log of every sound event.
This script feeds those frames to ffmpeg as they appear (so they never pile up on disk), mixes the audio offline from the game's own
WAV files following the event log, and muxes both into the final mp4.

usage: python3 tools/record_video.py OUT.mp4 [--width 768 --height 672 --seed 1985]
"""
import argparse, array, os, shutil, struct, subprocess, sys, tempfile, time, wave

RATE = 22050
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SFX = ["", "shot", "shot_heavy", "hit", "enemy_die", "explode_big", "ground_die", "pod_appear", "dock", "component_lost", "formation_start",
       "formation_end", "player_die", "menu_move", "menu_select", "boss_warning", "boss_hit", "boss_die", "one_up", "medal", "enemy_shot", "beam",
       "stage_clear", "pause", "dino_die", "dino_spit"]
MUS = ["", "title", "stage1", "stage2", "stage3", "boss", "gameover", "clear"]
QUIET = {"shot", "enemy_shot", "hit"}


def read_wav(path):
    with wave.open(path) as w:
        assert w.getnchannels() == 1 and w.getsampwidth() == 2 and w.getframerate() == RATE, path
        a = array.array("h")
        a.frombytes(w.readframes(w.getnframes()))
        return a


def mix_audio(events_path, frames, out_path):
    """Replays the event log: sfx are one-shots, music loops from its start whenever the track changes (as in game_audio.c)."""
    total = int(frames * 2 / 60.0 * RATE) + RATE
    mix = array.array("f", [0.0]) * total
    sfx = {i: read_wav(os.path.join(ROOT, "assets/audio/sfx_%s.wav" % n)) for i, n in enumerate(SFX) if n and os.path.exists(os.path.join(ROOT, "assets/audio/sfx_%s.wav" % n))}
    mus = {i: read_wav(os.path.join(ROOT, "assets/audio/mus_%s.wav" % n)) for i, n in enumerate(MUS) if n}
    music_vol, sfx_vol = 7, 8
    music_changes, shots = [], []
    for line in open(events_path):
        p = line.split()
        if len(p) >= 4 and p[1] == "V":
            music_vol, sfx_vol = int(p[2]), int(p[3])
        elif len(p) == 3 and p[1] == "S":
            shots.append((int(p[0]), int(p[2])))
        elif len(p) == 3 and p[1] == "M":
            music_changes.append((int(p[0]), int(p[2])))
    mgain, sgain = music_vol / 10.0 * 0.6, sfx_vol / 10.0 * 0.9
    ends = [t for t, _ in music_changes[1:]] + [total * 60.0 / RATE]
    for (t, track), end in zip(music_changes, ends):
        clip = mus[track]
        s0, s1 = int(t / 60.0 * RATE), min(total, int(end / 60.0 * RATE))
        for i in range(s0, s1):
            mix[i] += clip[(i - s0) % len(clip)] * mgain
    for t, sid in shots:
        clip = sfx.get(sid)
        if clip is None:
            continue
        g = sgain * (0.55 if SFX[sid] in QUIET else 1.0)
        s0 = int(t / 60.0 * RATE)
        for i, v in enumerate(clip):
            if s0 + i >= total:
                break
            mix[s0 + i] += v * g
    out = array.array("h", (max(-32768, min(32767, int(v * 0.8))) for v in mix))
    with wave.open(out_path, "wb") as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(RATE); w.writeframes(out.tobytes())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--width", type=int, default=768)
    ap.add_argument("--height", type=int, default=672)
    ap.add_argument("--seed", type=int, default=1985)
    ap.add_argument("--crf", type=int, default=26)
    args = ap.parse_args()
    ffmpeg = shutil.which("ffmpeg") or "/opt/homebrew/bin/ffmpeg"
    work = tempfile.mkdtemp(prefix="eight-wings-rec-")
    silent = os.path.join(work, "silent.mp4")
    game = subprocess.Popen([os.path.join(ROOT, "eight-wings"), "--record=" + work, "--width=%d" % args.width, "--height=%d" % args.height,
                             "--seed=%d" % args.seed, "--no-audio"], cwd=ROOT)
    enc = subprocess.Popen([ffmpeg, "-y", "-loglevel", "error", "-f", "image2pipe", "-framerate", "30", "-c:v", "bmp", "-i", "-",
                            "-c:v", "libx264", "-preset", "medium", "-crf", str(args.crf), "-pix_fmt", "yuv420p", "-movflags", "+faststart", silent],
                           stdin=subprocess.PIPE)
    n = 0
    while True:
        path = os.path.join(work, "f%06d.bmp" % n)
        if os.path.exists(path):
            with open(path, "rb") as f:
                enc.stdin.write(f.read())
            os.remove(path)
            n += 1
            if n % 1800 == 0:
                print("  %d frames (%d s of video)" % (n, n // 30), flush=True)
        elif os.path.exists(os.path.join(work, "DONE")) or game.poll() is not None:
            if not os.path.exists(path):
                break
        else:
            time.sleep(0.02)
    enc.stdin.close(); enc.wait(); game.wait()
    if n == 0 or enc.returncode or game.returncode:
        print("recording failed (frames=%d, game=%s, ffmpeg=%s)" % (n, game.returncode, enc.returncode)); return 1
    wav = os.path.join(work, "audio.wav")
    print("mixing audio...", flush=True)
    mix_audio(os.path.join(work, "events.txt"), n, wav)
    subprocess.check_call([ffmpeg, "-y", "-loglevel", "error", "-i", silent, "-i", wav, "-c:v", "copy", "-c:a", "aac", "-b:a", "128k", "-shortest", args.out])
    shutil.rmtree(work, ignore_errors=True)
    print("wrote %s (%d frames, %.1f MB)" % (args.out, n, os.path.getsize(args.out) / 1e6))
    return 0


if __name__ == "__main__":
    sys.exit(main())

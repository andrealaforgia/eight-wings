/** Scripted full-run recording: plays the whole game with the demo pilot, dumps a frame every other tick as BMP
    (30 fps) and logs every sound/music event with its tick, so tools/record_video.py can build an mp4 with sound. */
#ifndef PLATFORM_RECORD_H_
#define PLATFORM_RECORD_H_
#include "event_system.h"
#include "game/game.h"
#include "platform/renderer.h"
/** Subscribes the event log (writes out_dir/events.txt); call before game_init. Returns false if the log cannot be opened. */
bool record_attach(event_system_t* events, const char* out_dir);
/** Plays title -> all stages -> end, writing out_dir/f%06d.bmp and a final out_dir/DONE marker. Returns 0 on success. */
int record_run(Renderer* r, Game* g, const char* out_dir);
#endif

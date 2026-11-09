# EIGHT WINGS ASSET CONTRACT  (machine-read by tools/validate_assets.py and tools/pack_atlas.py)
# Format:  name  width  height  frames  group  description
#   frames = 1  -> file <group>/<name>.png
#   frames = N  -> files <group>/<name>.0.png ... <name>.<N-1>.png, ALL exactly width x height
# Anchor point of every sprite = geometric centre (w/2, h/2). Background fully transparent (alpha 0 or 255 only,
# NO semi-transparent pixels). Only colours from tools/assetgen/palette.py. Sprites face UP (nose toward -y) unless
# noted; enemies "face DOWN" (toward the player at the bottom of the screen). Light comes from the top-left.
# ROUND 4 (ULTRA HI-RES, ART_SCALE=4: sizes below are 4x the logical px; the 2x notes in this paragraph are superseded)
# ROUND 3 (HI-RES): every width/height below is in TEXELS at 2x art density (ART_SCALE=2): the game draws each sprite at
# (w/2 x h/2) logical px on the 256x224 playfield, so all art has TWICE the resolution it had in rounds 1-2 (ship_core is now 32x48).
# Logical playfield is still 256x224, so every sprite must have a strong silhouette, a 1px dark outline ('ink' or a
# dark ramp colour), 2-3 shade steps per material, and deliberate highlights. No anti-aliasing, no gradients.
# ----------------------------------------------------------------------------------------------------------------
# GROUP ship  (player vehicle, its docking components, player projectiles, phoenix)
#  Composite ship layout, EIGHT modules (centres relative to core centre, +y = down): C1 left pod (-12,+2), C2 right pod (+12,+2),
#  C3 nose cannon (0,-16), C4 rear crown/thruster (0,+15), C5 outer-left wing cannon (-22,+6), C6 outer-right wing cannon (+22,+6),
#  C7 left canard gun (-10,-12), C8 right canard gun (+10,-12). Components overlap the core by ~2px so they VISIBLY FUSE
#  with the core into one composite vehicle; each must look like an add-on module (distinct silhouette, own colour
#  accent) that clearly extends the base fighter. ship_c2 is the mirror of ship_c1.
ship_core      64 96 2 ship  base fighter, centre fuselage; frame 0/1 differ by engine flame flicker (flame at bottom)
ship_core_bank_l 64 96 1 ship  same fighter banking left (slightly narrower silhouette)
ship_core_bank_r 64 96 1 ship  same fighter banking right
ship_c1        48 64 2 ship  component 1: left wing pod with a gun barrel, 2 frames (small engine flicker)
ship_c2        48 64 2 ship  component 2: right wing pod (mirror of c1)
ship_c3        48 48 2 ship  component 3: nose cannon module, mounts on the front
ship_c4        64 40 2 ship  component 4: rear crown / twin thruster module, mounts on the back
ship_c5        48 56 2 ship  component 5: outer-left wing cannon, long barrel, mounts on the outside of C1 (overlap ~2px with C1), 2 frames
ship_c6        48 56 2 ship  component 6: outer-right wing cannon (mirror of c5, different accent colour than c5)
ship_c7        32 40 2 ship  component 7: left canard: small forward-swept fin with twin gun muzzle, mounts at the core's front-left shoulder
ship_c8        32 40 2 ship  component 8: right canard (mirror of c7, different accent colour)
phoenix        128 128 4 ship  flaming golden phoenix bird facing up, wings flapping over 4 frames (formation super-attack form)
shot_p         16 40 1 ship  normal player bullet (bright, vertical)
shot_w         24 40 1 ship  wider player bullet from wing pods
shot_f         24 48 2 ship  formation attack component bullet (different colour than shot_p, 2-frame shimmer)
shot_ph        64 96 2 ship  large phoenix fire projectile, 2-frame flicker
# GROUP fx
muzzle         32 32 3 fx  muzzle flash, 3 frames growing->fading
spark          32 32 4 fx  bullet-impact spark, 4 frames
expl_s         64 64 6 fx  small air explosion, 6 frames (white-yellow flash -> orange -> red -> smoke)
expl_l         128 128 8 fx  large explosion, 8 frames
expl_g         64 64 6 fx  ground explosion with dark smoke, 6 frames
ship_death     192 192 8 fx  player ship death burst, 8 frames, more dramatic (blue-white core then rings)
dock_flash     96 96 4 fx  sparkling docking flash, 4 frames
cloud          128 64 3 fx  three different small dithered cloud puffs drawn with 'white','g4','b4' (opaque pixels only, checker-dither edges)
# GROUP pickups
pod_1          64 64 2 pickups  component-1 pickup capsule, big readable numeral "1", 2 frames pulsing glow
pod_2          64 64 2 pickups  numeral "2"
pod_3          64 64 2 pickups  numeral "3"
pod_4          64 64 2 pickups  numeral "4"
pod_5          64 64 2 pickups  numeral "5"
pod_6          64 64 2 pickups  numeral "6"
pod_7          64 64 2 pickups  numeral "7"
pod_8          64 64 2 pickups  numeral "8"
medal          32 32 2 pickups  bonus score medal, 2 frames glint
# GROUP enemies  (face down)
e_drone        64 64 4 enemies  basic scout drone, 4-frame anim (spinning/flapping fins)
e_swoop        64 64 4 enemies  swooping interceptor, swept wings, 4 frames
e_diver        64 64 4 enemies  arrowhead kamikaze that dives at the player, 4 frames, glowing engine
e_spinner      64 64 4 enemies  rotating disc/saucer with sensor arms, 4-frame rotation
e_wing         64 64 2 enemies  formation wingman: small sleek fighter, 2 frames
e_bomber       128 96 2 enemies  heavy bomber, armoured, 2 frames (engine glow); mid-size silhouette
e_saucer       96 64 4 enemies  zig-zagging gun-saucer with a dome, 4 frames of light chase
bullet_e       24 24 4 enemies  enemy orb bullet, 4 frames pulsing (must contrast with ground colours: hot pink/orange/white)
bullet_m       24 48 2 enemies  enemy homing missile pointing DOWN, 2 frames
beam           32 64 2 enemies  boss laser beam segment, vertical, tileable vertically, 2 frames
# GROUP ground  (seen from above, sit on terrain)
g_turret       64 64 8 ground  gun turret on a round base, frame i aims at angle i*45 degrees clockwise from UP (0=up,1=up-right,2=right,...,7=up-left)
g_bunker       64 64 3 ground  armoured component bunker/hangar with hatch: 0 closed, 1 closed with blinking red beacon, 2 open-glowing (about to release pod)
g_tank         64 64 2 ground  tracked tank seen from above, facing down, 2 frames track motion
g_silo         64 64 4 ground  missile silo: 0 shut, 1-3 hatch opening
g_fort         128 128 2 ground  large fortress cannon emplacement (needs several hits), 2 frames muzzle glow
g_radar        64 64 4 ground  rotating radar dish on pad, 4 frames
g_raptor       80 80 4 ground  fast walking dinosaur (raptor-like, top-down, facing DOWN), 4-frame walk cycle with legs/tail swinging, olive/green scales with orange crest, 1px outline
g_rex          128 128 4 ground  big walking dinosaur (T-rex-like, top-down, facing DOWN), 4-frame stomp cycle, heavy head, thick tail, brown/red scales
g_bones_s      80 80 1 ground  skeleton of a raptor left on the ground when it dies: pale bone-white (s5/white/g4) ribcage, skull, spine, scattered with a soft shadow; no outline needed beyond dark gaps
g_bones_l      128 128 1 ground  skeleton of the big dinosaur: skull with teeth, ribcage, long spine and tail, bone-white; clearly a fossil-like skeleton lying on terrain
g_rubble_s     64 64 1 ground  wreckage left by destroyed 16x16 ground target (dark, burnt)
g_rubble_l     128 128 1 ground  wreckage left by destroyed 32x32 ground target
# GROUP bosses  (face down)  body is big; core = the vulnerable weak point (flashes); gun = turret part
boss1_body     384 224 1 bosses  stage-1 boss: armoured walker-fortress, 'Gargantua'. Leave 16x16 empty-looking mounting recesses at body-relative offsets (-28,+8) and (+28,+8) for guns and at (0,+14) for the core
boss1_core     64 64 2 bosses  its exposed reactor core, 2 frames: normal / white-hot flash
boss1_gun      64 64 4 bosses  its side gun turret (4 frames: idle, charge, fire, recoil)
boss2_body     384 224 1 bosses  stage-2 boss: sea-serpent-like segmented battleship 'Hydra' (different silhouette & palette family: cyan/blue)
boss2_core     64 64 2 bosses  core
boss2_gun      64 64 4 bosses  gun
boss3_body     384 224 1 bosses  stage-3 boss: mechanical queen / final mothership 'Mother Core' (purple/magenta/red family)
boss3_core     64 64 2 bosses  core
boss3_gun      64 64 4 bosses  gun
# GROUP ui
logo_title     768 192 1 ui  big chunky arcade logo reading EIGHT WINGS, 2-colour-ramp metallic lettering with outline and a subtle sci-fi star/wing motif, stylised and hand-pixelled-looking
ui_life        32 32 1 ui  tiny ship icon (lives counter)
ui_comp        32 32 8 ui  eight small component icons showing numerals 1..8 (frame i = component i+1)
# font: 95 glyphs, 8x8, ASCII 32..126, frame index = ascii-32, white pixels ('white') on transparent, 1px letter-spacing built in,
# classic arcade look (chunky, upper-case friendly; lower-case may be small-caps style).
font           32 32 95 ui  bitmap font
# GROUP tiles : 16x16 opaque terrain tiles, scrolled by the game. Three themes T=0,1,2:
#   T0 'Badlands' (desert canyon: sand, red rock, scrub, scorched craters, concrete roads, dry riverbed water=oasis blue)
#   T1 'Archipelago' (ocean with islands: deep blue animated-looking water, green island grass, sand shores, jungle, piers/roads)
#   T2 'Fortress' (industrial alien-tech: dark metal plating, pipes, vents, lit panels, coolant (cyan) pools, hazard stripes)
# EVERY tile is fully opaque. base0..3 are alternates of the same ground that tile SEAMLESSLY with each other in any arrangement
# (same average colour; no visible seams at 16px edges). liqM = liquid autotile, bits of M = which neighbour sides are LAND
# (1=north, 2=east, 4=south, 8=west); liq0 is open liquid with no shore; liq15 is a tiny pond surrounded by land. Shore pixels
# should follow a wavy, noisy edge and match between adjacent tiles (shore edge position must be identical on shared sides).
tile_t0_base0  64 64 1 tiles  theme 0 plain ground variant 0
tile_t0_base1  64 64 1 tiles  theme 0 plain ground variant 1
tile_t0_base2  64 64 1 tiles  theme 0 plain ground variant 2
tile_t0_base3  64 64 1 tiles  theme 0 plain ground variant 3
tile_t0_detail0 64 64 1 tiles  theme 0 ground with detail variant 0 (T0: rocks/scrub/crater/scorch; T1: rocks/grass tuft/shells/wreck; T2: vent/panel light/rivets/hatch) — edges must blend with base
tile_t0_detail1 64 64 1 tiles  theme 0 ground with detail variant 1 (T0: rocks/scrub/crater/scorch; T1: rocks/grass tuft/shells/wreck; T2: vent/panel light/rivets/hatch) — edges must blend with base
tile_t0_detail2 64 64 1 tiles  theme 0 ground with detail variant 2 (T0: rocks/scrub/crater/scorch; T1: rocks/grass tuft/shells/wreck; T2: vent/panel light/rivets/hatch) — edges must blend with base
tile_t0_detail3 64 64 1 tiles  theme 0 ground with detail variant 3 (T0: rocks/scrub/crater/scorch; T1: rocks/grass tuft/shells/wreck; T2: vent/panel light/rivets/hatch) — edges must blend with base
tile_t0_forest0 64 64 1 tiles  theme 0 vegetation or dense clutter cluster variant 0 (T2: pipe bundle/machinery clutter)
tile_t0_forest1 64 64 1 tiles  theme 0 vegetation or dense clutter cluster variant 1 (T2: pipe bundle/machinery clutter)
tile_t0_struct0 64 64 1 tiles  theme 0 decorative non-interactive structure seen from above variant 0 (buildings, domes, tanks, landing pads)
tile_t0_struct1 64 64 1 tiles  theme 0 decorative non-interactive structure seen from above variant 1 (buildings, domes, tanks, landing pads)
tile_t0_struct2 64 64 1 tiles  theme 0 decorative non-interactive structure seen from above variant 2 (buildings, domes, tanks, landing pads)
tile_t0_struct3 64 64 1 tiles  theme 0 decorative non-interactive structure seen from above variant 3 (buildings, domes, tanks, landing pads)
tile_t0_road_v 64 64 1 tiles  theme 0 road/causeway/walkway running vertically (north-south), left+right edges blend with base
tile_t0_road_h 64 64 1 tiles  theme 0 same running horizontally (east-west)
tile_t0_road_x 64 64 1 tiles  theme 0 crossroads of the above
tile_t0_liq0   64 64 1 tiles  theme 0 liquid autotile mask 0
tile_t0_liq1   64 64 1 tiles  theme 0 liquid autotile mask 1
tile_t0_liq2   64 64 1 tiles  theme 0 liquid autotile mask 2
tile_t0_liq3   64 64 1 tiles  theme 0 liquid autotile mask 3
tile_t0_liq4   64 64 1 tiles  theme 0 liquid autotile mask 4
tile_t0_liq5   64 64 1 tiles  theme 0 liquid autotile mask 5
tile_t0_liq6   64 64 1 tiles  theme 0 liquid autotile mask 6
tile_t0_liq7   64 64 1 tiles  theme 0 liquid autotile mask 7
tile_t0_liq8   64 64 1 tiles  theme 0 liquid autotile mask 8
tile_t0_liq9   64 64 1 tiles  theme 0 liquid autotile mask 9
tile_t0_liq10  64 64 1 tiles  theme 0 liquid autotile mask 10
tile_t0_liq11  64 64 1 tiles  theme 0 liquid autotile mask 11
tile_t0_liq12  64 64 1 tiles  theme 0 liquid autotile mask 12
tile_t0_liq13  64 64 1 tiles  theme 0 liquid autotile mask 13
tile_t0_liq14  64 64 1 tiles  theme 0 liquid autotile mask 14
tile_t0_liq15  64 64 1 tiles  theme 0 liquid autotile mask 15
tile_t1_base0  64 64 1 tiles  theme 1 plain ground variant 0
tile_t1_base1  64 64 1 tiles  theme 1 plain ground variant 1
tile_t1_base2  64 64 1 tiles  theme 1 plain ground variant 2
tile_t1_base3  64 64 1 tiles  theme 1 plain ground variant 3
tile_t1_detail0 64 64 1 tiles  theme 1 ground with detail variant 0 (T0: rocks/scrub/crater/scorch; T1: rocks/grass tuft/shells/wreck; T2: vent/panel light/rivets/hatch) — edges must blend with base
tile_t1_detail1 64 64 1 tiles  theme 1 ground with detail variant 1 (T0: rocks/scrub/crater/scorch; T1: rocks/grass tuft/shells/wreck; T2: vent/panel light/rivets/hatch) — edges must blend with base
tile_t1_detail2 64 64 1 tiles  theme 1 ground with detail variant 2 (T0: rocks/scrub/crater/scorch; T1: rocks/grass tuft/shells/wreck; T2: vent/panel light/rivets/hatch) — edges must blend with base
tile_t1_detail3 64 64 1 tiles  theme 1 ground with detail variant 3 (T0: rocks/scrub/crater/scorch; T1: rocks/grass tuft/shells/wreck; T2: vent/panel light/rivets/hatch) — edges must blend with base
tile_t1_forest0 64 64 1 tiles  theme 1 vegetation or dense clutter cluster variant 0 (T2: pipe bundle/machinery clutter)
tile_t1_forest1 64 64 1 tiles  theme 1 vegetation or dense clutter cluster variant 1 (T2: pipe bundle/machinery clutter)
tile_t1_struct0 64 64 1 tiles  theme 1 decorative non-interactive structure seen from above variant 0 (buildings, domes, tanks, landing pads)
tile_t1_struct1 64 64 1 tiles  theme 1 decorative non-interactive structure seen from above variant 1 (buildings, domes, tanks, landing pads)
tile_t1_struct2 64 64 1 tiles  theme 1 decorative non-interactive structure seen from above variant 2 (buildings, domes, tanks, landing pads)
tile_t1_struct3 64 64 1 tiles  theme 1 decorative non-interactive structure seen from above variant 3 (buildings, domes, tanks, landing pads)
tile_t1_road_v 64 64 1 tiles  theme 1 road/causeway/walkway running vertically (north-south), left+right edges blend with base
tile_t1_road_h 64 64 1 tiles  theme 1 same running horizontally (east-west)
tile_t1_road_x 64 64 1 tiles  theme 1 crossroads of the above
tile_t1_liq0   64 64 1 tiles  theme 1 liquid autotile mask 0
tile_t1_liq1   64 64 1 tiles  theme 1 liquid autotile mask 1
tile_t1_liq2   64 64 1 tiles  theme 1 liquid autotile mask 2
tile_t1_liq3   64 64 1 tiles  theme 1 liquid autotile mask 3
tile_t1_liq4   64 64 1 tiles  theme 1 liquid autotile mask 4
tile_t1_liq5   64 64 1 tiles  theme 1 liquid autotile mask 5
tile_t1_liq6   64 64 1 tiles  theme 1 liquid autotile mask 6
tile_t1_liq7   64 64 1 tiles  theme 1 liquid autotile mask 7
tile_t1_liq8   64 64 1 tiles  theme 1 liquid autotile mask 8
tile_t1_liq9   64 64 1 tiles  theme 1 liquid autotile mask 9
tile_t1_liq10  64 64 1 tiles  theme 1 liquid autotile mask 10
tile_t1_liq11  64 64 1 tiles  theme 1 liquid autotile mask 11
tile_t1_liq12  64 64 1 tiles  theme 1 liquid autotile mask 12
tile_t1_liq13  64 64 1 tiles  theme 1 liquid autotile mask 13
tile_t1_liq14  64 64 1 tiles  theme 1 liquid autotile mask 14
tile_t1_liq15  64 64 1 tiles  theme 1 liquid autotile mask 15
tile_t2_base0  64 64 1 tiles  theme 2 plain ground variant 0
tile_t2_base1  64 64 1 tiles  theme 2 plain ground variant 1
tile_t2_base2  64 64 1 tiles  theme 2 plain ground variant 2
tile_t2_base3  64 64 1 tiles  theme 2 plain ground variant 3
tile_t2_detail0 64 64 1 tiles  theme 2 ground with detail variant 0 (T0: rocks/scrub/crater/scorch; T1: rocks/grass tuft/shells/wreck; T2: vent/panel light/rivets/hatch) — edges must blend with base
tile_t2_detail1 64 64 1 tiles  theme 2 ground with detail variant 1 (T0: rocks/scrub/crater/scorch; T1: rocks/grass tuft/shells/wreck; T2: vent/panel light/rivets/hatch) — edges must blend with base
tile_t2_detail2 64 64 1 tiles  theme 2 ground with detail variant 2 (T0: rocks/scrub/crater/scorch; T1: rocks/grass tuft/shells/wreck; T2: vent/panel light/rivets/hatch) — edges must blend with base
tile_t2_detail3 64 64 1 tiles  theme 2 ground with detail variant 3 (T0: rocks/scrub/crater/scorch; T1: rocks/grass tuft/shells/wreck; T2: vent/panel light/rivets/hatch) — edges must blend with base
tile_t2_forest0 64 64 1 tiles  theme 2 vegetation or dense clutter cluster variant 0 (T2: pipe bundle/machinery clutter)
tile_t2_forest1 64 64 1 tiles  theme 2 vegetation or dense clutter cluster variant 1 (T2: pipe bundle/machinery clutter)
tile_t2_struct0 64 64 1 tiles  theme 2 decorative non-interactive structure seen from above variant 0 (buildings, domes, tanks, landing pads)
tile_t2_struct1 64 64 1 tiles  theme 2 decorative non-interactive structure seen from above variant 1 (buildings, domes, tanks, landing pads)
tile_t2_struct2 64 64 1 tiles  theme 2 decorative non-interactive structure seen from above variant 2 (buildings, domes, tanks, landing pads)
tile_t2_struct3 64 64 1 tiles  theme 2 decorative non-interactive structure seen from above variant 3 (buildings, domes, tanks, landing pads)
tile_t2_road_v 64 64 1 tiles  theme 2 road/causeway/walkway running vertically (north-south), left+right edges blend with base
tile_t2_road_h 64 64 1 tiles  theme 2 same running horizontally (east-west)
tile_t2_road_x 64 64 1 tiles  theme 2 crossroads of the above
tile_t2_liq0   64 64 1 tiles  theme 2 liquid autotile mask 0
tile_t2_liq1   64 64 1 tiles  theme 2 liquid autotile mask 1
tile_t2_liq2   64 64 1 tiles  theme 2 liquid autotile mask 2
tile_t2_liq3   64 64 1 tiles  theme 2 liquid autotile mask 3
tile_t2_liq4   64 64 1 tiles  theme 2 liquid autotile mask 4
tile_t2_liq5   64 64 1 tiles  theme 2 liquid autotile mask 5
tile_t2_liq6   64 64 1 tiles  theme 2 liquid autotile mask 6
tile_t2_liq7   64 64 1 tiles  theme 2 liquid autotile mask 7
tile_t2_liq8   64 64 1 tiles  theme 2 liquid autotile mask 8
tile_t2_liq9   64 64 1 tiles  theme 2 liquid autotile mask 9
tile_t2_liq10  64 64 1 tiles  theme 2 liquid autotile mask 10
tile_t2_liq11  64 64 1 tiles  theme 2 liquid autotile mask 11
tile_t2_liq12  64 64 1 tiles  theme 2 liquid autotile mask 12
tile_t2_liq13  64 64 1 tiles  theme 2 liquid autotile mask 13
tile_t2_liq14  64 64 1 tiles  theme 2 liquid autotile mask 14
tile_t2_liq15  64 64 1 tiles  theme 2 liquid autotile mask 15
# ---------------- ROUND 6: more excitement (new enemies, big dinosaurs, weapon types) ----------------
e_weaver       64 64 4 enemies  sine-wave flyer: slim crescent-winged fighter in teal/lime, arrives in chains and snakes across the screen, 4 frames (wing beat), faces DOWN
e_hunter       64 64 4 enemies  homing dart: needle body with a glowing red eye and swept fins, chases the player and turns toward them, 4 frames (engine pulse), faces DOWN
e_gunship      128 96 2 enemies  armoured gunship: wide twin-cannon hull in steel and crimson with a front glass canopy, hovers and fires aimed bursts, 2 frames (cannon glow), faces DOWN
e_splitter     80 80 4 enemies  segmented egg-pod carrier in purple and bone-white with visible seams; when destroyed it releases three drones; 4 frames (pulsing core)
g_trike        160 160 4 ground  big charging triceratops seen from above, facing DOWN: bony frill, three horns, armoured back plates, heavy legs, 4-frame stampede cycle, ochre/rust scales with a cream frill
g_spino        176 176 4 ground  spinosaurus seen from above, facing DOWN: long crocodile snout, tall dorsal sail with stripes, clawed forelimbs, thick tail, 4-frame walk cycle, teal/blue-green with an orange sail
g_bronto       224 224 4 ground  huge long-necked sauropod seen from above, facing DOWN: small head on a long curved neck, massive round body, four pillar legs, long tapering tail, 4-frame slow stomp, olive-grey with pale underbelly spots
g_bones_trike  160 160 1 ground  fossil skeleton of the triceratops lying on the ground: horned skull with frill, ribcage, spine, bone-white
g_bones_spino  176 176 1 ground  fossil skeleton of the spinosaurus: long skull, sail spines, ribcage, tail, bone-white
g_bones_bronto 224 224 1 ground  fossil skeleton of the sauropod: tiny skull, long neck vertebrae, huge ribcage, tail, bone-white
shot_pellet    16 16 1 ship  small round spread-gun pellet, hot yellow core with orange rim
shot_laser     12 64 2 ship  long thin laser beam segment pointing up, white-cyan core with cyan glow rim, 2 frames shimmer
shot_missile   20 40 2 ship  player homing missile pointing UP: slim body, red nose, fins, orange exhaust flame, 2 frames flame flicker
shot_wave      40 24 2 ship  crescent wave bullet (arc opening downward, tips curling back), lime-green to white, 2 frames shimmer
wp_vulcan      64 64 2 pickups  weapon capsule HEXAGONAL (clearly not the round numbered module pods) with a big letter "V", blue family, 2 frames pulsing glow
wp_spread      64 64 2 pickups  hexagonal weapon capsule, big letter "S", orange family
wp_laser       64 64 2 pickups  hexagonal weapon capsule, big letter "L", cyan family
wp_missile     64 64 2 pickups  hexagonal weapon capsule, big letter "M", red family
wp_wave        64 64 2 pickups  hexagonal weapon capsule, big letter "W", green family

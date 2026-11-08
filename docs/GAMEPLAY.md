# Gameplay

## Loop
Title → (Enter) → Stage 1 *Badlands* → boss → Stage 2 *Archipelago* → boss → Stage 3 *The Fortress* → boss → the loop restarts at higher
difficulty (faster, denser bullets; tougher enemies and bosses). Three lives; extra lives at 30,000 and every 70,000 points. High score is saved
(`SDL_GetPrefPath`). Idle on the title screen: controls page, then an attract-mode demo flown by the built-in pilot; any key returns to the title.
Game over shows the score; Enter restarts.

## Ship components and docking
You start with the basic fighter (core). Ground **bunkers** (armoured hatch with a blinking beacon) hide numbered modules. Destroy one and a
numbered **pod** (1–8) drops; fly into it and the module slides into its slot and fuses with the core. The pod released is always the lowest
module you do not have (and that is not already falling), so lost modules can be recovered. With all eight, bunkers give medals instead.

| Module | Slot (from core) | Effect |
|---|---|---|
| 1 left pod | (-12,+2) | angled shot on the left |
| 2 right pod | (+12,+2) | angled shot on the right |
| 3 nose cannon | (0,-16) | centre shot becomes a heavy 2-damage bolt |
| 4 rear crown | (0,+15) | +18% speed and two wide side shots |
| 5 outer-left wing cannon | (-22,+6) | wide-angle shot on the far left |
| 6 outer-right wing cannon | (+22,+6) | wide-angle shot on the far right |
| 7 left canard | (-10,-12) | fast straight bolt |
| 8 right canard | (+10,-12) | fast straight bolt |

Bullets per volley with 0..8 modules: 1, 2, 3, 3, 5, 6, 7, 8, 9; damage per volley: 1, 2, 3, 4, 6, 7, 8, 9, 10 (covered by tests). The hurtbox grows with the ship: a **hit on a module
destroys only that module** (75 ticks of invulnerability follow); a hit on the **core** destroys the ship, all modules and one life. Respawn takes 100 ticks, then
180 ticks of invulnerability. Touching an enemy body is also a hit (the enemy dies unless it is a bomber).

## Formation attack (X)
* **Charge**: the gauge fills from kills (air 4, ground 6–8, bomber 10, boss guns 15) and docking (25) and medals (10).
* **Requirements**: gauge full, at least 2 modules **fully docked**, no cooldown. It cannot be started while docking is in progress.
* **Sequence** (60 Hz ticks): `SCATTER` 30 → `HOLD` 300 → `REUNITE` 32 → `COOLDOWN` 90.
  Modules fly out from their slots to evenly spaced positions on a radius-36 orbit (angular speed 0.075 rad/tick); with more than four modules two counter-rotating rings (radius 30 inside, 48 outside) form, fire radial shots (pinwheel) plus
  upward shots, and ram enemies (2 damage every 5 ticks). They absorb enemy bullets. The core is invulnerable for the whole attack and keeps firing.
  The gauge doubles as the remaining-time bar, then reunion returns each module exactly to its slot with a docking flash.
* **Phoenix**: with all eight modules the core becomes a flaming phoenix for the HOLD phase: piercing 4-damage fire shots, a large bullet-absorbing body,
  contact damage 3 every 4 ticks.
* The geometry is a pure function (`formation_offset`) covered by tests: start offset = slot, constant orbit radius, evenly spaced, exact return.

## Enemies
Air (face down): **Drone** snake (sine weave), **Swoop** (arc and U-turn from a side), **Wing** V-formation (peels off sideways and fires), **Diver**
(stops, flashes, then dashes at your position), **Spinner** (weaves, 3-way shots, 3+ hp), **Saucer** (zig-zag shooter), **Bomber** (heavy, 5-way fan, drops a medal).
Ground (scroll with the terrain): **Raptor** packs (hunt your column, 2 hp) and the **Rex** (10 hp, amble down the screen, spit twin fireballs, drop a medal) are walking dinosaurs that never wade into water; when killed they **leave their skeleton behind**, lying in the pose they died in and scrolling away with the terrain. **Turret** (8-direction aim), **Bunker** (pods), **Tank** (patrols), **Silo** (accelerating missiles), **Fort** (big, 5-way fan), **Radar** (medal).
Spawns come from a deterministic per-stage script: weighted wave pools get harder per stage; one showpiece fortress per stage; bunkers every ~500 px.

## Bosses
Each boss has two guns and a core. Phase 0: guns are the targets (hull armour absorbs shots, core is shielded). Both guns dead → phase 1: core exposed,
new attacks, enemy bullets cleared. Core below 40% → phase 2: enrage. Touching the hull is lethal. Defeat plays a ~2 s chain of explosions, drops medals, clears bullets.
* **Gargantua** (stage 1): aimed 3-bursts → 12-way rings → ring + spiral.
* **Hydra** (stage 2): 5-way fans → sweeping streams and telegraphed beams → rings + double beams.
* **Mother Core** (stage 3): rapid aimed streams → twin spirals + drone summons → triple spiral, triple beams, rings.
Beams are telegraphed (45 ticks of flickering line) before dealing damage (50 ticks).

## Scoring
Drone 100, Wing 120, Swoop 150, Diver 200, Spinner 300, Saucer 400, Bomber 800; turret 200, bunker 300, tank 300, silo 250, radar 150, fort 2000;
docking 500; medal 1000; boss guns 1000; boss core 3000×stage; stage clear 5000×stage.

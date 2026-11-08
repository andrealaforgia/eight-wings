# ROUND 2 — graphics quality pass + new sprites (Claude Code -> Codex)

The game is running (see `screenshots/*.png` in the repo: LOOK AT THEM, they are real in-game captures at 3x). The owner says: "improve the graphics
of the various elements; many more ship pieces; walking dinosaurs that leave bones". The spec `assets/spec/sprites.spec` has been UPDATED with the new
sprites (ship_c5..c8, pod_5..pod_8, ui_comp now 8 frames, g_raptor, g_rex, g_bones_s, g_bones_l). Read it again; sizes and names are binding.

Read first: BRIEF_COMMON.md rules still apply (palette only, stdlib only, deterministic, validator must pass: `python3 tools/validate_assets.py --group <g>`).
You may now ALSO edit the existing generator files named in your job (rewrite freely) — keep `generate()` and output names/sizes identical to the spec.

## What "better graphics" means here (look at screenshots, then fix what you see)
* More **form and material**: every object should read as a 3-D lit object (top-left light): highlight edge, mid, shade, darkest core shadow, plus a coloured
  reflected-light pixel on the shaded side. Use 4+ tonal steps for hulls/metal. Panel lines, rivets, vents, cockpit glass with a specular pixel, thruster glows.
* More **silhouette character** and **detail density** at the same pixel size: add wing struts, antennae, intake vents, weapon hardpoints, hazard markings.
* Better **animation**: engines flicker, lights chase, turrets/radars/legs move; frames must differ visibly yet keep the same silhouette centre.
* **Terrain**: richer, less noisy, more believable; larger coherent features (dune ripples, strata cracks, tyre tracks, tide lines, wave highlights, metal plating with seams/bolts/glow);
  keep it lower-contrast and lower-saturation than sprites so enemies and bullets pop. Forest/jungle clusters should look like tree canopies with shaded undersides, not noise.
* Dither sparingly and deliberately (ordered 2x2 patterns), never random speckle noise.
* Enemy bullets must be unmistakable (bright core, dark outline, glow ring pixels).
Spend real effort: view enlarged contact sheets (render with `python3 tools/contact_sheet.py <group> 6`, which writes /tmp/sheet_<group>.png; open it if you can view images)
and iterate several times. The previous round got a good baseline — this round must clearly surpass it.

Reworked [gen_tiles.py](/Users/andrealaforgia/dev/games/eight-wings/tools/assetgen/gen_tiles.py) and regenerated all 99 tiles with dune contours, clearer surf, shaded canopies, metal plating, hazards and richer structures.

Added [_c_checks.py](/Users/andrealaforgia/dev/games/eight-wings/tools/assetgen/_c_checks.py): eight checks pass, covering seams, shorelines, road joins and determinism. Asset validator: **99 files, 0 problems**. Repeated generation produces byte-identical PNGs.

Reviewed enlarged sheets and tiled previews. No spec problems. More time would go towards reducing visible repetition within the fixed 16×16 tile set.
"""Contract checks for terrain generation; run directly with Python."""
import unittest

from gen_tiles import build_tiles
from palette import PALETTE


def edge(c, side):
    if side == 'n':
        return tuple(c.get(x, 0) for x in range(c.w))
    if side == 's':
        return tuple(c.get(x, c.h - 1) for x in range(c.w))
    if side == 'w':
        return tuple(c.get(0, y) for y in range(c.h))
    return tuple(c.get(c.w - 1, y) for y in range(c.h))


class TerrainChecks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tiles = build_tiles()

    def test_complete_opaque_deterministic_set(self):
        self.assertEqual(99, len(self.tiles))
        again = build_tiles()
        for name, c in self.tiles.items():
            self.assertEqual((64, 64), (c.w, c.h))
            self.assertEqual(4096, c.opaque_count())
            self.assertTrue(set(c.px) <= set(PALETTE.values()))
            self.assertEqual(c.px, again[name].px)

    def test_ground_variants_and_decoration_share_edges(self):
        for theme in range(3):
            prefix = f'tile_t{theme}_'
            base = self.tiles[prefix + 'base0']
            self.assertEqual(edge(base, 'n'), edge(base, 's'))
            self.assertEqual(edge(base, 'e'), edge(base, 'w'))
            for kind, count in [('base', 4), ('detail', 4), ('forest', 2), ('struct', 4)]:
                for n in range(count):
                    for side in 'nsew':
                        self.assertEqual(edge(base, side), edge(self.tiles[prefix + kind + str(n)], side))

    def test_road_ports_match_crossing_and_opposite_ends(self):
        for theme in range(3):
            prefix = f'tile_t{theme}_road_'
            cross = self.tiles[prefix + 'x']
            for direction, sides in [('v', 'ns'), ('h', 'ew')]:
                road = self.tiles[prefix + direction]
                self.assertEqual(edge(road, sides[0]), edge(road, sides[1]))
                for side in sides:
                    self.assertEqual(edge(road, side), edge(cross, side))

    def test_road_sides_blend_into_ground(self):
        for theme in range(3):
            base = self.tiles[f'tile_t{theme}_base0']
            for direction, sides in [('v', 'ew'), ('h', 'ns')]:
                road = self.tiles[f'tile_t{theme}_road_{direction}']
                for side in sides:
                    self.assertEqual(edge(base, side), edge(road, side))

    def test_compatible_water_neighbours_have_identical_edges(self):
        for theme in range(3):
            prefix = f'tile_t{theme}_liq'
            for a in range(16):
                for b in range(16):
                    if not a & 2 and not b & 8 and a & 5 == b & 5:
                        self.assertEqual(edge(self.tiles[prefix + str(a)], 'e'), edge(self.tiles[prefix + str(b)], 'w'))
                    if not a & 4 and not b & 1 and a & 10 == b & 10:
                        self.assertEqual(edge(self.tiles[prefix + str(a)], 's'), edge(self.tiles[prefix + str(b)], 'n'))

    def test_land_facing_shore_edges_match_ground(self):
        for theme in range(3):
            base = self.tiles[f'tile_t{theme}_base0']
            for mask in range(16):
                liquid = self.tiles[f'tile_t{theme}_liq{mask}']
                for bit, side in ((1, 'n'), (2, 'e'), (4, 's'), (8, 'w')):
                    if mask & bit:
                        self.assertEqual(edge(base, side), edge(liquid, side))

    def test_meeting_banks_round_the_inside_water_corner(self):
        north = self.tiles['tile_t0_liq1']
        east = self.tiles['tile_t0_liq2']
        corner = self.tiles['tile_t0_liq3']
        sand = {PALETTE[name] for name in ('s2', 's3', 's4')}
        rounded = sum(n not in sand and e not in sand and c in sand
                      for n, e, c in zip(north.px, east.px, corner.px))
        self.assertGreater(rounded, 5)

    def test_art_has_native_density_detail(self):
        for name, canvas in self.tiles.items():
            varied_blocks = sum(len({canvas.get(x + dx, y + dy)
                                     for dx in (0, 1) for dy in (0, 1)}) > 1
                                for x in range(0, 64, 2) for y in range(0, 64, 2))
            self.assertGreater(varied_blocks, 24, name)

    def test_four_distinct_base_variants_per_theme(self):
        for theme in range(3):
            self.assertEqual(4, len({tuple(self.tiles[f'tile_t{theme}_base{i}'].px)
                                     for i in range(4)}))

    def test_ocean_has_readable_wave_crests(self):
        ocean = self.tiles['tile_t1_liq0']
        self.assertGreaterEqual(sum(p in (PALETTE['c2'], PALETTE['c3']) for p in ocean.px), 8)

    def test_coolant_has_a_small_luminous_crest(self):
        self.assertGreaterEqual(self.tiles['tile_t2_liq0'].px.count(PALETTE['c3']), 2)

    def test_fortress_ground_has_recesses_and_metal_highlights(self):
        for variant in range(4):
            pixels = self.tiles[f'tile_t2_base{variant}'].px
            self.assertGreaterEqual(pixels.count(PALETTE['ink']), 8)
            self.assertGreaterEqual(pixels.count(PALETTE['g3']), 2)


if __name__ == '__main__':
    unittest.main()

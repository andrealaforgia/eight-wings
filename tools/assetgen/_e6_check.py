"""Focused Round 6 contract and preservation checks (stdlib only).

Run: python3 tools/assetgen/_e6_check.py
An optional /tmp/e6_existing_hashes.json records the pre-job shipped assets.
"""
import hashlib
import json
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from spec import read_spec, files_for, ROOT
from validate_assets import check_sprite
from _e6_enemies import weaver, hunter, gunship, splitter
from _e6_creatures import dinosaur, anatomy

NEW = {'e_weaver', 'e_hunter', 'e_gunship', 'e_splitter',
       'g_trike', 'g_spino', 'g_bronto', 'g_bones_trike', 'g_bones_spino', 'g_bones_bronto'}


class Round6Checks(unittest.TestCase):
    def test_new_art_matches_contract_and_every_frame_is_distinct(self):
        entries = [entry for entry in read_spec() if entry['name'] in NEW]
        self.assertEqual(len(entries), 10)
        for entry in entries:
            with self.subTest(sprite=entry['name']):
                errors, frames = [], []
                for path in files_for(entry, str(Path(ROOT)/'assets/generated')):
                    self.assertTrue(Path(path).is_file(), path)
                    frames.append(check_sprite(path, entry, errors))
                self.assertEqual(errors, [])
                self.assertEqual(len(set(frames)), entry['frames'])

    def test_shipped_art_has_identical_bytes(self):
        path = Path('/tmp/e6_existing_hashes.json')
        if not path.exists():
            self.skipTest('Pre-job hashes were not captured in this session')
        for name, expected in json.loads(path.read_text()).items():
            with self.subTest(sprite=name):
                self.assertEqual(hashlib.sha256(Path(ROOT, name).read_bytes()).hexdigest(), expected)

    def test_live_and_fossil_landmarks_share_the_same_pose(self):
        for kind in ('trike', 'spino', 'bronto'):
            fixed = anatomy(kind, 0)[:-1]
            for frame in range(4):
                self.assertEqual(anatomy(kind, frame)[:-1], fixed)
            for frame in range(1, 4):
                self.assertNotEqual(anatomy(kind, frame)[-1], anatomy(kind, 0)[-1])

    def test_procedural_render_is_reproducible(self):
        for name, draw in [('e_weaver', weaver), ('e_hunter', hunter),
                           ('e_gunship', gunship), ('e_splitter', splitter)]:
            with self.subTest(sprite=name):
                self.assertEqual(draw(1).px, draw(1).px)
        for kind in ('trike', 'spino', 'bronto'):
            with self.subTest(sprite=kind):
                self.assertEqual(dinosaur(kind, 1).px, dinosaur(kind, 1).px)


if __name__ == '__main__':
    unittest.main()

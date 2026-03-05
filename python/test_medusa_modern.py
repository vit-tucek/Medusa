#!/usr/bin/env python3

import sys
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))

import numpy as np

from medusa_modern import (
    doubling_orbit,
    kneading_data,
    read_param_file,
    render_plane_pullback,
    render_sphere_pullback,
)
from medusa_full import compute_parameter_sequence


class MedusaModernTests(unittest.TestCase):
    def test_doubling_orbit_periodic(self) -> None:
        orbit = doubling_orbit(1, 7)
        self.assertEqual(orbit.preperiod, 0)
        self.assertEqual(orbit.period, 3)
        self.assertEqual(orbit.orbit.tolist(), [1, 2, 4])

    def test_kneading_data_sizes(self) -> None:
        orbit = doubling_orbit(1, 7)
        preim1, preim2 = kneading_data(7, orbit.orbit, orbit.period, orbit.preperiod)
        self.assertEqual(len(preim1), orbit.period + orbit.preperiod)
        self.assertEqual(len(preim2), orbit.period + orbit.preperiod)

    def test_render_smoke(self) -> None:
        root = Path(__file__).resolve().parent.parent
        a_params, b_params = read_param_file(root / "param.txt")
        self.assertTrue(len(a_params) > 0)
        self.assertEqual(len(a_params), len(b_params))

        plane = render_plane_pullback(a_params, b_params, dpi=24)
        self.assertEqual(plane.inside_unit_disk.ndim, 2)
        self.assertTrue(np.isfinite(plane.inside_unit_disk.astype(np.float64)).all())

        sphere = render_sphere_pullback(a_params, b_params, dpi=24)
        self.assertEqual(sphere.classes.ndim, 2)
        self.assertTrue(set(np.unique(sphere.classes)).issubset({0, 2, 3}))

    def test_full_port_matches_cpp_prefix(self) -> None:
        root = Path(__file__).resolve().parent.parent
        a_cpp, b_cpp = read_param_file(root / "output" / "m17v37.txt")
        a_py, b_py = compute_parameter_sequence(1, 7, 3, 7, iterations=10, stop_on_failure=True)
        n = min(5, len(a_cpp), len(a_py))
        self.assertGreaterEqual(n, 5)
        self.assertLess(np.max(np.abs(a_cpp[:n] - a_py[:n])), 1e-5)
        self.assertLess(np.max(np.abs(b_cpp[:n] - b_py[:n])), 1e-5)


if __name__ == "__main__":
    unittest.main()

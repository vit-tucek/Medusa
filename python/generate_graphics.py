#!/usr/bin/env python3
"""Generate Medusa graphics from a param.txt-style parameter sequence."""

from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

from medusa_modern import (
    plot_parameter_trajectory,
    plot_plane_pullback,
    plot_sphere_pullback,
    read_param_file,
    render_plane_pullback,
    render_sphere_pullback,
    save_figure,
)
from medusa_full import compute_parameter_sequence, write_param_file


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--param-file", default=Path("param.txt"), type=Path, help="input parameter file")
    parser.add_argument("--p1", type=int, help="mating angle numerator 1")
    parser.add_argument("--q1", type=int, help="mating angle denominator 1")
    parser.add_argument("--p2", type=int, help="mating angle numerator 2")
    parser.add_argument("--q2", type=int, help="mating angle denominator 2")
    parser.add_argument("--iterations", default=35, type=int, help="iterations for Python full-port parameter lift")
    parser.add_argument(
        "--stop-on-failure",
        action="store_true",
        help="stop early if one full-port iterate fails (default: continue)",
    )
    parser.add_argument(
        "--write-param-file",
        type=Path,
        help="optional output path to write computed parameters when using --p1/--q1/--p2/--q2",
    )
    parser.add_argument(
        "--output-dir",
        default=Path("output_py"),
        type=Path,
        help="directory for generated images",
    )
    parser.add_argument("--dpi", default=128, type=int, help="base resolution for raster rendering")
    parser.add_argument("--xmin", default=-3.0, type=float, help="plane plot min real bound")
    parser.add_argument("--xmax", default=3.0, type=float, help="plane plot max real bound")
    parser.add_argument("--ymin", default=-3.0, type=float, help="plane plot min imag bound")
    parser.add_argument("--ymax", default=3.0, type=float, help="plane plot max imag bound")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    have_angles = None not in (args.p1, args.q1, args.p2, args.q2)
    if have_angles:
        a_params, b_params = compute_parameter_sequence(
            args.p1,
            args.q1,
            args.p2,
            args.q2,
            args.iterations,
            stop_on_failure=args.stop_on_failure,
        )
        if args.write_param_file is not None:
            write_param_file(args.write_param_file, a_params, b_params)
    else:
        a_params, b_params = read_param_file(args.param_file)

    bounds = (args.xmin, args.xmax, args.ymin, args.ymax)
    plane = render_plane_pullback(a_params, b_params, bounds=bounds, dpi=args.dpi)
    sphere = render_sphere_pullback(a_params, b_params, dpi=args.dpi)

    fig, axes = plt.subplots(1, 3, figsize=(15, 5))
    plot_parameter_trajectory(a_params, b_params, ax=axes[0])
    plot_plane_pullback(plane, ax=axes[1])
    plot_sphere_pullback(sphere, ax=axes[2])
    fig.suptitle(f"Medusa Pullback (n={len(a_params)} iterates)")
    fig.tight_layout()

    out_dir = args.output_dir
    save_figure(fig, out_dir / "medusa_overview.png")
    save_figure(fig, out_dir / "medusa_overview.pdf")
    plt.close(fig)

    fig_plane, ax_plane = plt.subplots(figsize=(7, 7))
    plot_plane_pullback(plane, ax=ax_plane)
    fig_plane.tight_layout()
    save_figure(fig_plane, out_dir / "medusa_plane.png")
    plt.close(fig_plane)

    fig_sphere, ax_sphere = plt.subplots(figsize=(7, 7))
    plot_sphere_pullback(sphere, ax=ax_sphere)
    fig_sphere.tight_layout()
    save_figure(fig_sphere, out_dir / "medusa_sphere.png")
    plt.close(fig_sphere)

    if have_angles:
        print(f"Computed {len(a_params)} parameter pairs via Python full-port engine")
    else:
        print(f"Loaded {len(a_params)} parameter pairs from {args.param_file}")
    print(f"Wrote output to {out_dir.resolve()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

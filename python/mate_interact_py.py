#!/usr/bin/env python3
"""Python equivalent of mate_interact (batch/CLI form)."""

from __future__ import annotations

import argparse
from pathlib import Path

from medusa_full import compute_parameter_sequence, write_param_file


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--p1", type=int, required=True)
    parser.add_argument("--q1", type=int, required=True)
    parser.add_argument("--p2", type=int, required=True)
    parser.add_argument("--q2", type=int, required=True)
    parser.add_argument("--iterations", type=int, default=35)
    parser.add_argument("--output", type=Path, default=Path("param_py.txt"))
    parser.add_argument(
        "--stop-on-failure",
        action="store_true",
        help="stop when one iterate returns failure (default: continue, like mate_interact)",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    a_vals, b_vals = compute_parameter_sequence(
        args.p1,
        args.q1,
        args.p2,
        args.q2,
        args.iterations,
        stop_on_failure=args.stop_on_failure,
    )
    write_param_file(args.output, a_vals, b_vals)
    print(f"wrote {len(a_vals)} parameter pairs to {args.output.resolve()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())


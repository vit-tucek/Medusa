"""Modern Python port of Medusa pullback rendering utilities.

This module ports the core computational pieces used by:
  - src/drawpullback.cc
  - src/draw_pb_sphere.cc
  - selected helper logic from src/medusa_B.cc
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re
from typing import Sequence

import matplotlib.pyplot as plt
from matplotlib.colors import ListedColormap
import numpy as np

MEDUSA_INFINITY = 1.0e100
DENOM_EPS = 1.0e-24

_CPLX_RE = re.compile(
    r"\(\s*([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\s*,\s*"
    r"([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)\s*\)"
)


@dataclass(frozen=True)
class OrbitData:
    """Orbit data for doubling map p/q -> 2p/q mod 1."""

    orbit: np.ndarray
    period: int
    preperiod: int


@dataclass(frozen=True)
class PreimageSymbol:
    """Kneading data entry (leg number + 0/1 symbol)."""

    legno: int
    symbol: int


@dataclass(frozen=True)
class PlanePullbackResult:
    """Result image for planar pullback drawing."""

    inside_unit_disk: np.ndarray
    bounds: tuple[float, float, float, float]


@dataclass(frozen=True)
class SpherePullbackResult:
    """Result image for sphere pullback drawing.

    classes use the C++ convention:
      0 = front (black)
      2 = hidden/back (gray)
      3 = outside/clear (white)
    """

    classes: np.ndarray


def _validate_param_lengths(a_params: Sequence[complex], b_params: Sequence[complex]) -> None:
    if len(a_params) != len(b_params):
        raise ValueError("a_params and b_params must have the same length")
    if len(a_params) == 0:
        raise ValueError("parameter sequence is empty")


def doubling_orbit(p: int, q: int) -> OrbitData:
    """Compute orbit/preperiod/period under doubling mod q."""

    if q <= 0:
        raise ValueError("q must be positive")
    p %= q
    seen: dict[int, int] = {}
    values: list[int] = []

    while p not in seen:
        seen[p] = len(values)
        values.append(p)
        p = (2 * p) % q

    preperiod = seen[p]
    period = len(values) - preperiod
    return OrbitData(orbit=np.asarray(values, dtype=np.int64), period=period, preperiod=preperiod)


def kneading_data(q: int, orbit: Sequence[int], period: int, preperiod: int) -> tuple[list[PreimageSymbol], list[PreimageSymbol]]:
    """Port of `find_kneading_data` from `medusa_B.cc`.

    Returns (`preim1`, `preim2`) in 0-based Python lists.
    """

    if q <= 0:
        raise ValueError("q must be positive")

    size = period + preperiod
    if size <= 0:
        return ([], [])
    if len(orbit) < size:
        raise ValueError("orbit length is shorter than period + preperiod")

    preim1 = [PreimageSymbol(-1, -1) for _ in range(size)]
    preim2 = [PreimageSymbol(-1, -1) for _ in range(size)]

    if preperiod == 0:
        preim1[0] = PreimageSymbol(period, int(2 * orbit[period - 1] >= q))

    for j in range(2, size + 1):
        preim1[j - 1] = PreimageSymbol(j - 1, int(2 * orbit[j - 2] >= q))

    if preperiod > 0:
        idx = preperiod  # (preperiod + 1) in 1-based indexing
        preim2[idx] = PreimageSymbol(period + preperiod, int(2 * orbit[preperiod + period - 1] >= q))

    return preim1, preim2


def medusa_angle_data(p1: int, q1: int, p2: int, q2: int) -> dict[str, object]:
    """Compute orbit + kneading helper data for mating angle input.

    This mirrors the setup used by `mate_interact` before iterative lifting.
    """

    if q1 <= 0 or q2 <= 0:
        raise ValueError("q1 and q2 must be positive")

    q = q1 * q2
    inner_p = q - (p1 * q2)  # constructor flips inner orientation in C++ code
    outer_p = p2 * q1

    inner = doubling_orbit(inner_p, q)
    outer = doubling_orbit(outer_p, q)

    i_preim1, i_preim2 = kneading_data(q, inner.orbit, inner.period, inner.preperiod)
    o_preim1, o_preim2 = kneading_data(q, outer.orbit, outer.period, outer.preperiod)

    return {
        "q": q,
        "inner_orbit": inner,
        "outer_orbit": outer,
        "inner_preim1": i_preim1,
        "inner_preim2": i_preim2,
        "outer_preim1": o_preim1,
        "outer_preim2": o_preim2,
    }


def read_param_file(path: str | Path) -> tuple[np.ndarray, np.ndarray]:
    """Read `param.txt`-style sequence of complex `a,b` values."""

    a_list: list[complex] = []
    b_list: list[complex] = []

    with Path(path).open("r", encoding="utf-8") as handle:
        for raw_line in handle:
            line = raw_line.strip()
            if not line:
                continue
            if line[0] == "*":
                break
            if line[0] == "/":
                line = line[1:].strip()
            matches = _CPLX_RE.findall(line)
            if len(matches) != 2:
                continue
            (ar, ai), (br, bi) = matches
            a_list.append(complex(float(ar), float(ai)))
            b_list.append(complex(float(br), float(bi)))

    a_params = np.asarray(a_list, dtype=np.complex128)
    b_params = np.asarray(b_list, dtype=np.complex128)
    _validate_param_lengths(a_params, b_params)
    return a_params, b_params


def canvas_size(dpi: int, bounds: tuple[float, float, float, float]) -> tuple[int, int]:
    """Replicate the historical raster dimensions from the C++ tools."""

    if dpi <= 0:
        raise ValueError("dpi must be positive")
    x_min, x_max, y_min, y_max = bounds
    xr = int(5 * dpi / 6) * 8
    yr = 8 * int(((y_max - y_min) / (x_max - x_min)) * xr / 8)
    if xr <= 0 or yr <= 0:
        raise ValueError("invalid bounds produced non-positive canvas size")
    return xr, yr


def _complex_grid(bounds: tuple[float, float, float, float], width: int, height: int) -> np.ndarray:
    x_min, x_max, y_min, y_max = bounds
    xs = x_min + (x_max - x_min) * (np.arange(width, dtype=np.float64) / width)
    ys = y_min + (y_max - y_min) * (np.arange(height, dtype=np.float64) / height)
    return xs[np.newaxis, :] + 1j * ys[:, np.newaxis]


def compose_pullback_map(z: np.ndarray, a_params: Sequence[complex], b_params: Sequence[complex]) -> np.ndarray:
    """Apply f_1 o ... o f_n as in the original draw routines."""

    _validate_param_lengths(a_params, b_params)
    out = np.asarray(z, dtype=np.complex128).copy()

    with np.errstate(over="ignore", divide="ignore", invalid="ignore"):
        for a, b in zip(reversed(a_params), reversed(b_params)):
            z2 = out * out
            denom = b * z2 - b + 1.0
            numer = a * z2 - a + 1.0
            safe = np.abs(denom) > DENOM_EPS
            out = np.where(safe, numer / denom, complex(MEDUSA_INFINITY, 0.0))
    return out


def render_plane_pullback(
    a_params: Sequence[complex],
    b_params: Sequence[complex],
    *,
    bounds: tuple[float, float, float, float] = (-3.0, 3.0, -3.0, 3.0),
    dpi: int = 144,
) -> PlanePullbackResult:
    """Render pullback image on the complex plane."""

    width, height = canvas_size(dpi, bounds)
    z = _complex_grid(bounds, width, height)
    mapped = compose_pullback_map(z, a_params, b_params)
    inside = np.abs(mapped) < 1.0
    return PlanePullbackResult(inside_unit_disk=inside, bounds=bounds)


def _composition_in_unit_disk(
    u: np.ndarray,
    v: np.ndarray,
    a_params: Sequence[complex],
    b_params: Sequence[complex],
) -> np.ndarray:
    _validate_param_lengths(a_params, b_params)

    u_out = np.asarray(u, dtype=np.complex128).copy()
    v_out = np.asarray(v, dtype=np.complex128).copy()

    with np.errstate(over="ignore", divide="ignore", invalid="ignore"):
        for idx, (a, b) in enumerate(zip(reversed(a_params), reversed(b_params)), start=1):
            u2 = u_out * u_out
            v2 = v_out * v_out
            diff = u2 - v2
            u_out = a * diff + v2
            v_out = b * diff + v2
            if idx % 4 == 0:
                scale = np.sqrt(np.abs(u_out) ** 2 + np.abs(v_out) ** 2)
                scale = np.where(scale > 0.0, scale, 1.0)
                u_out = u_out / scale
                v_out = v_out / scale

    return np.abs(u_out) ** 2 < np.abs(v_out) ** 2


def render_sphere_pullback(
    a_params: Sequence[complex],
    b_params: Sequence[complex],
    *,
    dpi: int = 144,
) -> SpherePullbackResult:
    """Render pullback classes on unit disk view of sphere projection."""

    bounds = (-1.0, 1.0, -1.0, 1.0)
    width, height = canvas_size(dpi, bounds)
    z = _complex_grid(bounds, width, height)

    r = np.abs(z) ** 2
    in_disk = r <= 1.0
    classes = np.full(z.shape, 3, dtype=np.uint8)
    if not np.any(in_disk):
        return SpherePullbackResult(classes=classes)

    z_flat = z[in_disk]
    sqrt_term = np.sqrt(np.maximum(0.0, 1.0 - np.abs(z_flat) ** 2))
    denom = (1.0 - np.imag(z_flat)).astype(np.complex128)

    u_front = sqrt_term + 1j * np.real(z_flat)
    u_hidden = -sqrt_term + 1j * np.real(z_flat)

    front_inside = _composition_in_unit_disk(u_front, denom, a_params, b_params)
    local = np.full(z_flat.shape, 3, dtype=np.uint8)
    local[front_inside] = 0

    remaining = ~front_inside
    if np.any(remaining):
        hidden_inside = _composition_in_unit_disk(
            u_hidden[remaining],
            denom[remaining],
            a_params,
            b_params,
        )
        local[remaining] = np.where(hidden_inside, 2, 3).astype(np.uint8)

    classes[in_disk] = local
    return SpherePullbackResult(classes=classes)


def plot_parameter_trajectory(
    a_params: Sequence[complex],
    b_params: Sequence[complex],
    *,
    ax: plt.Axes | None = None,
) -> plt.Axes:
    """Plot parameter trajectories (real/imag projections)."""

    _validate_param_lengths(a_params, b_params)
    if ax is None:
        _, ax = plt.subplots(figsize=(6, 5))

    a_arr = np.asarray(a_params)
    b_arr = np.asarray(b_params)
    ax.plot(a_arr.real, a_arr.imag, "-o", ms=2.5, lw=1.0, label="a_n")
    ax.plot(b_arr.real, b_arr.imag, "-o", ms=2.5, lw=1.0, label="b_n")
    ax.set_title("Parameter Trajectory")
    ax.set_xlabel("Re")
    ax.set_ylabel("Im")
    ax.grid(alpha=0.25)
    ax.legend(loc="best")
    return ax


def plot_plane_pullback(result: PlanePullbackResult, *, ax: plt.Axes | None = None) -> plt.Axes:
    """Display planar pullback result."""

    if ax is None:
        _, ax = plt.subplots(figsize=(6, 6))

    x_min, x_max, y_min, y_max = result.bounds
    image = np.where(result.inside_unit_disk, 0.0, 1.0)
    ax.imshow(
        image,
        cmap="gray",
        origin="lower",
        extent=(x_min, x_max, y_min, y_max),
        interpolation="nearest",
    )
    ax.set_title("Plane Pullback")
    ax.set_xlabel("Re(z)")
    ax.set_ylabel("Im(z)")
    return ax


def plot_sphere_pullback(result: SpherePullbackResult, *, ax: plt.Axes | None = None) -> plt.Axes:
    """Display sphere pullback classes with fixed black/gray/white palette."""

    if ax is None:
        _, ax = plt.subplots(figsize=(6, 6))

    idx = np.full(result.classes.shape, 2, dtype=np.uint8)  # white
    idx[result.classes == 2] = 1  # gray
    idx[result.classes == 0] = 0  # black

    cmap = ListedColormap(["#111111", "#8c8c8c", "#ffffff"])
    ax.imshow(idx, cmap=cmap, origin="lower", interpolation="nearest")
    ax.set_title("Sphere Pullback (Projected)")
    ax.set_xticks([])
    ax.set_yticks([])
    return ax


def save_figure(fig: plt.Figure, path: str | Path) -> None:
    """Save figure to disk, creating parent directories as needed."""

    out = Path(path)
    out.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out, bbox_inches="tight")


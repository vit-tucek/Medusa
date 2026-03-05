"""Full Python port of the Medusa lifting algorithm from 1998 C++ code.

This module ports the iterative parameter-lifting engine from:
  - src/medusa_A.cc
  - src/medusa_B.cc
  - src/mate_interact.cc
  - src/find_matings.cc
"""

from __future__ import annotations

from copy import deepcopy
from dataclasses import dataclass
import cmath
import math
from pathlib import Path
from typing import Generic, TypeVar

import numpy as np

EPSILON = 1e-15
MEDUSA_INFINITY = 1e100
INFINITY = 1e100

T = TypeVar("T")


class Corruption(Exception):
    """Raised when homotopy checks fail in the same way as the C++ code."""


class DynArray(Generic[T]):
    """Minimal 1-based dynamic array used by the original C++ sources."""

    def __init__(self, default: T):
        self._default = default
        self._data = [deepcopy(default)]  # index 0 unused
        self._max_assigned = 0

    def assign(self, value: T, index: int) -> None:
        if index < 1:
            raise IndexError("DynArray uses 1-based indexing")
        while len(self._data) <= index:
            self._data.append(deepcopy(self._default))
        self._data[index] = value
        if index > self._max_assigned:
            self._max_assigned = index

    def __getitem__(self, index: int) -> T:
        if index < 1 or index > self._max_assigned:
            raise IndexError(index)
        return self._data[index]

    def maxassigned(self) -> int:
        return self._max_assigned

    def resetmaxassigned(self) -> None:
        self._max_assigned = 0


@dataclass
class Preim:
    legno: int
    symbol: int


class MedusaComp:
    def __init__(self, z: complex):
        self.z = complex(z)
        self.next_component: MedusaComp | None = None

    def position(self) -> complex:
        return self.z


class Segment(MedusaComp):
    pass


class Joint(MedusaComp):
    def __init__(self, z: complex, i_leg: int, o_leg: int):
        super().__init__(z)
        self.i_leg = i_leg
        self.o_leg = o_leg


@dataclass
class Leg:
    attachment_p: Joint | None = None
    leg_p: Segment | None = None


@dataclass
class MatingEstimate:
    a: complex
    b: complex
    iterations_used: int


@dataclass
class FareySurveyRow:
    p1: int
    q1: int
    a: complex
    b: complex
    lifts: int


def _norm(z: complex) -> float:
    return z.real * z.real + z.imag * z.imag


def _arg(z: complex) -> float:
    return cmath.phase(z)


def _polar(r: float, theta: float) -> complex:
    return cmath.rect(r, theta)


def _numerical_equal(x: complex, y: complex) -> bool:
    magnitude = math.sqrt(_norm(x) + _norm(y))
    if magnitude <= EPSILON:
        return True
    return abs(x - y) / magnitude < EPSILON


def _lies_in_sector(a: complex, b: complex, x: complex, z: complex) -> bool:
    a_prime = (a - x) / (b - x) * _polar(1.0, math.pi)
    z_prime = (z - x) / (b - x) * _polar(1.0, math.pi)
    return _arg(z_prime) < _arg(a_prime) and _arg(z_prime) > -math.pi


def _not_lies_in_sector(a: complex, b: complex, x: complex, z: complex) -> bool:
    a_prime = (a - x) / (b - x) * _polar(1.0, math.pi)
    z_prime = (z - x) / (b - x) * _polar(1.0, math.pi)
    return _arg(z_prime) > _arg(a_prime) and _arg(z_prime) < math.pi


def _mobius_to_line(z: complex, a: complex, b: complex, star: complex) -> complex:
    if _numerical_equal(z, star):
        return complex(MEDUSA_INFINITY, 0.0)
    return ((z - a) / (z - star)) * ((b - star) / (b - a))


def _lies_in_circle(z: complex, a: complex, b: complex, star: complex, up_or_down: float) -> bool:
    z_prime = _mobius_to_line(z, a, b, star)
    return z_prime.imag * up_or_down >= 0


def _not_lies_in_circle(z: complex, a: complex, b: complex, star: complex, up_or_down: float) -> bool:
    z_prime = _mobius_to_line(z, a, b, star)
    return z_prime.imag * up_or_down <= 0


def _lies_in_hyperbola(a: complex, b: complex, z: complex) -> bool:
    hyp_const = min(a.real * a.imag, b.real * b.imag)
    if _numerical_equal(z, a) or _numerical_equal(z, b):
        return False
    if z.real * z.imag >= hyp_const:
        if _numerical_equal(z, 0):
            return True
        if abs(_arg(a / z)) <= math.pi / 2:
            return True
    return False


def _not_lies_in_hyperbola(a: complex, b: complex, z: complex) -> bool:
    hyp_const = min(a.real * a.imag, b.real * b.imag)
    if _numerical_equal(z, a) or _numerical_equal(z, b):
        return False
    if z.real * z.imag <= hyp_const:
        return True
    if _numerical_equal(z, 0):
        return True
    if abs(_arg(a / z)) >= math.pi / 2:
        return True
    return False


def _find_circle(a: complex, b: complex, star: complex) -> tuple[complex, float, complex]:
    a1, a2 = a.real, a.imag
    b1, b2 = b.real, b.imag
    c1, c2 = star.real, star.imag

    denom = a1 * b2 - a1 * c2 - a2 * b1 + a2 * c1 - c1 * b2 + c2 * b1
    if abs(denom) >= EPSILON:
        c_val = -(
            -a2 * b1 * b1
            + c2 * b1 * b1
            - c2 * a2 * a2
            - a2 * b2 * b2
            + c2 * c2 * a2
            - c1 * c1 * b2
            + a2 * c1 * c1
            - a1 * a1 * c2
            - b2 * c2 * c2
            + a1 * a1 * b2
            + a2 * a2 * b2
            + b2 * b2 * c2
        ) / denom
        d_val = (
            a1 * a1 * b1
            - a1 * a1 * c1
            + a2 * a2 * b1
            - a2 * a2 * c1
            - a1 * b1 * b1
            - a1 * b2 * b2
            + a1 * c1 * c1
            + a1 * c2 * c2
            - c1 * c1 * b1
            - c2 * c2 * b1
            + c1 * b1 * b1
            + c1 * b2 * b2
        ) / denom
        center = complex(-c_val / 2, -d_val / 2)
        radius = abs(center - a)
    else:
        center = complex(MEDUSA_INFINITY, 0.0)
        radius = MEDUSA_INFINITY

    average = (a + b + star) / 3
    return center, radius, average


def _get_midpoint(a: complex, b: complex) -> complex:
    a1, a2 = a.real, a.imag
    b1, b2 = b.real, b.imag
    hyp_const = min(a1 * a2, b1 * b2)
    if abs(a1 - b1) > abs(a2 - b2):
        return complex((a1 + b1) / 2, 2 * hyp_const / (a1 + b1))
    return complex(2 * hyp_const / (a2 + b2), (a2 + b2) / 2)


def _bulge_or_dip(a_prime: complex, b_prime: complex, lista_pts: DynArray[complex]) -> bool:
    if lista_pts.maxassigned() == 2:
        return False
    return _numerical_equal(lista_pts[2], a_prime) and _numerical_equal(lista_pts[3], b_prime)


def _order_points(points: DynArray[complex]) -> None:
    vals = [points[i] for i in range(1, points.maxassigned() + 1)]
    vals.sort(key=_arg)
    points.resetmaxassigned()
    for i, v in enumerate(vals, start=1):
        points.assign(v, i)


def _asymptote_rotate_angle(a: complex, b: complex) -> complex:
    temp = b if _numerical_equal(a, 0) else a
    z = cmath.sqrt(a * a - b * b).conjugate()
    if abs(z) > 0.0:
        z = z / abs(z)
    else:
        z = complex(1.0, 0.0)

    up = complex(0.0, 1.0)
    i = 0
    while (temp * z).real < 0 or (temp * z).imag <= 0:
        z *= up
        if i >= 4:
            raise Corruption()
        i += 1
    return z


def _find_intersections(
    a: complex,
    b: complex,
    star: complex,
    intersection_pts: DynArray[complex],
    lista_pts: DynArray[complex],
) -> int:
    a1, a2 = a.real, a.imag
    b1, b2 = b.real, b.imag
    c1, c2 = star.real, star.imag
    hyp_const = min(a1 * a2, b1 * b2)

    a_prime = -b1 * c2 + a2 * b1 + a1 * a2 - a2 * c1
    b_prime = a2 * (a2 * b2 + c2 * c2 - b2 * c2 + c1 * c1 - a2 * c2 - a1 * c1 + a1 * b1 - b1 * c1)
    c_prime = a2 * a2 * (-b2 * c1 + a1 * b2 + a1 * a2 - a1 * c2)

    ay_prime = -b2 * c1 + a1 * b2 + a2 * a1 - a1 * c2
    by_prime = a1 * (a1 * b1 + c1 * c1 - b1 * c1 + c2 * c2 - a1 * c1 - a2 * c2 + a2 * b2 - b2 * c2)
    cy_prime = a1 * a1 * (-b1 * c2 + a2 * b1 + a2 * a1 - a2 * c1)

    discriminant = b_prime * b_prime - 4 * a_prime * c_prime
    discriminanty = by_prime * by_prime - 4 * ay_prime * cy_prime

    intersection_pts.assign(a, 1)
    intersection_pts.assign(b, 2)
    lista_pts.assign(a, 1)
    lista_pts.assign(b, 2)

    if discriminant < 0.0 or discriminant >= MEDUSA_INFINITY:
        _order_points(intersection_pts)
        _order_points(lista_pts)
        return 2

    if discriminant > 0.0:
        x = ((-b_prime + math.sqrt(discriminant)) / (2 * a_prime))
        y1 = ((-by_prime + math.sqrt(discriminanty)) / (2 * ay_prime))
        y2 = ((-by_prime - math.sqrt(discriminanty)) / (2 * ay_prime))
        if abs(hyp_const - x * y1) > abs(hyp_const - x * y2):
            y = y2
            y2 = y1
            y1 = y
        if abs(x) > abs(y):
            root = complex(x, hyp_const / x)
        else:
            root = complex(hyp_const / y, y)
        if abs(_arg(a / root)) > math.pi / 2 or abs(_arg(b / root)) > math.pi / 2:
            _order_points(intersection_pts)
            _order_points(lista_pts)
            return 2

        if _numerical_equal(root, a) or _numerical_equal(root, b):
            _order_points(intersection_pts)
            lista_pts.assign(a, 3)
            lista_pts.assign(b, 4)
            _order_points(lista_pts)
            return 2

        intersection_pts.assign(root, 3)
        lista_pts.assign(root, 3)

        x = ((-b_prime - math.sqrt(discriminant)) / (2 * a_prime))
        if abs(x) > abs(y2):
            root = complex(x, hyp_const / x)
        else:
            root = complex(hyp_const / y, y)
        intersection_pts.assign(root, 4)
        lista_pts.assign(root, 4)

        _order_points(intersection_pts)
        _order_points(lista_pts)
        return 4

    if discriminant == 0.0:
        x = -b_prime / (2 * a_prime)
        root = complex(x, hyp_const / x)
        if abs(_arg(a / root)) > math.pi / 2 and abs(_arg(b / root)) > math.pi:
            _order_points(intersection_pts)
            _order_points(lista_pts)
            return 2

        intersection_pts.assign(root, 3)
        lista_pts.assign(root, 3)
        lista_pts.assign(root, 4)
        _order_points(intersection_pts)
        _order_points(lista_pts)
        return 3

    _order_points(intersection_pts)
    _order_points(lista_pts)
    return 2


def _sign(x: float) -> int:
    if x > 0:
        return 1
    if x < 0:
        return -1
    return 0


def is_homotopic(a: complex, b: complex, star: complex, distinguished_pts: DynArray[complex]) -> tuple[bool, complex]:
    """Port of `is_homotopic` from medusa_A.cc."""

    length = distinguished_pts.maxassigned()
    new_point = 0j

    if abs(b) > EPSILON:
        if abs(_arg(a / b)) > math.pi / 2 and (abs(b.real) > EPSILON or abs(b.imag) > EPSILON):
            raise Corruption()

    if _numerical_equal(a, b):
        return True, new_point

    rotation = _asymptote_rotate_angle(a, b)
    a *= rotation
    b *= rotation
    star *= rotation

    intersection_pts = DynArray[complex](0j)
    intersection_pts.assign(a, 1)
    intersection_pts.assign(b, 2)
    _order_points(intersection_pts)
    a = intersection_pts[1]
    b = intersection_pts[2]

    center, _radius, average = _find_circle(a, b, star)
    up_or_down = _mobius_to_line(average, a, b, star).imag

    if abs(a.real * a.imag) <= EPSILON or abs(b.real * b.imag) <= EPSILON:
        if not (abs(center) < MEDUSA_INFINITY):
            if not (abs(star) < MEDUSA_INFINITY):
                return True, new_point
        for point_i in range(1, length + 1):
            z = distinguished_pts[point_i] * rotation
            if not _numerical_equal(z, a) and not _numerical_equal(z, b):
                if _lies_in_circle(z, a, b, star, up_or_down) and (((z - a) / (b - a)).imag * ((star - a) / (b - a)).imag < 0):
                    new_point = (a + b) / 2
                    new_point /= rotation
                    return False, new_point
        return True, new_point

    if not (abs(center) < MEDUSA_INFINITY):
        if (not (abs(star) < MEDUSA_INFINITY)) or (
            ((star - a) / ((b - a) * 1j)).imag * ((b - a) / ((b - a) * 1j)).imag < 0
            or ((star - b) / ((b - a) * 1j)).imag * ((a - b) / ((b - a) * 1j)).imag < 0
        ):
            for point_i in range(1, length + 1):
                z = distinguished_pts[point_i] * rotation
                if not _numerical_equal(z, a) and not _numerical_equal(z, b):
                    if _lies_in_hyperbola(a, b, z) and (((z - a) / (b - a)).imag * ((0 - a) / (b - a)).imag >= 0):
                        new_point = _get_midpoint(a, b)
                        new_point /= rotation
                        return False, new_point
        else:
            for point_i in range(1, length + 1):
                z = distinguished_pts[point_i] * rotation
                if not _numerical_equal(z, a) and not _numerical_equal(z, b):
                    if _not_lies_in_hyperbola(a, b, z) and (((z - a) / (b - a)).imag * ((0 - a) / (b - a)).imag >= 0):
                        new_point = _get_midpoint(a, b)
                        new_point /= rotation
                        return False, new_point
        return True, new_point

    lista_pts = DynArray[complex](0j)
    num_intersections = _find_intersections(a, b, star, intersection_pts, lista_pts)
    if num_intersections == 2:
        for point_i in range(1, length + 1):
            z = distinguished_pts[point_i] * rotation
            if not _numerical_equal(z, a) and not _numerical_equal(z, b):
                if _lies_in_sector(b, a, center, star):
                    if _bulge_or_dip(a, b, lista_pts):
                        if _not_lies_in_circle(z, a, b, star, up_or_down) and _lies_in_hyperbola(a, b, z) and _lies_in_sector(a, b, center, z):
                            new_point = _get_midpoint(a, b)
                            new_point /= rotation
                            return False, new_point
                    elif _lies_in_circle(z, a, b, star, up_or_down) and _not_lies_in_hyperbola(a, b, z):
                        new_point = _get_midpoint(a, b)
                        new_point /= rotation
                        return False, new_point
                else:
                    if _bulge_or_dip(a, b, lista_pts):
                        if _not_lies_in_circle(z, a, b, star, up_or_down) and (
                            _not_lies_in_hyperbola(a, b, z) or _not_lies_in_sector(a, b, center, z)
                        ):
                            new_point = _get_midpoint(a, b)
                            new_point /= rotation
                            return False, new_point
                    elif _lies_in_circle(z, a, b, star, up_or_down) and _lies_in_hyperbola(a, b, z):
                        new_point = _get_midpoint(a, b)
                        new_point /= rotation
                        return False, new_point
        return True, new_point

    i_i = 1
    while a != intersection_pts[i_i]:
        i_i += 1
    while b != intersection_pts[i_i]:
        a_prime = intersection_pts[i_i]
        b_prime = intersection_pts[i_i + 1]
        for point_i in range(1, length + 1):
            z = distinguished_pts[point_i] * rotation
            # NOTE: preserve original code's boolean form (||) for behavior compatibility.
            if (not _numerical_equal(z, a_prime)) or (not _numerical_equal(z, b_prime)):
                if _lies_in_sector(b_prime, a_prime, center, star):
                    if _bulge_or_dip(a_prime, b_prime, lista_pts):
                        if _not_lies_in_circle(z, a, b, star, up_or_down) and _lies_in_hyperbola(a_prime, b_prime, z) and _lies_in_sector(a_prime, b_prime, center, z):
                            new_point = _get_midpoint(a_prime, b_prime)
                            new_point /= rotation
                            return False, new_point
                    elif _lies_in_circle(z, a, b, star, up_or_down) and _not_lies_in_hyperbola(a_prime, b_prime, z) and _lies_in_sector(a_prime, b_prime, center, z):
                        new_point = _get_midpoint(a_prime, b_prime)
                        new_point /= rotation
                        return False, new_point
                else:
                    if _bulge_or_dip(a_prime, b_prime, lista_pts):
                        if _not_lies_in_circle(z, a, b, star, up_or_down) and (
                            _not_lies_in_hyperbola(a_prime, b_prime, z) or _not_lies_in_sector(a_prime, b_prime, center, z)
                        ):
                            new_point = _get_midpoint(a_prime, b_prime)
                            new_point /= rotation
                            return False, new_point
                    elif (
                        _lies_in_circle(z, a, b, star, up_or_down) and _lies_in_hyperbola(a_prime, b_prime, z)
                    ) or (
                        _lies_in_circle(z, a, b, star, up_or_down) and _not_lies_in_sector(a_prime, b_prime, center, z)
                    ):
                        new_point = _get_midpoint(a_prime, b_prime)
                        new_point /= rotation
                        return False, new_point
        i_i += 1

    return True, new_point


def can_prune(a: complex, b: complex, c: complex, star: complex, distinguished_pts: DynArray[complex]) -> bool:
    """Port of `can_prune` from medusa_A.cc."""

    if _numerical_equal(a, c) or _numerical_equal(b, c):
        return True

    star_at_inf = _norm(star) >= MEDUSA_INFINITY
    if not star_at_inf:
        a_mob = 1 / (a - star)
        b_mob = 1 / (b - star)
        c_mob = 1 / (c - star)
    else:
        a_mob = a
        b_mob = b
        c_mob = c

    for i in range(1, distinguished_pts.maxassigned() + 1):
        z = distinguished_pts[i]
        if not _numerical_equal(z, a) and not _numerical_equal(z, b) and not _numerical_equal(z, c) and not _numerical_equal(z, star):
            z_mob = z if star_at_inf else 1 / (z - star)
            if (((z_mob - a_mob) / (b_mob - a_mob)).imag * _sign(((c_mob - a_mob) / (b_mob - a_mob)).imag) > -1e-3):
                if (((z_mob - b_mob) / (c_mob - b_mob)).imag * _sign(((a_mob - b_mob) / (c_mob - b_mob)).imag) > -1e-3):
                    if (((z_mob - c_mob) / (a_mob - c_mob)).imag * _sign(((b_mob - c_mob) / (a_mob - c_mob)).imag) > -1e-3):
                        return False
    return True


def _reciprok(z: complex) -> complex:
    if _norm(z) > EPSILON * EPSILON:
        return 1 / z
    return complex(INFINITY, 0.0)


def _preim_list(size: int) -> list[Preim]:
    return [Preim(-1, -1) for _ in range(size + 1)]  # 1-based


class Medusa:
    """Port of class `Medusa` from `medusa_B.cc`."""

    def __init__(self, p1: int, p2: int, q: int):
        self.a = complex(1.0, 0.0)
        self.b = complex(0.0, 0.0)

        p1 = q - p1  # inner leg orientation reversed in original code

        table1 = DynArray[int](0)
        self.inner_period, self.inner_preperiod = self.findorbit(p1, q, table1)
        table2 = DynArray[int](0)
        self.outer_period, self.outer_preperiod = self.findorbit(p2, q, table2)

        inner_size = self.inner_period + self.inner_preperiod
        outer_size = self.outer_period + self.outer_preperiod
        self.i_preim1 = _preim_list(inner_size)
        self.i_preim2 = _preim_list(inner_size)
        self.o_preim1 = _preim_list(outer_size)
        self.o_preim2 = _preim_list(outer_size)

        self.find_kneading_data(q, self.inner_period, self.inner_preperiod, table1, self.i_preim1, self.i_preim2)
        self.find_kneading_data(q, self.outer_period, self.outer_preperiod, table2, self.o_preim1, self.o_preim2)

        self.init_body(table1, table2, q)

    @property
    def param_a(self) -> complex:
        return self.a

    @property
    def param_b(self) -> complex:
        return self.b

    def findorbit(self, p: int, q: int, table: DynArray[int]) -> tuple[int, int]:
        k = 1
        inlist = False
        period = 0
        preperiod = 0
        while not inlist:
            table.assign(p, k)
            p *= 2
            if p >= q:
                p -= q
            for j in range(1, k + 1):
                if table[j] == p:
                    preperiod = j - 1
                    inlist = True
                    period = k - preperiod
                    break
            k += 1
        return period, preperiod

    def find_kneading_data(
        self,
        q: int,
        period: int,
        preperiod: int,
        table: DynArray[int],
        preim1: list[Preim],
        preim2: list[Preim],
    ) -> None:
        if preperiod == 0:
            preim1[1] = Preim(period, 1 if 2 * table[period] >= q else 0)

        for j in range(2, period + preperiod + 1):
            preim1[j] = Preim(j - 1, 1 if 2 * table[j - 1] >= q else 0)

        for j in range(1, period + preperiod + 1):
            preim2[j] = Preim(-1, -1)

        if preperiod > 0:
            preim2[preperiod + 1] = Preim(period + preperiod, 1 if 2 * table[preperiod + period] >= q else 0)

    def find_succ(self, p: int, q: int, table: DynArray[int]) -> tuple[bool, int, int]:
        pp = q
        legno = 0
        for j in range(1, table.maxassigned() + 1):
            if table[j] > p and table[j] < pp:
                pp = table[j]
                legno = j
        ok = pp != q
        return ok, pp, legno

    def init_body(self, table1: DynArray[int], table2: DynArray[int], q: int) -> None:
        inner_n = self.inner_period + self.inner_preperiod
        outer_n = self.outer_period + self.outer_preperiod
        self.i_legs = [Leg() for _ in range(inner_n)]
        self.o_legs = [Leg() for _ in range(outer_n)]

        self.body_p = Segment(1)
        act_p: MedusaComp = self.body_p
        p = -1

        i_ok, p1, i_legno = self.find_succ(p, q, table1)
        o_ok, p2, o_legno = self.find_succ(p, q, table2)

        while i_ok or o_ok:
            if i_ok and o_ok and p1 == p2:
                p = p1
                act_p.next_component = Joint(cmath.exp(complex(0.0, 2.0 * math.pi * p / q)), i_legno, o_legno)
                act_p = act_p.next_component
                self.i_legs[i_legno - 1].attachment_p = act_p
                self.o_legs[o_legno - 1].attachment_p = act_p
            elif (i_ok and p1 < p2) or (not o_ok):
                p = p1
                act_p.next_component = Joint(cmath.exp(complex(0.0, 2.0 * math.pi * p / q)), i_legno, -1)
                act_p = act_p.next_component
                self.i_legs[i_legno - 1].attachment_p = act_p
            else:
                p = p2
                act_p.next_component = Joint(cmath.exp(complex(0.0, 2.0 * math.pi * p / q)), -1, o_legno)
                act_p = act_p.next_component
                self.o_legs[o_legno - 1].attachment_p = act_p

            i_ok, p1, i_legno = self.find_succ(p, q, table1)
            o_ok, p2, o_legno = self.find_succ(p, q, table2)

        act_p.next_component = self.body_p

        for k in range(1, inner_n):
            self.i_legs[k].leg_p = Segment(0.5 * cmath.exp(complex(0.0, 2.0 * math.pi * table1[k + 1] / q)))
        self.i_legs[0].leg_p = Segment(0)

        for k in range(1, outer_n):
            self.o_legs[k].leg_p = Segment(2.0 * cmath.exp(complex(0.0, 2.0 * math.pi * table2[k + 1] / q)))
        self.o_legs[0].leg_p = Segment(INFINITY * cmath.exp(complex(0.0, 2.0 * math.pi * table2[1] / q)))

    def invf(self, refpos: complex, u: complex) -> tuple[complex, complex]:
        if _norm(u * self.b - self.a) < EPSILON * EPSILON:
            if refpos.real > 0:
                return complex(INFINITY, 0.0), complex(-INFINITY, 0.0)
            return complex(-INFINITY, 0.0), complex(INFINITY, 0.0)

        tmp = ((self.b - 1) * u + 1 - self.a) / (self.b * u - self.a)
        z1 = cmath.sqrt(tmp)
        z2 = -z1
        try:
            ang2 = abs(_arg(z2 / refpos))
        except ZeroDivisionError:
            ang2 = math.inf
        try:
            ang1 = abs(_arg(z1 / refpos))
        except ZeroDivisionError:
            ang1 = math.inf
        if ang2 < ang1:
            z1, z2 = z2, z1
        return z1, z2

    def pullback_leg(
        self,
        from_leg: int,
        to_leg: int,
        old_legs: list[Leg],
        new_legs: list[Leg],
        startpos: complex,
        attach_p: Joint,
        dist_pts: DynArray[complex],
    ) -> None:
        new_legs[to_leg - 1].attachment_p = attach_p
        pos, _dummy = self.invf(startpos, old_legs[from_leg - 1].leg_p.position())
        to_p = Segment(pos)
        from_p = old_legs[from_leg - 1].leg_p
        new_legs[to_leg - 1].leg_p = to_p
        refpos = pos

        while from_p.next_component is not None:
            from_p = from_p.next_component  # type: ignore[assignment]
            pos, _dummy = self.invf(refpos, from_p.position())
            to_p.next_component = Segment(pos)
            refpos = pos
            to_p = to_p.next_component

        dist_pts.assign(to_p.position(), dist_pts.maxassigned() + 1)
        to_p.next_component = None

    def pullback(self, dist_pts: DynArray[complex]) -> None:
        dist_pts.resetmaxassigned()

        body_A_p = Segment(1)
        body_B_p = Segment(-1)
        A_p: MedusaComp = body_A_p
        B_p: MedusaComp = body_B_p
        old_p = self.body_p.next_component

        new_i_legs = [Leg() for _ in range(self.inner_preperiod + self.inner_period)]
        new_o_legs = [Leg() for _ in range(self.outer_preperiod + self.outer_period)]

        while old_p is not self.body_p:
            A_pos, B_pos = self.invf(A_p.position(), old_p.position())
            old_is_segment = isinstance(old_p, Segment)
            old_joint = old_p if isinstance(old_p, Joint) else None

            if old_is_segment or (
                old_joint is not None
                and (
                    old_joint.o_leg == -1
                    or (
                        self.o_preim1[old_joint.o_leg].symbol != 0 and self.o_preim2[old_joint.o_leg].symbol != 0
                    )
                )
                and (
                    old_joint.i_leg == -1
                    or (
                        self.i_preim1[old_joint.i_leg].symbol != 0 and self.i_preim2[old_joint.i_leg].symbol != 0
                    )
                )
            ):
                A_p.next_component = Segment(A_pos)
            else:
                if old_joint.i_leg == -1:
                    newilegno = -1
                elif self.i_preim1[old_joint.i_leg].symbol == 0:
                    newilegno = self.i_preim1[old_joint.i_leg].legno
                elif self.i_preim2[old_joint.i_leg].symbol == 0:
                    newilegno = self.i_preim2[old_joint.i_leg].legno
                else:
                    newilegno = -1

                if old_joint.o_leg == -1:
                    newolegno = -1
                elif self.o_preim1[old_joint.o_leg].symbol == 0:
                    newolegno = self.o_preim1[old_joint.o_leg].legno
                elif self.o_preim2[old_joint.o_leg].symbol == 0:
                    newolegno = self.o_preim2[old_joint.o_leg].legno
                else:
                    newolegno = -1

                A_p.next_component = Joint(A_pos, newilegno, newolegno)
                if newilegno != -1:
                    self.pullback_leg(
                        old_joint.i_leg,
                        newilegno,
                        self.i_legs,
                        new_i_legs,
                        A_pos,
                        A_p.next_component,  # type: ignore[arg-type]
                        dist_pts,
                    )
                if newolegno != -1:
                    self.pullback_leg(
                        old_joint.o_leg,
                        newolegno,
                        self.o_legs,
                        new_o_legs,
                        A_pos,
                        A_p.next_component,  # type: ignore[arg-type]
                        dist_pts,
                    )

            if old_is_segment or (
                old_joint is not None
                and (
                    old_joint.o_leg == -1
                    or (
                        self.o_preim1[old_joint.o_leg].symbol != 1 and self.o_preim2[old_joint.o_leg].symbol != 1
                    )
                )
                and (
                    old_joint.i_leg == -1
                    or (
                        self.i_preim1[old_joint.i_leg].symbol != 1 and self.i_preim2[old_joint.i_leg].symbol != 1
                    )
                )
            ):
                B_p.next_component = Segment(B_pos)
            else:
                if old_joint.i_leg == -1:
                    newilegno = -1
                elif self.i_preim1[old_joint.i_leg].symbol == 1:
                    newilegno = self.i_preim1[old_joint.i_leg].legno
                elif self.i_preim2[old_joint.i_leg].symbol == 1:
                    newilegno = self.i_preim2[old_joint.i_leg].legno
                else:
                    newilegno = -1

                if old_joint.o_leg == -1:
                    newolegno = -1
                elif self.o_preim1[old_joint.o_leg].symbol == 1:
                    newolegno = self.o_preim1[old_joint.o_leg].legno
                elif self.o_preim2[old_joint.o_leg].symbol == 1:
                    newolegno = self.o_preim2[old_joint.o_leg].legno
                else:
                    newolegno = -1

                B_p.next_component = Joint(B_pos, newilegno, newolegno)
                if newilegno != -1:
                    self.pullback_leg(
                        old_joint.i_leg,
                        newilegno,
                        self.i_legs,
                        new_i_legs,
                        B_pos,
                        B_p.next_component,  # type: ignore[arg-type]
                        dist_pts,
                    )
                if newolegno != -1:
                    self.pullback_leg(
                        old_joint.o_leg,
                        newolegno,
                        self.o_legs,
                        new_o_legs,
                        B_pos,
                        B_p.next_component,  # type: ignore[arg-type]
                        dist_pts,
                    )

            old_p = old_p.next_component
            A_p = A_p.next_component
            B_p = B_p.next_component

        self.body_p = body_A_p
        A_p.next_component = body_B_p
        B_p.next_component = body_A_p
        self.i_legs = new_i_legs
        self.o_legs = new_o_legs

    def find_params_and_cvs(self) -> tuple[complex, complex]:
        ptr = self.o_legs[0].leg_p
        while ptr.next_component is not None:
            ptr = ptr.next_component
        omega1 = ptr.position()

        ptr = self.i_legs[0].leg_p
        while ptr.next_component is not None:
            ptr = ptr.next_component
        omega2 = ptr.position()

        self.b = (omega2 - 1) / (omega2 - omega1)
        self.a = omega1 * self.b
        return omega1, omega2

    def rectify(self, dist_pts: DynArray[complex], omega1: complex, omega2: complex) -> bool:
        try:
            M = 4000
            added_parts = 0

            ptr = self.body_p
            while True:
                ok, mid_pos = is_homotopic(ptr.position(), ptr.next_component.position(), omega1, dist_pts)
                while not ok and added_parts < M:
                    tmp_p = Segment(mid_pos)
                    tmp_p.next_component = ptr.next_component
                    ptr.next_component = tmp_p
                    added_parts += 1
                    ok, mid_pos = is_homotopic(ptr.position(), ptr.next_component.position(), omega1, dist_pts)
                ptr = ptr.next_component
                if ptr is self.body_p:
                    break

            for j in range(self.inner_period + self.inner_preperiod):
                att_pos = self.i_legs[j].attachment_p.position()
                ok, mid_pos = is_homotopic(att_pos, self.i_legs[j].leg_p.position(), omega1, dist_pts)
                while not ok and added_parts < M:
                    tmp_p = Segment(mid_pos)
                    tmp_p.next_component = self.i_legs[j].leg_p
                    self.i_legs[j].leg_p = tmp_p
                    added_parts += 1
                    ok, mid_pos = is_homotopic(att_pos, self.i_legs[j].leg_p.position(), omega1, dist_pts)

                ptr = self.i_legs[j].leg_p
                while ptr.next_component is not None:
                    ok, mid_pos = is_homotopic(ptr.position(), ptr.next_component.position(), omega1, dist_pts)
                    while not ok and added_parts < M:
                        tmp_p = Segment(mid_pos)
                        tmp_p.next_component = ptr.next_component
                        ptr.next_component = tmp_p
                        added_parts += 1
                        ok, mid_pos = is_homotopic(ptr.position(), ptr.next_component.position(), omega1, dist_pts)
                    ptr = ptr.next_component

            for j in range(1, dist_pts.maxassigned() + 1):
                dist_pts.assign(_reciprok(dist_pts[j]), j)

            for j in range(self.outer_period + self.outer_preperiod):
                att_pos = _reciprok(self.o_legs[j].attachment_p.position())
                ok, mid_pos = is_homotopic(att_pos, _reciprok(self.o_legs[j].leg_p.position()), _reciprok(omega2), dist_pts)
                while not ok and added_parts < M:
                    tmp_p = Segment(_reciprok(mid_pos))
                    tmp_p.next_component = self.o_legs[j].leg_p
                    self.o_legs[j].leg_p = tmp_p
                    added_parts += 1
                    ok, mid_pos = is_homotopic(att_pos, _reciprok(self.o_legs[j].leg_p.position()), _reciprok(omega2), dist_pts)

                ptr = self.o_legs[j].leg_p
                while ptr.next_component is not None:
                    ok, mid_pos = is_homotopic(_reciprok(ptr.position()), _reciprok(ptr.next_component.position()), _reciprok(omega2), dist_pts)
                    while not ok and added_parts < M:
                        tmp_p = Segment(_reciprok(mid_pos))
                        tmp_p.next_component = ptr.next_component
                        ptr.next_component = tmp_p
                        added_parts += 1
                        ok, mid_pos = is_homotopic(
                            _reciprok(ptr.position()),
                            _reciprok(ptr.next_component.position()),
                            _reciprok(omega2),
                            dist_pts,
                        )
                    ptr = ptr.next_component

            return added_parts < M
        except Corruption:
            return False

    def prune_legs(self, nooflegs: int, legtable: list[Leg], omega: complex, dist_pts: DynArray[complex]) -> None:
        for j in range(nooflegs):
            att_pos = legtable[j].attachment_p.position()
            ptr = legtable[j].leg_p
            while ptr.next_component is not None and can_prune(att_pos, ptr.position(), ptr.next_component.position(), omega, dist_pts):
                legtable[j].leg_p = ptr.next_component
                ptr = legtable[j].leg_p

            while ptr.next_component is not None and ptr.next_component.next_component is not None:
                while ptr.next_component.next_component is not None and can_prune(
                    ptr.position(),
                    ptr.next_component.position(),
                    ptr.next_component.next_component.position(),
                    omega,
                    dist_pts,
                ):
                    ptr.next_component = ptr.next_component.next_component
                ptr = ptr.next_component

    def prune(self, dist_pts: DynArray[complex], omega1: complex, omega2: complex) -> None:
        ptr = self.body_p
        while True:
            while (
                ptr.next_component is not self.body_p
                and isinstance(ptr.next_component, Segment)
                and can_prune(ptr.position(), ptr.next_component.position(), ptr.next_component.next_component.position(), omega1, dist_pts)
            ):
                ptr.next_component = ptr.next_component.next_component
            ptr = ptr.next_component
            if ptr is self.body_p:
                break

        self.prune_legs(self.inner_period + self.inner_preperiod, self.i_legs, omega1, dist_pts)
        self.prune_legs(self.outer_period + self.outer_preperiod, self.o_legs, omega2, dist_pts)

    def iterate(self) -> bool:
        dist_pts = DynArray[complex](0j)
        self.pullback(dist_pts)
        omega1, omega2 = self.find_params_and_cvs()
        ok = self.rectify(dist_pts, omega1, omega2)
        if ok:
            self.prune(dist_pts, omega1, omega2)
        return ok


def compute_parameter_sequence(
    p1: int,
    q1: int,
    p2: int,
    q2: int,
    iterations: int,
    *,
    stop_on_failure: bool = False,
) -> tuple[np.ndarray, np.ndarray]:
    """Python equivalent of `mate_interact` batch operation."""

    if iterations <= 0:
        raise ValueError("iterations must be > 0")
    medusa = Medusa(p1 * q2, p2 * q1, q1 * q2)

    a_values: list[complex] = []
    b_values: list[complex] = []
    for _ in range(iterations):
        ok = medusa.iterate()
        if not ok and stop_on_failure:
            break
        a_values.append(medusa.param_a)
        b_values.append(medusa.param_b)
    return np.asarray(a_values, dtype=np.complex128), np.asarray(b_values, dtype=np.complex128)


def write_param_file(path: str | Path, a_values: np.ndarray, b_values: np.ndarray) -> None:
    if len(a_values) != len(b_values):
        raise ValueError("a_values and b_values length mismatch")
    out = Path(path)
    out.parent.mkdir(parents=True, exist_ok=True)
    with out.open("w", encoding="utf-8") as handle:
        for a, b in zip(a_values, b_values):
            handle.write(f"/ ({a.real:.12g},{a.imag:.12g}) ({b.real:.12g},{b.imag:.12g})\n")
        handle.write("*\n")


def estimate_param(p1: int, p2: int, q: int, max_it: int = 50) -> MatingEstimate:
    """Port of `estimparam` from `find_matings.cc`."""

    medusa = Medusa(p1, p2, q)
    a_params: list[complex] = []
    b_params: list[complex] = []
    no_it = 1
    ok = True
    while ok and no_it <= max_it:
        ok = medusa.iterate()
        a_params.append(medusa.param_a)
        b_params.append(medusa.param_b)
        no_it += 1

    best_entry = 1
    min_diff = 1e10
    for k in range(1, len(a_params)):
        diff = _norm(a_params[k] - a_params[k - 1]) + _norm(b_params[k] - b_params[k - 1])
        if diff < min_diff:
            min_diff = diff
            best_entry = k + 1  # back to 1-based

    return MatingEstimate(a=a_params[best_entry - 1], b=b_params[best_entry - 1], iterations_used=best_entry)


def farey_survey(depth: int, base_p: int = 3, base_q: int = 7, max_it: int = 50) -> list[FareySurveyRow]:
    """Port of `fareysurvey` recursion from `find_matings.cc`."""

    rows: list[FareySurveyRow] = []

    def _walk(p1: int, q1: int, p2: int, q2: int, d: int) -> None:
        if d == 0:
            est = estimate_param(p1 * base_q, q1 * base_p, q1 * base_q, max_it=max_it)
            rows.append(FareySurveyRow(p1=p1, q1=q1, a=est.a, b=est.b, lifts=est.iterations_used))
            return
        _walk(p1, q1, p1 + p2, q1 + q2, d - 1)
        _walk(p1 + p2, q1 + q2, p2, q2, d - 1)

    _walk(1, 7, 1, 3, depth)
    return rows

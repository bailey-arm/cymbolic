#!/usr/bin/env python3
"""
Generate synthetic regression datasets for testing/benchmarking cymbolic.

Usage:
    python tools/gen_dataset.py --target "sin(x) + 0.5*x**2" --n 1000 --out data/dataset.csv
    python tools/gen_dataset.py --target "x*y - log(abs(z)+1)"  --vars x y z --n 5000 --out data/multi.csv
"""

import argparse
import math
import csv
import random


# Safe math namespace for eval()
MATH_ENV = {name: getattr(math, name) for name in dir(math) if not name.startswith("_")}
MATH_ENV["abs"] = abs


def generate(target_expr: str, var_names: list[str], n: int, xmin: float, xmax: float, noise: float, seed: int):
    rng = random.Random(seed)
    rows = []
    for _ in range(n):
        env = {v: rng.uniform(xmin, xmax) for v in var_names}
        try:
            y = eval(target_expr, {"__builtins__": {}}, {**MATH_ENV, **env})
        except (ZeroDivisionError, ValueError):
            continue
        if noise > 0:
            y += rng.gauss(0, noise)
        rows.append({**env, "y": y})
    return rows


def main():
    parser = argparse.ArgumentParser(description="Generate synthetic datasets for cymbolic")
    parser.add_argument("--target", required=True,   help="Python math expression, e.g. 'sin(x) + x**2'")
    parser.add_argument("--vars",   nargs="+", default=["x"], help="Variable names (default: x)")
    parser.add_argument("--n",      type=int,   default=1000,  help="Number of samples")
    parser.add_argument("--xmin",   type=float, default=-5.0,  help="Lower bound for each variable")
    parser.add_argument("--xmax",   type=float, default=5.0,   help="Upper bound for each variable")
    parser.add_argument("--noise",  type=float, default=0.0,   help="Gaussian noise std-dev (0 = no noise)")
    parser.add_argument("--seed",   type=int,   default=42,    help="Random seed")
    parser.add_argument("--out",    required=True,             help="Output CSV path")
    args = parser.parse_args()

    rows = generate(args.target, args.vars, args.n, args.xmin, args.xmax, args.noise, args.seed)

    with open(args.out, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=args.vars + ["y"])
        writer.writeheader()
        writer.writerows(rows)

    print(f"Wrote {len(rows)} rows to {args.out}  (target: {args.target})")


if __name__ == "__main__":
    main()

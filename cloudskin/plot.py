#!/usr/bin/env python3
"""
Plot aggregate CPU utilization from the CSV produced by record_cpu_per_core.py.

Aggregate is:
  - avg: mean across all cores (0–100%)
  - total: sum across cores (0–100 * Ncores)

Requires: pandas, matplotlib
"""

import argparse
import pandas as pd
import matplotlib.pyplot as plt

def add_ladder(ax, coords, x_min=None, x_max=None, where="post"):
    """
    Add a red ladder/step line on a 2nd Y axis.

    coords: list of (x, y) where x can be:
      - pandas.Timestamp / datetime
      - or a string parseable by pd.to_datetime (e.g. "2025-12-23 12:00:00")
      - or a float (if your x-axis is numeric)

    Interpretation (default where="post"):
      y is held constant from x_i until x_{i+1}, then jumps.
    """
    ax2 = ax.twinx()

    # Parse & sort coordinates by x
    xs, ys = zip(*coords)
    xs = pd.to_datetime(list(xs), errors="ignore")
    ladder = sorted(zip(xs, ys), key=lambda t: t[0])
    xs, ys = zip(*ladder)

    xs = list(xs)
    ys = list(ys)

    # Extend to cover the whole plot range so it keeps its last value
    if x_min is not None and xs[0] > x_min:
        xs = [x_min] + xs
        ys = [ys[0]] + ys
    if x_max is not None and xs[-1] < x_max:
        xs = xs + [x_max]
        ys = ys + [ys[-1]]

    ax2.step(xs, ys, where=where, color="red", linewidth=2, label="ladder")
    ax2.set_ylabel("# Concurrent Jobs")
    ax2.tick_params(axis="y", colors="red")
    ax2.spines["right"].set_color("red")
    return ax2

import pandas as pd

def add_text_box(ax, x, y, text, *,
                 fontsize=10,
                 ha="left",
                 va="center",
                 zorder=10):
    """
    Add a white text box with black border at (x, y) in data coordinates.

    x:
      - pandas.Timestamp / datetime
      - string parseable by pd.to_datetime
      - or numeric
    y:
      - numeric (data coordinates of the axis)

    text:
      - multiline supported using '\\n'
    """
    # Allow datetime strings
    try:
        x = pd.to_datetime(x)
    except Exception:
        pass

    bbox = dict(
        boxstyle="round,pad=0.3",
        facecolor="white",
        edgecolor="black",
        linewidth=1,
    )

    ax.text(
        x, y, text,
        fontsize=fontsize,
        ha=ha,
        va=va,
        bbox=bbox,
        zorder=zorder,
    )

def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--in", dest="inp", default="cpu_per_core.csv", help="Input CSV path")
    ap.add_argument("--out", dest="outp", default=None, help="Save plot to file (png/pdf/etc)")
    ap.add_argument("--mode", choices=["avg", "total"], default="avg",
                    help="Aggregate mode: avg (mean per-core) or total (sum)")
    ap.add_argument("--show", action="store_true", help="Show interactive window")
    args = ap.parse_args()

    df = pd.read_csv(args.inp)

    # Parse timestamps
    df["timestamp_iso"] = pd.to_datetime(df["timestamp_iso"])
    df = df.sort_values("timestamp_iso").set_index("timestamp_iso")

    core_cols = [c for c in df.columns if c.startswith("core_")]
    if not core_cols:
        raise SystemExit("No core_* columns found in CSV.")

    if args.mode == "avg":
        series = df[core_cols].mean(axis=1)
        ylabel = "Average CPU utilization (%)"
    else:
        series = df[core_cols].sum(axis=1)
        ylabel = "Total CPU utilization (%)"

    ax = series.plot()
    ax.set_xlabel("Time")
    ax.set_ylabel(ylabel)
    ax.grid(True)

    x_min = df.index.min()
    print(x_min)
    x_max = df.index.max()
    coords = [
        (x_min, 2),
        ("2025-12-23 16:01:44.398000+00:00", 1),
        ("2025-12-23 16:02:05.398000+00:00", 0),
    ]
    add_ladder(ax, coords, x_min=x_min, x_max=x_max, where="post")

    # Add text box.
    add_text_box(ax, "2025-12-23 16:01:35.398000+00:00", 10, "100 IPS")
    add_text_box(ax, "2025-12-23 16:01:55.398000+00:00", 30, "300 IPS")
    add_text_box(ax, "2025-12-23 16:02:15.398000+00:00", 52, "500 IPS")

    plt.tight_layout()

    if args.outp:
        plt.savefig(args.outp, dpi=200)

    if args.show or not args.outp:
        plt.show()


if __name__ == "__main__":
    main()

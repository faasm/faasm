"""
Record per-core CPU utilization over time into a CSV file.

Output CSV columns:
  timestamp_iso, unix_ts, core_0, core_1, ..., core_{N-1}

Requires: psutil
  pip install psutil
"""

import argparse
import csv
import datetime as dt
import os
import time
from typing import List

import psutil


def iso_now() -> str:
    return dt.datetime.now(dt.timezone.utc).astimezone().isoformat(timespec="milliseconds")


def ensure_header(csv_path: str, ncores: int) -> None:
    """Create file with header if it doesn't exist or is empty."""
    needs_header = (not os.path.exists(csv_path)) or os.path.getsize(csv_path) == 0
    if not needs_header:
        return

    header = ["timestamp_iso", "unix_ts"] + [f"core_{i}" for i in range(ncores)]
    with open(csv_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(header)


def record(csv_path: str, interval: float, duration: float | None) -> None:
    ncores = psutil.cpu_count(logical=True) or 1
    ensure_header(csv_path, ncores)

    # Prime cpu_percent so first reading isn't all zeros.
    psutil.cpu_percent(interval=None, percpu=True)
    time.sleep(max(0.0, interval))

    start = time.time()
    with open(csv_path, "a", newline="") as f:
        writer = csv.writer(f)

        while True:
            now = time.time()
            if duration is not None and (now - start) >= duration:
                break

            # Non-blocking read since we already sleep for interval.
            per_core: List[float] = psutil.cpu_percent(interval=None, percpu=True)
            # If core count changes (rare, but can happen with hotplug/containers),
            # re-sync header expectations by padding/truncating.
            if len(per_core) < ncores:
                per_core = per_core + [0.0] * (ncores - len(per_core))
            elif len(per_core) > ncores:
                per_core = per_core[:ncores]

            writer.writerow([iso_now(), f"{now:.6f}"] + [f"{v:.1f}" for v in per_core])
            f.flush()

            time.sleep(max(0.0, interval))


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="cpu_per_core.csv", help="Output CSV path")
    ap.add_argument("--interval", type=float, default=0.5, help="Sampling interval in seconds")
    ap.add_argument("--duration", type=float, default=None, help="Total duration in seconds (omit for infinite)")
    args = ap.parse_args()

    if args.interval <= 0:
        raise SystemExit("--interval must be > 0")

    print(f"Recording per-core CPU to {args.out} every {args.interval}s"
          + (f" for {args.duration}s" if args.duration is not None else " (until Ctrl-C)"))
    try:
        record(args.out, args.interval, args.duration)
    except KeyboardInterrupt:
        print("\nStopped.")


if __name__ == "__main__":
    main()

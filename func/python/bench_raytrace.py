from performance.benchmarks.bm_raytrace import bench_raytrace, DEFAULT_WIDTH, DEFAULT_HEIGHT


if __name__ == "__main__":
    filename = "raytrace.ppm"
    bench_raytrace(2, DEFAULT_WIDTH, DEFAULT_HEIGHT, filename)

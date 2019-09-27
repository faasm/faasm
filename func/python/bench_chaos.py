from performance.benchmarks.bm_chaos import Spline, GVector, Chaosgame, DEFAULT_THICKNESS, DEFAULT_WIDTH, \
    DEFAULT_HEIGHT, DEFAULT_ITERATIONS, DEFAULT_RNG_SEED

if __name__ == "__main__":
    splines = [
        Spline([
            GVector(1.597350, 3.304460, 0.000000),
            GVector(1.575810, 4.123260, 0.000000),
            GVector(1.313210, 5.288350, 0.000000),
            GVector(1.618900, 5.329910, 0.000000),
            GVector(2.889940, 5.502700, 0.000000),
            GVector(2.373060, 4.381830, 0.000000),
            GVector(1.662000, 4.360280, 0.000000)],
            3, [0, 0, 0, 1, 1, 1, 2, 2, 2]),
        Spline([
            GVector(2.804500, 4.017350, 0.000000),
            GVector(2.550500, 3.525230, 0.000000),
            GVector(1.979010, 2.620360, 0.000000),
            GVector(1.979010, 2.620360, 0.000000)],
            3, [0, 0, 0, 1, 1, 1]),
        Spline([
            GVector(2.001670, 4.011320, 0.000000),
            GVector(2.335040, 3.312830, 0.000000),
            GVector(2.366800, 3.233460, 0.000000),
            GVector(2.366800, 3.233460, 0.000000)],
            3, [0, 0, 0, 1, 1, 1])
    ]

    chaos = Chaosgame(splines, DEFAULT_THICKNESS)

    filename = "chaos.ppm"
    iterations = DEFAULT_ITERATIONS * 2
    chaos.create_image_chaos(DEFAULT_WIDTH, DEFAULT_HEIGHT, iterations, filename, DEFAULT_RNG_SEED)

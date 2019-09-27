import io
import logging
from performance.benchmarks.bm_logging import BENCHMARKS, truncate_stream

if __name__ == "__main__":
    stream = io.StringIO()

    handler = logging.StreamHandler(stream=stream)
    logger = logging.getLogger("benchlogger")
    logger.propagate = False
    logger.addHandler(handler)
    logger.setLevel(logging.WARNING)

    benchmarks = sorted(BENCHMARKS)

    loops = 100

    for bench in benchmarks:
        bench_func = BENCHMARKS[bench]

        for i in range(3):
            bench_func(loops, logger, stream)

            truncate_stream(stream)

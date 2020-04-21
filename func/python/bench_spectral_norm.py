from pyperformance.benchmarks.bm_spectral_norm import DEFAULT_N, eval_AtA_times_u


def faasm_main():
    loops = 2
    range_it = range(loops)

    for _ in range_it:
        u = [1] * DEFAULT_N

        for dummy in range(10):
            v = eval_AtA_times_u(u)
            u = eval_AtA_times_u(v)

        vBv = vv = 0

        for ue, ve in zip(u, v):
            vBv += ue * ve
            vv += ve * ve


if __name__ == "__main__":
    faasm_main()

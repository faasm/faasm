from pyperformance.benchmarks.bm_pidigits import calc_ndigits, DEFAULT_DIGITS


def faasm_main():
    calc_ndigits(DEFAULT_DIGITS * 3)


if __name__ == "__main__":
    faasm_main()

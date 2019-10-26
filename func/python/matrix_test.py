import numpy as np


def _subdivide_matrix(m):
    n = m.shape[0]
    half_n = n // 2

    m_a = m[0:half_n, 0:half_n]
    m_b = m[0:half_n, half_n:n]
    m_c = m[half_n:n, 0:half_n]
    m_d = m[half_n:n, half_n:n]

    return m_a, m_b, m_c, m_d


def _reconstruct_matrix(m_a, m_b, m_c, m_d):
    return np.concatenate(
        (
            np.concatenate((m_a, m_b), axis=1),
            np.concatenate((m_c, m_d), axis=1)
        ),
        axis=0
    )


def _divide_and_conquer_multiply(a, b):
    n = a.shape[0]

    # Do naive divide and conquer
    m_aa, m_ab, m_ac, m_ad = _subdivide_matrix(a)
    m_ba, m_bb, m_bc, m_bd = _subdivide_matrix(b)

    # Keep dividing and conquering down to a limit
    if n > 4:
        r_a = _divide_and_conquer_multiply(m_aa, m_ba) + _divide_and_conquer_multiply(m_ab, m_bc)
        r_b = _divide_and_conquer_multiply(m_aa, m_bb) + _divide_and_conquer_multiply(m_ab, m_bd)
        r_c = _divide_and_conquer_multiply(m_ac, m_ba) + _divide_and_conquer_multiply(m_ad, m_bc)
        r_d = _divide_and_conquer_multiply(m_ac, m_bb) + _divide_and_conquer_multiply(m_ad, m_bd)
    else:
        r_a = np.dot(m_aa, m_ba) + np.dot(m_ab, m_bc)
        r_b = np.dot(m_aa, m_bb) + np.dot(m_ab, m_bd)
        r_c = np.dot(m_ac, m_ba) + np.dot(m_ad, m_bc)
        r_d = np.dot(m_ac, m_bb) + np.dot(m_ad, m_bd)

    return _reconstruct_matrix(r_a, r_b, r_c, r_d)


def main():
    # Do normal matrix multiplication
    a = np.array((
        [1, 2, 3, 4, 5, 6, 7, 8],
        [9, 10, 11, 12, 13, 14, 15, 16],
        [17, 18, 19, 20, 21, 22, 23, 24],
        [25, 26, 27, 28, 29, 30, 31, 32],
        [33, 34, 35, 36, 37, 38, 39, 40],
        [41, 42, 43, 44, 45, 46, 47, 48],
        [49, 50, 51, 52, 53, 54, 55, 56],
        [57, 58, 59, 60, 61, 62, 63, 64],
    ))

    b = np.array((
        [1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5],
        [9.5, 10.5, 11.5, 12.5, 13.5, 14.5, 15.5, 16.5],
        [17.5, 18.5, 19.5, 20.5, 21.5, 22.5, 23.5, 24.5],
        [25.5, 26.5, 27.5, 28.5, 29.5, 30.5, 31.5, 32.5],
        [33.5, 34.5, 35.5, 36.5, 37.5, 38.5, 39.5, 40.5],
        [41.5, 42.5, 43.5, 44.5, 45.5, 46.5, 47.5, 48.5],
        [49.5, 50.5, 51.5, 52.5, 53.5, 54.5, 55.5, 56.5],
        [57.5, 58.5, 59.5, 60.5, 61.5, 62.5, 63.5, 64.5],
    ))

    expected = np.dot(a, b)

    # Do divide and conquer multiplication
    actual = _divide_and_conquer_multiply(a, b)

    # Print results
    print("\nEXPECTED\n {}".format(expected))
    print("\nACTUAL\n{}".format(actual))


if __name__ == "__main__":
    main()

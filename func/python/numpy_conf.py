from time import time

import numpy as np

INFOS = [
    "blas_mkl_info",
    "blis_info",
    "openblas_info",
    "atlas_3_10_blas_threads_info",
    "atlas_3_10_blas_info",
    "atlas_blas_threads_info",
    "atlas_blas_info",
    "accelerate_info",
    "blas_info",
    "blas_src_info",
    "blas_opt_info",
    "lapack_mkl_info",
    "openblas_lapack_info",
    "openblas_clapack_info",
    "flame_info",
    "atlas_3_10_threads_info",
    "atlas_3_10_info",
    "atlas_threads_info",
    "atlas_info",
    "lapack_info",
    "lapack_src_info",
    "lapack_opt_info"
]


def main_func():
    for info_name in INFOS:
        this_info = np.__config__.get_info(info_name)
        if this_info:
            print("Unexpected linalg support: {}:\n{}".format(info_name, this_info))

    # Sanity check to see how long this takes
    dim = 1024
    print("Multiplying two {}x{} matrices".format(dim, dim))
    dt_a = time()
    a = np.random.rand(dim, dim)
    b = np.random.rand(dim, dim)
    np.dot(a, b)

    time_taken = time() - dt_a
    print("Took {:.2f}s".format(time_taken))


if __name__ == "__main__":
    main_func()

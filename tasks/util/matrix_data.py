from os import makedirs
from os.path import join, exists

import numpy as np
from numpy import int32
from pyfaasm.matrix import SUBMATRICES_KEY_A, SUBMATRICES_KEY_B, load_matrix_conf_from_state, RESULT_MATRIX_KEY, \
    subdivide_matrix_into_file, MATRIX_CONF_STATE_KEY

from tasks.util.config import get_faasm_config
from tasks.util.env import HOME_DIR
from tasks.util.state import upload_binary_state

MATRIX_DATA_DIR = join(HOME_DIR, "faasm", "data", "matrix")
MATRIX_A_FILE = join(MATRIX_DATA_DIR, "mat_a")
PARAMS_FILE = join(MATRIX_DATA_DIR, "params")
MATRIX_B_FILE = join(MATRIX_DATA_DIR, "mat_b")
RESULT_FILE = join(MATRIX_DATA_DIR, "result")


def set_up_matrix_data(matrix_size, n_splits, host="localhost", knative=False):
    matrix_size = int(matrix_size)
    n_splits = int(n_splits)

    # Write params to file
    print("Setting up {}x{} matrix with {} splits".format(matrix_size, matrix_size, n_splits))
    params = np.array((matrix_size, n_splits), dtype=int32)
    with open(PARAMS_FILE, "wb") as fh:
        fh.write(params.tobytes())

    # Set up the problem
    if not exists(MATRIX_DATA_DIR):
        makedirs(MATRIX_DATA_DIR)

    mat_a = np.random.rand(matrix_size, matrix_size)
    mat_b = np.random.rand(matrix_size, matrix_size)
    subdivide_matrix_into_file(mat_a, MATRIX_A_FILE)
    subdivide_matrix_into_file(mat_b, MATRIX_B_FILE)

    # Zero the result
    zero_result = np.zeros((matrix_size, matrix_size))
    with open(RESULT_FILE, "wb") as fh:
        fh.write(zero_result.tobytes())

    faasm_conf = get_faasm_config()
    if knative:
        if not faasm_conf.has_section("Kubernetes"):
            host = host if host else "localhost"
        else:
            host = faasm_conf["Kubernetes"]["upload_host"]

    upload_binary_state("python", MATRIX_CONF_STATE_KEY, PARAMS_FILE, host=host)
    upload_binary_state("python", SUBMATRICES_KEY_A, MATRIX_A_FILE, host=host)
    upload_binary_state("python", SUBMATRICES_KEY_B, MATRIX_B_FILE, host=host)
    upload_binary_state("python", RESULT_MATRIX_KEY, RESULT_FILE, host=host)

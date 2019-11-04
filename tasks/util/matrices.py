from os import makedirs
from os.path import join, exists

from tasks.util.env import HOME_DIR

# NOTE - THESE HAVE TO MATCH PYFAASM
SUBMATRICES_KEY_A = "submatrices_a"
SUBMATRICES_KEY_B = "submatrices_b"
RESULT_MATRIX_KEY = "result_matrix"
MATRIX_CONF_STATE_KEY = "matrix_state"

MATRIX_DATA_DIR = join(HOME_DIR, "faasm", "data", "matrix")
MATRIX_A_FILE = join(MATRIX_DATA_DIR, "mat_a")
PARAMS_FILE = join(MATRIX_DATA_DIR, "params")
MATRIX_B_FILE = join(MATRIX_DATA_DIR, "mat_b")
RESULT_FILE = join(MATRIX_DATA_DIR, "result")


def _get_file(prefix, matrix_size, n_splits):
    # Set up the problem
    if not exists(MATRIX_DATA_DIR):
        makedirs(MATRIX_DATA_DIR)

    return "{}_{}_{}".format(prefix, matrix_size, n_splits)


def get_params_file(matrix_size, n_splits):
    return _get_file(PARAMS_FILE, matrix_size, n_splits)


def get_mat_a_file(matrix_size, n_splits):
    return _get_file(MATRIX_A_FILE, matrix_size, n_splits)


def get_mat_b_file(matrix_size, n_splits):
    return _get_file(MATRIX_B_FILE, matrix_size, n_splits)


def get_result_file(matrix_size, n_splits):
    return _get_file(RESULT_FILE, matrix_size, n_splits)

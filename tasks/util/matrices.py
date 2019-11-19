from os import makedirs
from os.path import join, exists

from tasks.util.env import HOME_DIR, FAASM_DATA_DIR

# NOTE - THESE HAVE TO MATCH PYFAASM
SUBMATRICES_KEY_A = "mat_a"
SUBMATRICES_KEY_B = "mat_b"
MATRIX_CONF_STATE_KEY = "matrix_state"

MATRIX_DATA_DIR = join(FAASM_DATA_DIR, "matrix")


def get_matrix_dir(matrix_size, n_splits):
    dir_name = "{}x{}_{}".format(matrix_size, matrix_size, n_splits)
    dir_path = join(MATRIX_DATA_DIR, dir_name)

    if not exists(dir_path):
        makedirs(dir_path)

    return dir_path

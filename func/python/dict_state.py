import pickle
from pyfaasm.core import (
    get_state,
    get_state_size,
    set_state,
    chain_this_with_input,
    await_call,
)

KEY_A = "dict_a"
KEY_B = "dict_b"


def get_dict_from_state(key):
    dict_size = get_state_size(key)
    pickled_dict = get_state(key, dict_size)
    return pickle.loads(pickled_dict)


def write_dict_to_state(key, dict_in):
    pickled_dict = pickle.dumps(dict_in)
    set_state(key, pickled_dict)


# This function is chained
def check_pickle(input_bytes):
    d = get_dict_from_state(KEY_A)
    d["epsilon"] = "zeta"
    write_dict_to_state(KEY_B, d)


def faasm_main():
    # Write initial dictionary to state
    dict_a = {"alpha": "beta", "gamma": "delta"}
    write_dict_to_state(KEY_A, dict_a)

    # Make the chained call
    call_id = chain_this_with_input(check_pickle, b"")
    await_call(call_id)

    # Load from state again
    dict_b = get_dict_from_state(KEY_B)

    # Check expectation
    expected = {"alpha": "beta", "gamma": "delta", "epsilon": "zeta"}
    if dict_b != expected:
        print("Expected {} but got {}".format(expected, dict_b))
        exit(1)

    print("Dictionary as expected: {}".format(dict_b))

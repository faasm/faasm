import os

from flask import Flask, request
from pyfaasm.core import setFunctionIdx, setInput, getOutput

app = Flask(__name__)

os.environ["PYTHON_LOCAL_INPUT_OUTPUT"] = True


@app.route('/')
def run_func():
    json_data = request.get_json()

    user = json_data["py_user"]
    func = json_data["py_func"]
    idx = json_data["py_idx"]
    input_data = json_data["input_data"]

    print("Executing {}/{} (idx {}) with input {}", user, func, idx, input_data)

    # Assume function is in the current path
    module_name = "{}.{}".format(user, func)
    mod = __import__(module_name, fromlist=[""])

    # Set up input
    setInput(input_data)

    # Set up the function index and run
    setFunctionIdx(idx)
    mod.main_func()

    # Return the output
    return getOutput()


if __name__ == "__main__":
    app.run(debug=True, host='0.0.0.0', port=int(os.environ.get('PORT', 8080)))

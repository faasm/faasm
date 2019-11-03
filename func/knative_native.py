import os
import threading
from decimal import Decimal
from json import dumps
from time import sleep

from flask import Flask, request
from pyfaasm.core import getOutput, setEmulatorMessage, emulatorSetStatus, emulatorGetAsyncResponse, setLocalInputOutput

app = Flask(__name__)

is_cold_start = True


def execute_main(mod):
    mod.main_func()
    emulatorSetStatus(1)


def handle_message(json_data):
    # Set up the emulator
    setEmulatorMessage(dumps(json_data))

    user = json_data["py_user"]
    func = json_data["py_func"]
    idx = json_data.get("py_idx", 0)

    print("Executing {}/{} (idx {})".format(user, func, idx))

    # Assume function is in the current path
    module_name = "{}.{}".format(user, func)
    mod = __import__(module_name, fromlist=[""])

    if json_data.get("async", False):
        # Use emulator to dispatch in the background
        func_thread = threading.Thread(target=execute_main, args=[mod])
        func_thread.start()

        return emulatorGetAsyncResponse()
    else:
        # Run the function
        mod.main_func()

        # Return the output
        func_output = getOutput()

        if not func_output:
            return "Empty output"
        else:
            return func_output


@app.route('/', methods=["GET", "POST"])
def run_func():
    global is_cold_start
    if is_cold_start:
        delay_str = os.environ.get("COLD_START_DELAY_MS", "1000")
        delay_seconds = Decimal(delay_str) / 1000
        print("Simulating cold start for {} seconds".format(delay_seconds))

        sleep(delay_seconds)
        is_cold_start = False

    json_data = request.get_json()
    return handle_message(json_data)


if __name__ == "__main__":
    setLocalInputOutput(True)
    app.run(debug=True, host='0.0.0.0', port=int(os.environ.get('PORT', 8080)))

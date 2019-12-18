import logging
import os
import threading
from decimal import Decimal
from json import dumps
from time import sleep

from flask import Flask, request
from pyfaasm.core import getOutput, setEmulatorMessage, emulatorSetStatus, emulatorGetAsyncResponse, setLocalInputOutput

app = Flask(__name__)

request_count = 0

logging.basicConfig(level=logging.INFO)


def execute_main(json_data):
    # Set up the emulator again (in case we're running in a separate thread)
    setEmulatorMessage(dumps(json_data))

    user = json_data["py_user"]
    func = json_data["py_func"]
    idx = json_data.get("py_idx", 0)

    app.logger.info("Executing {}/{} (idx {})".format(user, func, idx))

    # Assume function is in the current path
    module_name = "{}.{}".format(user, func)
    mod = __import__(module_name, fromlist=[""])

    mod.main_func()
    emulatorSetStatus(1)


@app.route('/', methods=["GET", "POST"])
def run_func():
    global request_count
    setLocalInputOutput(True)

    # Get input
    json_data = request.get_json()
    app.logger.info("Knative request: {}".format(json_data))

    # Check if we should be doing a cold start
    is_cold_start = False
    cold_start_interval_str = json_data.get("cold_start_interval")
    if cold_start_interval_str:
        cold_start_interval = int(cold_start_interval_str)
        is_cold_start = request_count % cold_start_interval == 0

    if request_count == 0 or is_cold_start:
        # Simulate cold start if necessary
        delay_str = os.environ.get("COLD_START_DELAY_MS", "0")
        if delay_str != "0":
            delay_seconds = Decimal(delay_str) / 1000
            app.logger.info("Simulating cold start for {} seconds".format(delay_seconds))

            sleep(delay_seconds)

        # TODO - need to clear out state here if it's a cold start

    # Up the request count
    request_count += 1

    # Set up this main thread with the emulator
    # Make sure to pass on the message ID for child threads
    msgId = setEmulatorMessage(dumps(json_data))
    json_data["id"] = msgId

    if json_data.get("async", False):
        # Run in background if async request
        func_thread = threading.Thread(target=execute_main, args=[json_data])
        func_thread.start()

        return emulatorGetAsyncResponse()
    else:
        # Run in main thread
        execute_main(json_data)
        func_output = getOutput()

        if not func_output:
            return "Empty output"
        else:
            return func_output

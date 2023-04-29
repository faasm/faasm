# The conversion here must match the enum in the HttpMessage definition in
# the planner protobuf file
PLANNER_HTTP_MESSAGE_TYPE = {
    "RESET": 1,
    "GET_CONFIG": 2,
    "FLUSH_AVAILABLE_HOSTS": 3,
    "FLUSH_EXECUTORS": 4,
    "EXECUTE": 5,
    "EXECUTE_STATUS": 6,
}


def prepare_planner_msg(msg_type, msg_body=None):
    if msg_type not in PLANNER_HTTP_MESSAGE_TYPE:
        print(
            "Unrecognised HTTP message type for planner: {}".format(msg_type)
        )
        raise RuntimeError("Unrecognised planner HTTP message type")

    planner_msg = {
        "http_type": PLANNER_HTTP_MESSAGE_TYPE[msg_type],
    }

    if msg_body:
        # FIXME: currently we use protobuf for JSON (de)-serialisation in
        # faabric. In addition, we nest a JSON as a string in another JSON,
        # which means that boolean values (in JSON) are serialised in the
        # nested string as True, False. Unfortunately, protobuf only identifies
        # as booleans the string literals `true` and `false` (with lower-case).
        # So we need to be careful here
        boolean_flags_in_nested_msg = ["mpi", "sgx"]
        for key in boolean_flags_in_nested_msg:
            if key in msg_body:
                msg_body[key] = str(msg_body[key]).lower()

        planner_msg["payload"] = str(msg_body)

    return planner_msg

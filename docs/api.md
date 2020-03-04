# Faasm HTTP API

Most interactions with Faasm can be performed through the [CLI](cli.md),
which itself uses the Faasm HTTP API.

You should only need to interact with the HTTP API directly when performing 
benchmarking or integrating with another system.

Note that the API currently has no authentication (as this is still a research
project). 
 
## Endpoints

The Faasm API is split into two parts which correspond to two endpoints:

- `upload` - uploading state and functions
- `invoke` - function invocation and function status

Generally `upload` is on port 8002, and `invoke` on port 8080. The specifics of
the endpoints themselves will depend on the deployment of Faasm. Locally these
are available on `localhost`, whereas in a [Knative deployment](kubernetes.md) 
these will be provided by the Kubernetes infrastructure.
 
In some deployments the `upload` server will also act as a file server for the 
Faasm runtime instances.
 
## Upload

The upload API is REST-ish and a list of interactions with cURL are shown below.

### Functions

Functions all have a `user` and a `name`.

```bash
# Upload a C/C++ function file
curl -X PUT <host>:8002/f/<user>/<name> -T <wasm_file>

# Download a C/C++ function file
curl -X GET <host>:8002/f/<user>/<name> -o <out_file>

# Upload a Python function file
curl -X PUT <host>:8002/p/<user>/<name> -T <python_file>

# Download a Python function file
curl -X GET <host>:8002/p/<user>/<name> -o <out_file>
```

### State

State values all have a `user` and a `key`.

These `key`s are then accessible to functions at runtime. 

```bash
# Upload state data directly
curl -X PUT <host>:8002/s/<user>/<key> -d "some state"

# Upload state from a file
curl -X PUT <host>:8002/s/<user>/<key> -T <state_file>

# Download a state value
curl -X GET <host>:8002/s/<user>/<key> -o <out_file>
```

### Shared files

Files can be made accessible to functions through the virtual filesystem.

```bash
# Upload a file (specifying the path at which functions can access the file as a header)
curl -X PUT -H "FilePath: <access_path>" <host>:8002/file -T <file>

# Download a file
curl -X GET <host>:8002/file -o <out_file>
```

## Invoke

The invoke API is handled with JSON messages over HTTP. 

Functions can be invoked synchronously while the client awaits the result, or 
asynchronously when a call ID is returned which can be used to poll
the function's status.

### JSON Format

All Faasm invoke API calls use the same JSON message format. Possible  
fields are as follows:

| Field | Type | Description | Use |
|---|---|---|---|
| `user` | string | User associated with the function | Invoking | 
| `function` | string | The function name | Invoking |
| `async` | bool | Flag for asynchronous calls | Invoking |
| `input_data` | bool | Input data for the function call | Invoking |
| `status` | bool | Flag to indicate a status call | Polling |
| `id` | int | The call ID, used for async calls | Polling |
| `python` | bool | Flag to indicate a Python function | Python |
| `py_user` | string | Python user (`user` must be `python`) | Python |
| `py_func` | string | Python function (`function` must be `py_func`) | Python |
| `mpi` | bool | Flag to indicate an MPI call | MPI |
| `mpi_world_size` | int | How big to make the MPI world | MPI |

### Example - synchronous invocation

An example of synchronously invoking a function from Python is shown below:

```python
import requests

endpoint = "http://localhost:8080"

json_data = {
    "user": "demo",
    "function": "echo",
    "input_data": "Hello API",
}

requests.post(endpoint, json=json_data)
```

The response will contain the function output as well as any captured stdout.

### Example - asynchronous invocation

An example of asynchronously invoking a function from Python is shown below:

```python
import requests
from time import sleep

endpoint = "http://localhost:8080"

invoke_json = {
    "user": "demo",
    "function": "echo",
    "input_data": "Hello API",
    "async": True
}

res = requests.post(endpoint, json=invoke_json)
call_id = res.content.decode()
print("Async call {}".format(call_id))

status_json = {
    "id": int(call_id),
    "status": True,
}

str_res = ""
count = 0
while not str_res.startswith("SUCCESS") and count < 5:
    res = requests.post(endpoint, json=status_json)
    str_res = res.content.decode()
    count+=1

    sleep(0.2)

print(str_res)
```
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

All Faasm invoke API calls use the same JSON message format. The following 
fields are shared by all calls:

| Field | Type | Purpose |
|---|---|---|
| `user` | string | User associated with the function | 
| `function` | string | The function name |
| `id` | int | The call ID, used for async calls |
| `async` | bool | Flag for asynchronous calls |
| `input_data` | bool | Input data for the function call |

### Invoke functions


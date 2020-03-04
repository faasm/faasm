# Faasm HTTP API

Most interactions with Faasm can be performed through the [CLI](cli.md),
which itself uses the Faasm HTTP API.

You should only need to interact with the HTTP API directly when performing 
benchmarking or integrating with another system.
 
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
# Upload a function file
curl -X PUT <host>:8002/f/<user>/<name> -T <wasm_file>

# Download a function file
curl -X GET <host>:8002/f/<user>/<name> -o <out_file>
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

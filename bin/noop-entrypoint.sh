#!/bin/bash

# Used as a no-op entrypoint to override entrypoints set in parent containers
# Would be better to unset the entrypoint but can't work out how

set -e

# Continue with normal command
exec "$@"
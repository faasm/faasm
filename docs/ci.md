# Continuous Integration

CI builds can be found on Travis.

## Checking Locally

To check things locally you can run the following:

```
# Build the containers
source workon.sh
inv build-worker build-testing

# Execute the tests in the container with Docker Compose
docker-compose -f test-compose.yml run tester /usr/local/code/faasm/bin/ci_run.sh
```

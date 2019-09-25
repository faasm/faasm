# Continuous Integration

CI builds can be found on Travis.

## Checking Locally

To check things locally you can run the following:

```
# Build the containers
source workon.sh
inv docker-build-worker docker-build-testing

# Execute the tests in the container with Docker Compose
docker-compose -f docker-compose-test.yml run tester /usr/local/code/faasm/bin/ci_run.sh
```

If things don't work, you can attach to the container and run the tests:

```
docker-compose -f docker-compose-test.yml run tester /bin/bash
/usr/local/code/faasm/bin/ci_run.sh
```

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

You can run individual tests with Catch syntax, e.g.

```
/faasm/build/bin/tests -d yes --order lex "Python fileserver test"
```

You can rebuild tests while in the container as follows:

```
cd /faasm/build
cmake --build . --target tests -- -j
```

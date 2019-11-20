# Continuous Integration

CI builds can be found on [Travis](https://travis-ci.org/lsds/Faasm).

We have to build and push the `testing` container _before_ the build runs. This is because building the container from scratch on Travis takes a while.

## Checking Locally

To check things locally you can run the following:

```
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

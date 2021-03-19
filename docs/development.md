# Faasm Development

Most of the Faasm development is captured in the [faasm/development
repo](https://github.com/faasm/development/)

### Troubleshooting CI

If the CI build fails and you can't work out why, you can replicate the test
environment locally.

First, make sure the environment variables and Docker image in 
[`docker/docker-compose-ci.yml`](../docker/docker-compose-ci.yml) exactly match
those in [`.github/workflows/tests.yml`](../.github/workflows/tests.yml).

Then, run the container and work through the steps in the Github Actions file,
to see where things might have gone wrong.

```
# To start the container 
docker-compose -f docker/docker-compose-ci.yml run cli /bin/bash
```

## Code style

Code style is checked as part of the CI build and uses the following
configuration:

- **C/C++** - [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html)
  with the [Mozilla C++
  style](https://firefox-source-docs.mozilla.org/code-quality/coding-style/coding_style_cpp.html)
- **Python** - [Black](https://github.com/psf/black) with an 80 char line limit
- **Markdown** - wrap to 80 characters where possible (not enforced)
- **CMake** - TBD
- **Bash** - TBD

## Detailed notes

### Networking

If you want to switch on network isolation you need to set up network
namespaces. To do this we need to ensure consistent interface naming (`eth0` for
main public interface). If your public interface is already called `eth0` then
you can skip this step.

- Edit `/etc/default/grub` and add `net.ifnames=0 biosdevname=0` to
  `GRUB_CMDLINE_LINUX_DEFAULT`
- Run `sudo update-grub`
- Restart the machine

This script will then set up the namespaces

```bash
sudo ./bin/netns.sh 20
```

### Cgroups

To use cgroup isolation, you'll need to run:

```bash
sudo ./bin/cgroup.sh
```

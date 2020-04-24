# Faasm CLI

This is just a wrapper package around the Faasm CLI to allow it to be used
in other projects. You only need to build and install this if you are creating 
a Faasm release or using the CLI from another project. Using the CLI normally 
from Faasm should "just work".

To release, you need to run:

```bash
./release.sh
```

You'll need to have set up your `keyring` if you want to avoid entering creds,
this is described [here](https://pypi.org/project/twine/).


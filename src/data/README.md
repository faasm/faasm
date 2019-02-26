# Datasets

## SGD (Hogwild)

We must use exactly the same RCV1 data as the original Hogwild experiments. The original code can be found
on their [website](http://i.stanford.edu/hazy/victor/Hogwild/) but you can use the steps below:

- Run the `hogwild.yml` Ansible script in this repo (this should clone and build [my Hogwild fork](https://github.com/Shillaker/hogwild))
- Run `./bin/svm_data` in this folder (`/usr/local/code/hogwild`). It should create `~/faasm/rcv1/test`
- Parse this data by running the `reuters_parse` executable in this repo
- This should create the serialised files at `~/faasm/data/reuters/`

This output data can be loaded by the SGD functions, but first it needs to be put into the relevant state store.
For a Kubernetes/ local deployment you can use the `reuters_upload` task. For AWS deployments the data needs to
be put in S3, which can be done with `reuters_upload_s3`.

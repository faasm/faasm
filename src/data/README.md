# Datasets

## SGD (Hogwild)

We must use exactly the same RCV1 data as the original Hogwild experiments. The original code can be found
on their [website](http://i.stanford.edu/hazy/victor/Hogwild/) but you can use the steps below:

- Run the `hogwild.yml` Ansible script in this repo (this should clone and build [my Hogwild fork](https://github.com/Shillaker/hogwild))
- Run `bin/svm_data` in this folder (`/usr/local/code/hogwild`)
- Parse this data by running the `reuters_parse` executable in this repo

This process parses and serialises the RCV1 data such that it can be loaded by the SGD functions. For the functions
to be able to do this, they need to have access to the data. For a Kubernetes/ local deployment you can use
the `upload` endpoint and the `reuters_upload` task. For AWS deployments the data needs to be put in S3,
which can be done with `reuters_upload_s3`.

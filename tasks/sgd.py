from botocore.exceptions import ClientError
from invoke import task

from tasks.aws import invoke_faasm_lambda, deploy_native_lambda_func, deploy_wasm_lambda_func, \
    lambda_concurrency, delete_lambda
from tasks.upload import upload

_MAX_BATCH_TIME = 180
_BATCH_MEMORY = 128
_BATCH_CONCURRENCY = 200

_SGD_FUNCS = {
    "sgd_finished": {
        "memory": 128,
        "timeout": 30,
        "concurrency": 1,
    },
    "sgd_loss": {
        "memory": 128,
        "timeout": 30,
        "concurrency": 1,
    },
    "reuters_svm": {
        "memory": _BATCH_MEMORY,
        "timeout": _MAX_BATCH_TIME,
        "concurrency": _BATCH_CONCURRENCY,
    },
}


# -------------------------------------------------
# AWS interaction with SGD
# -------------------------------------------------

@task
def begin_aws_svm(ctx):
    invoke_faasm_lambda(ctx, "sgd", "reuters_svm")


@task
def upload_sgd_funcs(ctx, host="localhost"):
    for func_name in _SGD_FUNCS:
        upload(ctx, "sgd", func_name, host=host)


@task
def deploy_sgd_wasm_lambda(ctx):
    for func_name, func_spec in _SGD_FUNCS.items():
        print("Deploying wasm {}".format(func_name))

        deploy_wasm_lambda_func(ctx, "sgd", func_name)


@task
def deploy_sgd_native_lambda(ctx, func=None):
    if func:
        _do_func_deploy(ctx, func, _SGD_FUNCS[func])
    else:
        for func_name, func_spec in _SGD_FUNCS.items():
            _do_func_deploy(ctx, func_name, func_spec)


@task
def zero_sgd_native_lambda(ctx):
    for func_name, func_spec in _SGD_FUNCS.items():
        lambda_concurrency(ctx, "sgd-{}".format(func_name), 0)


def _do_func_deploy(ctx, func_name, func_spec):
    print("Deploying native {}".format(func_name))

    deploy_native_lambda_func(
        ctx,
        "sgd", func_name,
        memory=func_spec["memory"],
        timeout=func_spec["timeout"],
        concurrency=func_spec["concurrency"],
    )


@task
def delete_sgd_native_lambda(ctx, func=None):
    if func:
        _do_delete(ctx, func)
    else:
        for func_name, func_spec in _SGD_FUNCS.items():
            _do_delete(ctx, func_name)


def _do_delete(ctx, func_name):
    lambda_name = "sgd-{}".format(func_name)
    try:
        delete_lambda(ctx, lambda_name)
    except ClientError:
        print("Failed to delete {}. Continuing".format(lambda_name))

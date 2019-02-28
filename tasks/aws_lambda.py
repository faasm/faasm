from json import dumps

from invoke import task

import boto3

from tasks.env import AWS_REGION


@task
def list_lambdas(ctx):
    client = boto3.client("lambda", region_name=AWS_REGION)

    result = client.list_functions()

    for func_data in result["Functions"]:
        print("\n--- {} --- ".format(func_data["FunctionName"]))
        print("Timeout:  {}".format(func_data["Timeout"]))
        print("Memory:   {}MB".format(func_data["MemorySize"]))
        print("Modified: {}".format(func_data["LastModified"]))


@task
def invoke_lambda(ctx, lambda_name, async=False, payload=None):
    client = boto3.client("lambda", region_name=AWS_REGION)

    response = client.invoke(
        FunctionName=lambda_name,
        InvocationType="Event" if async else "RequestResponse",
        Payload=dumps(payload) if payload else None,
    )

    print(response)
    print("\nResponse payload: {}".format(response["Payload"].read()))


@task
def invoke_faasm_lambda(ctx, user, func, input_data="", extra_payload=None):
    client = boto3.client("lambda", region_name=AWS_REGION)

    payload = {
        "user": user,
        "function": func,
        "input_data": input_data,
    }

    if extra_payload:
        payload.update(extra_payload)

    response = client.invoke(
        FunctionName="faasm-dispatch",
        InvocationType="RequestResponse",
        Payload=dumps(payload)
    )

    print(response)
    response_payload = response["Payload"]
    print("\nResponse payload: {}".format(response_payload.read()))

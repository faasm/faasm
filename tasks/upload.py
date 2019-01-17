from subprocess import call

from invoke import task


@task
def upload(context, wasm_file, user, func_name, host="localhost"):
    print("Uploading {} as {} for user {}".format(wasm_file, func_name, user))

    cmd = [
        "curl",
        "-X", "PUT",
        "http://{}:8002/f/{}/{}".format(host, user, func_name),
        "-T", wasm_file
    ]

    cmd = " ".join(cmd)

    res = call(cmd, shell=True)

    if res == 0:
        print("Upload finished")
    else:
        print("Upload failed")

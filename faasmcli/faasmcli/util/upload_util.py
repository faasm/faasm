import subprocess


def curl_file(url, file_path, headers=None, quiet=False):
    cmd = ["curl", "-X", "PUT", url, "-T", file_path]

    headers = headers if headers else {}
    for key, value in headers.items():
        cmd.append('-H "{}: {}"'.format(key, value))

    cmd = " ".join(cmd)

    if not quiet:
        print(cmd)

    res = subprocess.call(cmd, shell=True)

    if res == 0 and not quiet:
        print("Successfully PUT file {} to {}".format(file_path, url))
    elif res != 0:
        raise RuntimeError(
            "Failed PUTting file {} to {}".format(file_path, url)
        )

# Shared files

Faasm functions have access to a shared filesystem. Users can upload and
download files to and from this filesystem, and functions can read and write
from and to the filesystem. This can be useful for uploading shared data, and
supporting legacy applications that rely on a filesystem.

Shared files are accessed via the `faasm://` prefix, e.g. using standard C++
filesystem calls to open and write data at `faasm://my_dir/my_file.txt` will
create a shared file that can be downloaded with the key `my_dir/my_file.txt`.

An example function using shared files can be found
[here](https://github.com/faasm/cpp/blob/main/func/demo/shared_file.cpp).

You can use the CLI to upload and download shared files:

```bash
inv files.upload <local_path> <shared_path>

inv files.download <shared_path> <local_path>
```

For example:

```bash
# Create some data
echo "This is content" > /tmp/bar.txt

inv files.upload /tmp/bar.txt foo/bar.txt

# Function can access and modify file at faasm://foo/bar.txt

inv files.download foo/bar.txt /tmp/bar.txt
```

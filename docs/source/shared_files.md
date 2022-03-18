# Shared files

Faasm functions have access to shared files that users can upload before
function execution and download afterwards.

Shared files are accessed via the `faasm://` prefix, e.g. using standard C++
filesystem calls to open and write data at `faasm://my_dir/my_file.txt` will
create a shared file that can be downloaded with the key `my_dir/my_file.txt`.

An example function using shared files can be found
[here](https://github.com/faasm/cpp/blob/main/func/demo/shared_file.cpp).

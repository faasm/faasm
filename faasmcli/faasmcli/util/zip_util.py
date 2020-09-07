import tempfile
import zipfile
import shutil
import os


def remove_from_zip(zip_path, *filenames):
    tempdir = tempfile.mkdtemp()

    try:
        temp_path = os.path.join(tempdir, "new.zip")

        with zipfile.ZipFile(zip_path, "r") as zip_read:
            with zipfile.ZipFile(temp_path, "w") as zip_write:
                for item in zip_read.infolist():
                    if item.filename not in filenames:
                        data = zip_read.read(item.filename)
                        zip_write.writestr(item, data)

        shutil.move(temp_path, zip_path)

    finally:
        shutil.rmtree(tempdir)


def replace_in_zip(zip_path, filename, replacement_path):
    remove_from_zip(zip_path, filename)

    with zipfile.ZipFile(zip_path, "a") as z:
        z.write(replacement_path, arcname=filename)

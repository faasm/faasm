from distutils.core import setup, Extension

# Tutorial - https://realpython.com/build-python-c-extension-module/

def main():
    setup(
        name="pyfaasm",
        version="1.0.0",
        description="Python interface for Faasm",
        author="Simon S",
        author_email="foo@bar.com",
        ext_modules=[Extension("pyfaasm", ["pyfaasm.c"])]
    )


if __name__ == "__main__":
    main()

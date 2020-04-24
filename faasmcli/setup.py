try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup

PKG_NAME = "faasmcli"
PKG_VERSION = "0.1.3"


def main():
    setup(name=PKG_NAME,
          version=PKG_VERSION,
          description="Faasm CLI",
          url="https://github.com/lsds/Faasm",
          long_description="""## Faasm CLI \nSee main repo at https://github.com/lsds/Faasm.""",
          long_description_content_type="text/markdown",
          license="Apache-2.0",
          packages=[PKG_NAME],
          author="Simon Shillaker",
          author_email="foo@bar.com",
          )


if __name__ == "__main__":
    main()

#!/bin/bash

set -e

pip install twine
rm -rf dist/ build/
python setup.py sdist
twine check dist/*
twine upload dist/*


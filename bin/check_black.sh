#!/bin/bash

set -e

black --check $(git ls-files "*.py")


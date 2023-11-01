#!/bin/bash
set -e

# Wrapper script for running invoke in virtual env

source bin/workon.sh && inv "$@"

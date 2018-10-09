#!/bin/bash

set -e

cgcreate -t ${USER}:${USER} -a ${USER}:${USER} -g cpu:faasm
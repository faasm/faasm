#!/bin/bash

WORKERS=10

# To avoid any global data leakage/ threading issues, run multiple workers
# all single-threaded
gunicorn \
    knative_native:app \
    --workers ${WORKERS} \
    --bind 0.0.0.0:8080 \
    --threads 1

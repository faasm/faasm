FROM faasm/knative-native-py_func

ARG PY_FUNC
ARG PY_USER
ENV PY_FUNC=$PY_FUNC
ENV PY_USER=$PY_USER

# Copy function file into place
COPY func/${PY_USER}/${PY_FUNC}.py /usr/local/faasm/shared_store/pyfuncs/${PY_USER}/${PY_FUNC}/function.py

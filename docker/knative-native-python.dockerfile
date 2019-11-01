FROM faasm/knative-native-base

ARG PY_FUNC
ARG PY_USER
ENV PY_FUNC=$PY_FUNC
ENV PY_USER=$PY_USER
ENV FAASM_FUNC=py_func
ENV FAASM_USER=python

# Copy function file into place
COPY func/${PY_USER}/${PY_FUNC}.py /usr/local/faasm/shared_store/pyfuncs/${PY_USER}/${PY_FUNC}/function.py

CMD /faasm/build/bin/knative_native_runner

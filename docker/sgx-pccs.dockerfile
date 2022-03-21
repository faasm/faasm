FROM ubuntu:18.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y
RUN apt-get install -y \
    cracklib-runtime \
    curl \
    gcc \
    g++ \
    git \
    gnupg2 \
    make \
    systemd \
    wget \
    unzip

RUN curl -sL https://deb.nodesource.com/setup_14.x | bash -
RUN apt-get install -y nodejs

# Manual install in Docker:
RUN git clone -b DCAP_1.12.1 \
    https://github.com/intel/SGXDataCenterAttestationPrimitives.git \
    /opt/intel/sgxdcap
WORKDIR /opt/intel/sgxdcap/tools/PCKCertSelection
RUN make
WORKDIR /opt/intel/sgxdcap/QuoteGeneration/pccs/lib
RUN cp /opt/intel/sgxdcap/tools/PCKCertSelection/out/libPCKCertSelection.so .
WORKDIR /opt/intel/sgxdcap/QuoteGeneration/pccs

# Self-signed certificate
WORKDIR /opt/intel/sgxdcap/QuoteGeneration/pccs/ssl_key
RUN openssl rand -writerand /root/.rnd
RUN openssl genrsa -out private.pem 2048
RUN openssl req -new -key private.pem -out csr.pem -subj \
    "/C=US/ST=New Sweden/L=Stockholm /O=.../OU=.../CN=.../emailAddress=..."
RUN openssl x509 -req -days 365 -in csr.pem -signkey private.pem -out file.crt
WORKDIR /opt/intel/sgxdcap/QuoteGeneration/pccs

# Set configuration file to accept self-signed certificates
# TODO: we probably don't need this here, only in the AESM server
RUN echo -e "# PCCS server address\nPCCS_URL=https://localhost:8081/sgx/certification/v3/\n# To accept insecure HTTPS cert, set this option to FALSE\nUSE_SECURE_CERT=FALSE" > /etc/sgx_default_qcnl.conf

# Configure the local PCS config file to accept incoming requests from any URL
RUN sed -i 's/127\.0\.0\.1/0\.0\.0\.0/g' config/default.json
# TODO - make the key a docker secret!!!!
RUN sed -i 's/"ApiKey": ""/"ApiKey": "975d7b4fd43248b2a1f3da748cf568b4"/g' config/default.json

RUN (printf "Y\n\n\nN\n") | ./install.sh

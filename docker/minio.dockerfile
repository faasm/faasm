FROM minio/minio:RELEASE.2021-10-23T03-28-24Z

CMD ["server", "/data/minio"]

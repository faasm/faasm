FROM redis:5

# Put config in place
COPY conf/redis-cluster.conf /redis.conf

# Add entrypoint script which will handle IPs changing
COPY bin/redis-cluster-entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
CMD ["redis-server", "/redis.conf"]
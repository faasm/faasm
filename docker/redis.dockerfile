FROM redis:5

# Put config in place
COPY conf/redis-volatile.conf /redis.conf

CMD ["redis-server", "/redis.conf"]
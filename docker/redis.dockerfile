FROM redis:6-alpine

# Put config in place
COPY deploy/conf/redis-volatile.conf /redis.conf

CMD ["redis-server", "/redis.conf"]
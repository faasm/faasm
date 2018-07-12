# DOCKER
.PHONY: build-base
build-base:
	docker build -t shillaker/faasm-base .

.PHONY: build-export
build-export: build-base
	docker build -t shillaker/faasm-export export.dockerfile

.PHONY: build-worker
build-worker:
	docker build -t shillaker/faasm-worker worker.dockerfile

.PHONY: build-edge
build-edge:
	docker build -t shillaker/faasm-edge edge.dockerfile

# DOCKER COMPOSE
start-all:
	docker-compose up -d

bash-worker:
	docker-compose run worker bash -l

# KUBERNETES
start-local:
	minikube start --vm-driver kvm2

deploy:
	kubectl apply -f k8s

# REDIS

redis:
	docker-compose run redis redis-cli -h redis
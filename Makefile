# DOCKER
build-base:
	docker build -t shillaker/faasm-base .

push-worker:
	docker push shillaker/faasm-worker

# DOCKER COMPOSE
start-all:
	docker-compose up -d

bash-worker:
	docker-compose run worker bash -l

# KUBERNETES
start-local:
	minikube start --vm-driver kvm2

deploy-local:
	kubectl apply -f k8s

# REDIS

redis:
	docker-compose run redis redis-cli -h redis
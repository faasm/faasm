# DOCKER

.PHONY: build-base-minikube
build-base-minikube:
	eval $$(minikube docker-env) && docker build -t shillaker/faasm-base:0.0.1 -f base.dockerfile .

.PHONY: build-core-minikube
build-core-minikube:
	eval $$(minikube docker-env) && docker build -t shillaker/faasm-core:0.0.1 .

.PHONY: build-worker-minikube
build-worker-minikube: build-core-minikube
	eval $$(minikube docker-env) && docker build -t shillaker/faasm-worker:0.0.1 -f worker.dockerfile .

.PHONY: build-edge-minikube
build-edge-minikube: build-core-minikube
	eval $$(minikube docker-env) && docker build -t shillaker/faasm-edge:0.0.1 -f edge.dockerfile .

.PHONY: build-base
build-base:
	docker build -t shillaker/faasm-base -f base.dockerfile .

.PHONY: build-core
build-core:
	docker build -t shillaker/faasm-core .

.PHONY: build-worker
build-worker: build-core
	docker build -t shillaker/faasm-worker -f worker.dockerfile .

.PHONY: build-edge
build-edge: build-core
	docker build -t shillaker/faasm-edge:0.0.1 -f edge.dockerfile .


# DOCKER COMPOSE
start-all:
	docker-compose up -d

stop-all:
	docker-compose stop

restart-all:
	docker-compose stop; docker-compose up -d

restart-worker:
	docker-compose stop worker; docker-compose up -d worker

restart-edge:
	docker-compose stop edge; docker-compose up -d edge

bash-worker:
	docker-compose run worker bash -l

bash-edge:
	docker-compose run edge bash -l

# KUBERNETES

minikube-reset:
	minikube delete && rm -rf ~/.minikube

minikube-start:
	minikube start --vm-driver kvm2 --logtostderr --disk-size 10g

minikube-url:
	minikube service edge --namespace=faasm --url

k8-deploy:
	kubectl apply -f k8s/namespace.yml && kubectl apply -f k8s

k8-clean:
	kubectl delete --all deployments,pods --namespace=faasm

k8-pods:
	kubectl get pods --namespace=faasm

k8-logs-edge:
	kubectl logs --tail=20 -f --namespace=faasm edge

k8-logs-worker:
	kubectl logs --tail=20 -f --namespace=faasm worker

# REDIS

redis:
	docker-compose run redis redis-cli -h redis

# PROTOBUF

proto-compile:
	protoc -proto_path=src --cpp_out=src/util src/util/faasm.proto

# WAVM

wavm:
	cd ansible && ansible-playbook wavm.yml --tags "build" --ask-become-pass
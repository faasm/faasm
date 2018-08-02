# DOCKER

.PHONY: minikube-env
minikube-env:
	eval $$(minikube docker-env)

.PHONY: build-base
build-base: minikube-env
	docker build -t shillaker/faasm-base:0.0.1 -f base.dockerfile .

.PHONY: build-core
build-core: minikube-env
	docker build -t shillaker/faasm-core:0.0.1 .

.PHONY: build-worker
build-worker: build-core
	docker build -t shillaker/faasm-worker:0.0.1 -f worker.dockerfile .

.PHONY: build-edge
build-edge: build-core
	docker build -t shillaker/faasm-edge:0.0.1 -f edge.dockerfile .

# KUBERNETES

minikube-reset:
	minikube delete && rm -rf ~/.minikube

minikube-start:
	minikube start --vm-driver kvm2 --logtostderr --disk-size 10g

k8-deploy:
	kubectl apply -f k8s/namespace.yml && kubectl apply -f k8s

k8-clean:
	kubectl delete --all pods --namespace=faasm

# REDIS

redis:
	docker-compose run redis redis-cli -h redis

# PROTOBUF

proto-compile:
	protoc -proto_path=src --cpp_out=src/util src/util/faasm.proto

# WAVM

wavm:
	cd ansible && ansible-playbook wavm.yml --tags "build" --ask-become-pass
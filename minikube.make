# MINIKUBE

.PHONY: build-base-minikube
build-base-minikube:
	eval $$(minikube docker-env) && docker build -t shillaker/faasm-base:0.0.1 -f base.dockerfile .

.PHONY: build-core-minikube
build-core-minikube:
	eval $$(minikube docker-env) && docker build -t shillaker/faasm-core:0.0.1 .

minikube-reset:
	minikube delete && rm -rf ~/.minikube

minikube-start:
	minikube start --vm-driver kvm2 --logtostderr --disk-size 10g

minikube-url:
	minikube service edge --namespace=faasm --url

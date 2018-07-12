build-worker:
	cd src/worker && docker build -t shillaker/faasm-worker .

start-local:
	minikube start --vm-driver kvm2

# DOCKER

.PHONY: build-base
build-base:
	docker build -t shillaker/faasm-base -f base.dockerfile .

.PHONY: build-core
build-core:
	docker build -t shillaker/faasm-core .

push-core:
	docker push shillaker/faasm-core

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

k8-deploy:
	cd ansible && ansible-playbook -i inventory/lsds.ini deploy.yml

k8-clean:
	cd ansible && ansible-playbook -i inventory/lsds.ini clean.yml

# REDIS

redis:
	docker-compose run redis redis-cli -h redis

# PROTOBUF

proto-compile:
	protoc -proto_path=src --cpp_out=src/util src/util/faasm.proto

# ANSIBLE

setup-libs:
	cd ansible && ansible-playbook libs.yml --ask-become-pass

setup-protobuf:
	cd ansible && ansible-playbook protobuf.yml --ask-become-pass

setup-clang:
	cd ansible && ansible-playbook clang.yml --ask-become-pass

setup-k8:
	cd ansible && ansible-playbook kubernetes.yml -i inventory/lsds.ini

setup-cgroup:
	cd ansible && ansible-playbook cgroup.yml --ask-become-pass

setup-network:
	cd ansible && ansible-playbook network.yml --ask-become-pass

# WASM toolchain

build-tools:
	python wasm-waterfall/src/build.py --build-include llvm,musl,compiler-rt,libcxx,libcxxabi --sync-include llvm,lld,musl,compiler-rt,libcxx,libcxxabi,clang,host-toolchain,cmake --no-tool-tests --no-test 

untar-tools:
	rm -rf toolchain; mkdir toolchain && tar -xf toolchain.tar.gz -C toolchain

tar-tools:
	cd wasm-waterfall/src/work/ && tar -cf toolchain.tar.gz wasm-install && mv toolchain.tar.gz ../../../

build-musl-waterfall:
	python wasm-waterfall/src/build.py --sync-include musl --build-include musl --no-test --no-tool-tests

build-musl:
	cd musl && python libc.py \
		--clang_dir=/usr/local/code/faasm/toolchain/wasm-install/bin \
		--binaryen_dir=/usr/local/code/faasm/toolchain/wasm-install/bin \
		--compile-to-wasm \
		--musl=/usr/local/code/faasm/musl \
		--out=/tmp/muslwasmlibc.a

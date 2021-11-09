SHELL:=/bin/bash

help:
	@grep -E '^[0-9a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

prepare:  ## set up build environment
	sudo apt install -y libboost1.67-dev cmake
	pushd inotify-cpp && \
	cmake -DBUILD_EXAMPLE=OFF -DBUILD_TEST=OFF -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DUSE_BOOST_FILESYSTEM=OFF . && \
	make -j $$(nproc)

build: ## build tailer
	g++ -std=c++17 -Iinotify-cpp/src/include src/main.cpp -o tailer inotify-cpp/src/libinotify-cpp.a -lpthread

install:  ## install to /usr/local/bin/tailer
	sudo cp -prv tailer /usr/local/bin/tailer

format:  ## format src/main.cpp
	clang-format -i src/main.cpp

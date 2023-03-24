VERSION?=1.0.0.0

BINRARY=build
ENABLE_DEBUG?=true

.PHONY : arm64 centos

all:centos

centos:
	mkdir $(BINRARY) -p
	cd $(BINRARY) && cmake -D DEBUG_mode=$(ENABLE_DEBUG) -D SYSVERSION=$(VERSION) ../ && make -j 4

arm64:
	mkdir $(BINRARY) -p
	cd $(BINRARY) && cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/aarch64.cmake -D DEBUG_mode=$(ENABLE_DEBUG) -D SYSVERSION=$(VERSION) ../ && make -j 4
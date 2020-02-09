DV8_DEPS           = ./deps
DV8_SRC            = ./src
DV8_OUT            = ./bin
HTTPPARSER_INCLUDE = $(DV8_DEPS)/http_parser
PICOHTTP_INCLUDE   = $(DV8_DEPS)/picohttpparser
V8_INCLUDE         = $(DV8_DEPS)/v8/include
V8_DEPS            = $(DV8_DEPS)/v8
JSYS_INCLUDE       = $(DV8_DEPS)/jsys
MINIZ_INCLUDE      = $(DV8_DEPS)/miniz
MBEDTLS_INCLUDE    = $(DV8_DEPS)/mbedtls/include
BUILTINS           = $(DV8_SRC)/builtins
TRACE              = TRACE=0
CCFLAGS            = -D$(TRACE) -I$(PICOHTTP_INCLUDE) -I$(MBEDTLS_INCLUDE) -I$(JSYS_INCLUDE) -I$(HTTPPARSER_INCLUDE) -I$(MINIZ_INCLUDE) -I$(V8_INCLUDE) -I$(BUILTINS) -I$(DV8_SRC) -msse4 -pthread -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -m64 -O3 -g -fno-omit-frame-pointer -fno-rtti -ffast-math -fno-ident -fno-exceptions -fmerge-all-constants -fno-unroll-loops -fno-unwind-tables -fno-math-errno -fno-stack-protector -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -std=gnu++1y
LDFLAGS            = -pthread -m64 -Wl,-z,norelro -Wl,--start-group  $(DV8_OUT)/dv8main.o $(DV8_OUT)/dv8.a $(V8_DEPS)/libv8_monolith.a -Wl,--end-group
CC                 = ccache g++
C                  = ccache gcc

.PHONY: help clean

help: ## display this help message
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

$(DV8_OUT)/buffer.o:
	$(CC) $(CCFLAGS) -c -o $(DV8_OUT)/buffer.o $(DV8_SRC)/builtins/buffer.cc

$(DV8_OUT)/env.o:
	$(CC) $(CCFLAGS) -c -o $(DV8_OUT)/env.o $(DV8_SRC)/builtins/env.cc

$(DV8_OUT)/tty.o:
	$(CC) $(CCFLAGS) -I$(DV8_SRC)/modules/tty -c -o $(DV8_OUT)/tty.o $(DV8_SRC)/modules/tty/tty.cc

$(DV8_OUT)/epoll.o:
	$(CC) $(CCFLAGS) -I$(DV8_SRC)/modules/epoll -c -o $(DV8_OUT)/epoll.o $(DV8_SRC)/modules/epoll/epoll.cc

$(DV8_OUT)/fs.o:
	$(CC) $(CCFLAGS) -I$(DV8_SRC)/modules/fs -c -o $(DV8_OUT)/fs.o $(DV8_SRC)/modules/fs/fs.cc

$(DV8_OUT)/timer.o:
	$(CC) $(CCFLAGS) -I$(DV8_SRC)/modules/timer -c -o $(DV8_OUT)/timer.o $(DV8_SRC)/modules/timer/timer.cc

$(DV8_OUT)/jsyshttp.o:
	$(C) -msse4 -c -o $(DV8_OUT)/picohttpparser.o $(PICOHTTP_INCLUDE)/picohttpparser.c
	$(CC) $(CCFLAGS) -I$(DV8_SRC)/modules/jsyshttp -c -o $(DV8_OUT)/jsyshttp.o $(DV8_SRC)/modules/jsyshttp/jsyshttp.cc

$(DV8_OUT)/modules.o:
	$(CC) $(CCFLAGS) -c -o $(DV8_OUT)/modules.o $(DV8_SRC)/modules.cc

$(DV8_OUT)/dv8main.o:
	$(CC) $(CCFLAGS) -c -o $(DV8_OUT)/dv8main.o $(DV8_SRC)/dv8_main.cc

$(DV8_OUT)/dv8.o:
	$(CC) $(CCFLAGS) -c -o $(DV8_OUT)/dv8.o $(DV8_SRC)/dv8.cc

$(DV8_SRC)/builtins.h: src/main.js
	./builtins.sh

$(DV8_OUT)/dv8.a:
	rm -f $(DV8_OUT)/dv8.a
	ar crsT $(DV8_OUT)/dv8.a $(DV8_OUT)/buffer.o $(DV8_OUT)/env.o $(DV8_OUT)/dv8.o $(DV8_OUT)/modules.o $(DV8_OUT)/tty.o $(DV8_OUT)/epoll.o $(DV8_OUT)/timer.o $(DV8_OUT)/fs.o $(DV8_OUT)/jsyshttp.o $(DV8_OUT)/picohttpparser.o

$(DV8_OUT)/dv8:
	$(CC) $(LDFLAGS) -static -o $(DV8_OUT)/dv8

dv8: $(DV8_SRC)/builtins.h $(DV8_OUT)/buffer.o $(DV8_OUT)/env.o $(DV8_OUT)/tty.o $(DV8_OUT)/timer.o $(DV8_OUT)/epoll.o $(DV8_OUT)/fs.o $(DV8_OUT)/jsyshttp.o $(DV8_OUT)/modules.o $(DV8_OUT)/dv8main.o $(DV8_OUT)/dv8.o $(DV8_OUT)/dv8.a $(DV8_OUT)/dv8 ## dv8 runtime

clean: ## clean the generated artifacts
	rm -f bin/dv8 bin/*.o bin/*.a src/builtins.h

.DEFAULT_GOAL := help

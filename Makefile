# ===================================================================
# CMake Wrapper Makefile
# ===================================================================
APP         := vmc
SHELL       := /bin/bash
.SHELLFLAGS := -o pipefail -c
export CLICOLOR_FORCE := 1
RELPATH     := 2>&1 | sed "s|$(CURDIR)/||g"

# make -B / make -S, or B=1 / S=N
MAKE_LETTERS := $(firstword -$(MAKEFLAGS))
B ?= $(if $(findstring B,$(MAKE_LETTERS)),1)
S ?= $(if $(findstring S,$(MAKE_LETTERS)),2)
TOGGLES := $(if $(B),-D_TIME_REPORT=ON) $(if $(S),-D_MAX_ERRORS=$(S))

asan_ENV  := ASAN_OPTIONS=detect_leaks=0:color=always:abort_on_error=1:halt_on_error=1
ausan_ENV := $(asan_ENV) UBSAN_OPTIONS=color=always:print_stacktrace=1:halt_on_error=1

# specifies the default target instead of just picking the first one
.DEFAULT_GOAL := run

build-%:
	@cmake --preset $* $(TOGGLES) > /dev/null
	@cmake --build build/$* $(RELPATH)

run-%: build-%
ifeq ($(B),1)
	@echo "-B flag set! Compile time diagnostics will be printed."
endif
ifeq ($(S),2)
	@echo "-S flag set! Will fail on the first error."
endif
	@$($*_ENV) ./build/$*/$(APP)

build: build-dev
run:   run-dev
debug: build-dev
	lldb -o run -- build/dev/$(APP)
clean:
	cmake --build build/dev --target clean
clean-all:
	rm -rf build
.PHONY: build run debug clean clean-all help ? h

?: help
h: help
help:
	@for i in $$(seq $$(tput cols)); do printf "▂"; done
	@tput sgr0; tput bold;
	@echo "USAGE:"
	@tput sgr0;
	@echo "    make [flags] <target>"
	@for i in $$(seq $$(tput cols)); do printf "▂"; done
	@tput sgr0; tput bold; tput sitm;
	@echo "FLAGS:"
	@tput sgr0;
	@echo "    -B (B)enchmark compile time   -> Shows the time taken to compile each file."
	@echo "    -S (S)ingle error             -> fail outright immediately on the first error."
	@for i in $$(seq $$(tput cols)); do printf "▂"; done;
	@tput sgr0; tput bold; tput sitm;
	@echo "TARGETS:"
	@tput sgr0;
	@echo "    build                         -> builds the default flavour (dev)."
	@echo "    run                           -> builds the default flavour (dev), and runs the executable."
	@echo '    build-*                       -> builds with a specific flavour (*)'
	@echo '    run-*                         -> builds with a specific flavour (*), and runs the executable.'
	@echo "    debug                         -> builds the default flavour (dev), runs your debugger on that."
	@echo "    clean                         -> clears the build folder for the default flavour (dev)."
	@echo "    clean-all                     -> clears ALL build folders."
	@echo "    help                          -> prints this message."
	@echo ""

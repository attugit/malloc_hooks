.PHONY: all

all:
	g++ -Wno-deprecated-declarations -std=c++11 -O3 -shared -fPIC -o mhook.so malloc_hooks.cc

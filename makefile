.PHONY: debug
debug: main.cpp
	g++ -std=c++17 -O0 -g main.cpp -o maicomp

.PHONY: release
release: main.cpp
	g++ -std=c++17 -O3 main.cpp -o maicomp

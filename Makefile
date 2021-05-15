.PHONY: debug
debug: CXXFLAGS := -O0 -Wall -g
debug: maicomp

.PHONY: release
release: CXXFLAGS := -O3
release: maicomp

maicomp: main.cpp
	g++ -std=c++17 $(CXXFLAGS) main.cpp -o maicomp

clean:
	rm -rf maicomp maicomp.dSYM

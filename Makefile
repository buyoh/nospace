.PHONY: debug
debug: CXXFLAGS := -O0 -Wall -g
debug: maicomp

.PHONY: release
release: CXXFLAGS := -O3
release: maicomp

CPPSTD ?= c++17

maicomp: main.cpp
	$(CXX) -std=$(CPPSTD) $(CXXFLAGS) main.cpp -o maicomp

clean:
	rm -rf maicomp maicomp.dSYM

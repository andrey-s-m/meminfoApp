CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

TARGET := meminfoApp

SRCS := $(wildcard *.cc)
OBJS := $(SRCS:.cc=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean

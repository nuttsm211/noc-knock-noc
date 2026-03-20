CXX = g++
CXXFLAGS = -std=c++17 -O2

# Homebrew SystemC location for macOS
SYSTEMC_HOME ?= /opt/homebrew/opt/systemc

INCLUDES = -I$(SYSTEMC_HOME)/include -I.
LIBS     = -L$(SYSTEMC_HOME)/lib -lsystemc -lm
RPATH    = -Wl,-rpath,$(SYSTEMC_HOME)/lib

PROGRAM = noc.x
SRCS = arbiter.cpp buf_fifo.cpp crossbar.cpp router.cpp sink.cpp source.cpp main_noc.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(PROGRAM)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(PROGRAM): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OBJS) $(LIBS) $(RPATH) -o $(PROGRAM)

clean:
	rm -f $(OBJS) $(PROGRAM) graph.vcd graph.vcd.*
systemc network on chip simulator

a simple packet switched noc simulator built in systemc.
right now it runs a small 1x2 mesh with two routers just to verify routing, buffering and packet movement.

packets are split into flits and move using deterministic xy routing with wormhole style flow control.
traffic is controlled so both nodes send packets to each other and you can observe timing and delays.

current features

* fifo input buffering
* basic arbiter and crossbar switching
* fixed packet size flits
* simple latency measurement
* waveform trace output

goal is to scale this into a larger mesh and experiment with traffic patterns and performance.

build
make

run
./noc.x

requires systemc installed and paths updated in makefile if needed.

# noc-knock-noc

Upscaled from an earlier NoC version into a 4x4 torus model with wrap-aware routing.

reduced average flit delay from 50312.5 ns to 32500 ns under wrap-stress traffic, with full delivery of 160 flits.




# next?

upscale more maybe, depends tbh
# build

```
make clean
make
./noc.x
./noc.x wrap
./noc.x neighbour

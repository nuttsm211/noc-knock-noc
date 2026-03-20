#include "router.h"
#define SIM_NUM 100000

void router::func()
{
    int sim_count = 0;

    while (sim_count++ < SIM_NUM)
    {
        wait();

        if (in0.event()) { pkt_sent++; }
        if (in1.event()) { pkt_sent++; }
        if (in2.event()) { pkt_sent++; }
        if (in3.event()) { pkt_sent++; }
        if (in4.event()) { pkt_sent++; }
    }
}
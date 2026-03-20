#ifndef SOURCE_H
#define SOURCE_H

#include "packet.h"

SC_MODULE(source) {
    sc_out<packet>       packet_out;
    sc_in<sc_uint<4> >   source_id;
    sc_in<sc_uint<2> >   traffic_mode;   // 0 = uniform, 1 = neighbouring
    sc_in<bool>          ach_in;
    sc_in_clk            CLK;

    int pkt_snt;                 // number of flits sent
    int max_flits_to_send;       // finite traffic for controlled simulation
    double total_send_time_ns;

    void func();

    SC_CTOR(source)
    {
        SC_CTHREAD(func, CLK.pos());
        pkt_snt = 0;
        max_flits_to_send = 10;  // default per source
        total_send_time_ns = 0.0;
    }
};

#endif
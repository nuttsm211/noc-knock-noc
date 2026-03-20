#ifndef SINK_H
#define SINK_H

#include "packet.h"

SC_MODULE(sink) {
    sc_in<packet>       packet_in;
    sc_out<bool>        ack_out;
    sc_in<sc_uint<4> >  sink_id;
    sc_in<bool>         sclk;

    int pkt_recv;
    double total_delay_ns;
    double last_recv_time_ns;

    void receive_data();

    SC_CTOR(sink) {
        SC_METHOD(receive_data);
        dont_initialize();
        sensitive << packet_in;
        sensitive << sclk.pos();

        pkt_recv = 0;
        total_delay_ns = 0.0;
        last_recv_time_ns = 0.0;
    }
};

#endif
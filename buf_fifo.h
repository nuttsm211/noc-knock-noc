#ifndef BUF_FIFO_H
#define BUF_FIFO_H

#include "packet.h"

SC_MODULE(buf_fifo) {
    sc_in<packet>        wr;
    sc_out<packet>       re;
    sc_in<sc_uint<1> >   grant;
    sc_out<sc_uint<7> >  req;
    sc_out<bool>         ack;
    sc_in<bool>          bclk;

    void func();

    SC_CTOR(buf_fifo)
    {
        SC_THREAD(func);
        sensitive << wr;
        sensitive << bclk.pos();
    }
};

struct fifo {
public:
    packet registers[4];
    bool full;
    bool empty;
    int reg_num;

    fifo()
    {
        full = false;
        empty = true;
        reg_num = 0;

        for (int i = 0; i < 4; i++) {
            registers[i].data = 0;
            registers[i].id = 0;
            registers[i].dest = 0;
            registers[i].pkt_clk = 0;
            registers[i].h_t = 0;
            registers[i].send_time = 0;
        }
    }

    void packet_in(const packet& data_packet);
    packet packet_out();
};

#endif
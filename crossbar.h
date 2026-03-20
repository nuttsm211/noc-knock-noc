#ifndef CROSSBAR_H
#define CROSSBAR_H

#include "packet.h"

SC_MODULE(crossbar) {

    sc_in<packet> i0;
    sc_in<packet> i1;
    sc_in<packet> i2;
    sc_in<packet> i3;
    sc_in<packet> i4;

    sc_out<packet> o0;
    sc_out<packet> o1;
    sc_out<packet> o2;
    sc_out<packet> o3;
    sc_out<packet> o4;

    sc_in<sc_uint<15> > config;

    void func();

    SC_CTOR(crossbar)
    {
        SC_THREAD(func);
        sensitive << i0 << i1 << i2 << i3 << i4;
    }
};

#endif
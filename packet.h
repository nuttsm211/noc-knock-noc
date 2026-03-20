#ifndef PACKET
#define PACKET

#include "systemc.h"

struct packet {
    sc_uint<11> data;
    sc_uint<4>  id;         // packet source ID
    sc_uint<4>  dest;       // packet destination ID
    sc_uint<1>  pkt_clk;    // imaginary clock bit
    sc_uint<1>  h_t;        // 1 for tail flit
    sc_uint<32> send_time;  // send time in ns for delay calculation

    inline bool operator == (const packet& rhs) const
    {
        return (rhs.data      == data &&
                rhs.id        == id &&
                rhs.dest      == dest &&
                rhs.pkt_clk   == pkt_clk &&
                rhs.h_t       == h_t &&
                rhs.send_time == send_time);
    }
};

inline
ostream& operator << (ostream& os, const packet& a)
{
    os << "{data=" << a.data
       << ", id=" << a.id
       << ", dest=" << a.dest
       << ", pkt_clk=" << a.pkt_clk
       << ", h_t=" << a.h_t
       << ", send_time=" << a.send_time
       << "}";
    return os;
}

inline
void
#if defined(SC_API_VERSION_STRING)
sc_trace(sc_trace_file* tf, const packet& a, const std::string& name)
#else
sc_trace(sc_trace_file* tf, const packet& a, const sc_string& name)
#endif
{
    sc_trace(tf, a.data,      name + ".data");
    sc_trace(tf, a.id,        name + ".id");
    sc_trace(tf, a.dest,      name + ".dest");
    sc_trace(tf, a.pkt_clk,   name + ".pkt_clk");
    sc_trace(tf, a.h_t,       name + ".h_t");
    sc_trace(tf, a.send_time, name + ".send_time");
}

#endif
#include "source.h"

static sc_uint<4> choose_destination(sc_uint<4> sid, sc_uint<2> mode)
{
    sc_uint<4> dest = 0;
    if (mode == 0) {
        // uniform permutation (original)
        dest = (sid + 5) % 16;
    } else if (mode == 1) {
        // neighbouring (original)
        dest = sid ^ 1;
   } else {
    // wrap-exploit: dest is 3 hops direct but 1 hop via wrap
    // node at (row, col) -> same row, (col + 3) % 4
    // direct = 3 hops east, wrap = 1 hop west
    // wrap-aware routing picks west wrap = 1 hop
    // conservative XY picks east direct = 3 hops
        int row = (int)sid / 4;
        int col = (int)sid % 4;
        int dcol = (col + 3) % 4;
        dest = (sc_uint<4>)(row * 4 + dcol);
   }
    return dest;
}

void source::func()
{
    packet v_packet_out;
    v_packet_out.data = 1000;
    v_packet_out.pkt_clk = 0;
    v_packet_out.h_t = 0;
    v_packet_out.send_time = 0;
    v_packet_out.id = 0;
    v_packet_out.dest = 0;

    while (true)
    {
        wait();

        if (pkt_snt >= max_flits_to_send) {
            continue;
        }

        if (!ach_in.read())
        {
            sc_uint<4> sid = source_id.read();
            sc_uint<4> dest = choose_destination(sid, traffic_mode.read());

            v_packet_out.data = v_packet_out.data + sid + 1;
            v_packet_out.id = sid;
            v_packet_out.dest = dest;
            v_packet_out.pkt_clk = ~v_packet_out.pkt_clk;
            v_packet_out.h_t = 0;
//each got them 5 flits jit HBT time
            pkt_snt++;
            if ((pkt_snt % 5) == 0) {
                v_packet_out.h_t = 1;
            }

            v_packet_out.send_time = (sc_uint<32>) sc_time_stamp().to_double();
            total_send_time_ns += sc_time_stamp().to_double();

            packet_out.write(v_packet_out);

            cout << "New Pkt Sent: " << (int)v_packet_out.data
                 << " source: " << (int)v_packet_out.id
                 << " destination: " << (int)v_packet_out.dest
                 << " send_time(ns): " << (unsigned int)v_packet_out.send_time
                 << " tail: " << (int)v_packet_out.h_t
                 << endl;
        }
    }
}
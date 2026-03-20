#include "source.h"

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

        // stop injecting once the desired number of flits is reached
        if (pkt_snt >= max_flits_to_send) {
            continue;
        }

        // only inject when upstream router says input FIFO is not full
        if (!ach_in.read())
        {
            sc_uint<4> sid = source_id.read();

            v_packet_out.data = v_packet_out.data + sid + 1;
            v_packet_out.id = sid;

            // 1x2 clean traffic pattern:
            // source 0 -> sink 1
            // source 1 -> sink 0
            if (sid == 0)
                v_packet_out.dest = 1;
            else
                v_packet_out.dest = 0;

            v_packet_out.pkt_clk = ~v_packet_out.pkt_clk;
            v_packet_out.h_t = 0;

            pkt_snt++;

            // packet size = 5 flits, last flit is tail
            if ((pkt_snt % 5) == 0)
                v_packet_out.h_t = 1;

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
#include "sink.h"

void sink::receive_data()
{
    packet v_packet;

    if (sclk.event()) {
        ack_out.write(false);
    }

    if (packet_in.event())
    {
        v_packet = packet_in.read();

        pkt_recv++;
        ack_out.write(true);

        last_recv_time_ns = sc_time_stamp().to_double();
        double flit_delay_ns = last_recv_time_ns - (double)v_packet.send_time;
        total_delay_ns += flit_delay_ns;

        cout << "New Pkt Received: " << (int)v_packet.data
             << " source: " << (int)v_packet.id
             << " sink: " << (int)sink_id.read()
             << " recv_time(ns): " << last_recv_time_ns
             << " send_time(ns): " << (unsigned int)v_packet.send_time
             << " flit_delay(ns): " << flit_delay_ns
             << " tail: " << (int)v_packet.h_t
             << endl;
    }
}
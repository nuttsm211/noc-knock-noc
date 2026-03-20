#include "systemc.h"
#include <iostream>
#include <string>
#include <stdio.h>

#include "packet.h"
#include "source.h"
#include "sink.h"
#include "router.h"

int sc_main(int argc, char *argv[])
{
    sc_signal<packet> si_source[2];
    sc_signal<packet> si_input[2];
    sc_signal<packet> si_output[2];
    sc_signal<packet> si_sink[2];

    sc_signal<packet> si_zero[12];

    sc_signal<bool> si_ack_src[2], si_ack_ou[2];
    sc_signal<bool> si_ack_sink[2], si_ack_in[2];
    sc_signal<bool> si_ack_zero[12];

    sc_signal<sc_uint<4> > id0, id1;

    // required interim condition: source clock = router clock
    sc_clock s_clock("S_CLOCK", 5, SC_NS, 0.5, 0.0, SC_NS);
    sc_clock r_clock("R_CLOCK", 5, SC_NS, 0.5, 0.0, SC_NS);
    sc_clock d_clock("D_CLOCK", 5, SC_NS, 0.5, 0.0, SC_NS);

    source source0("source0");
    source0.packet_out(si_source[0]);
    source0.source_id(id0);
    source0.ach_in(si_ack_src[0]);
    source0.CLK(s_clock);

    source source1("source1");
    source1.packet_out(si_source[1]);
    source1.source_id(id1);
    source1.ach_in(si_ack_src[1]);
    source1.CLK(s_clock);

    // controlled finite traffic for cleaner interim results
    source0.max_flits_to_send = 20; // 4 packets
    source1.max_flits_to_send = 20; // 4 packets

    router router0("router0");
    router0.in0(si_source[0]);
    router0.in1(si_zero[0]);
    router0.in2(si_input[0]);
    router0.in3(si_zero[1]);
    router0.in4(si_zero[2]);

    router0.router_id(id0);

    router0.out0(si_sink[0]);
    router0.out1(si_zero[3]);
    router0.out2(si_output[0]);
    router0.out3(si_zero[4]);
    router0.out4(si_zero[5]);

    router0.inack0(si_ack_sink[0]);
    router0.inack1(si_ack_zero[0]);
    router0.inack2(si_ack_in[0]);
    router0.inack3(si_ack_zero[1]);
    router0.inack4(si_ack_zero[2]);

    router0.outack0(si_ack_src[0]);
    router0.outack1(si_ack_zero[3]);
    router0.outack2(si_ack_ou[0]);
    router0.outack3(si_ack_zero[4]);
    router0.outack4(si_ack_zero[5]);

    router0.rclk(r_clock);

    router router1("router1");
    router1.in0(si_source[1]);
    router1.in1(si_zero[6]);
    router1.in2(si_zero[7]);
    router1.in3(si_zero[8]);
    router1.in4(si_output[0]);

    router1.router_id(id1);

    router1.out0(si_sink[1]);
    router1.out1(si_zero[9]);
    router1.out2(si_zero[10]);
    router1.out3(si_zero[11]);
    router1.out4(si_input[0]);

    router1.inack0(si_ack_sink[1]);
    router1.inack1(si_ack_zero[6]);
    router1.inack2(si_ack_zero[7]);
    router1.inack3(si_ack_zero[8]);
    router1.inack4(si_ack_ou[0]);

    router1.outack0(si_ack_src[1]);
    router1.outack1(si_ack_zero[9]);
    router1.outack2(si_ack_zero[10]);
    router1.outack3(si_ack_zero[11]);
    router1.outack4(si_ack_in[0]);

    router1.rclk(r_clock);

    sink sink0("sink0");
    sink0.packet_in(si_sink[0]);
    sink0.ack_out(si_ack_sink[0]);
    sink0.sink_id(id0);
    sink0.sclk(d_clock);

    sink sink1("sink1");
    sink1.packet_in(si_sink[1]);
    sink1.ack_out(si_ack_sink[1]);
    sink1.sink_id(id1);
    sink1.sclk(d_clock);

    sc_trace_file *tf = sc_create_vcd_trace_file("graph");

    sc_trace(tf, s_clock, "s_clock");
    sc_trace(tf, r_clock, "r_clock");
    sc_trace(tf, d_clock, "d_clock");

    sc_trace(tf, si_source[0], "si_source0");
    sc_trace(tf, si_source[1], "si_source1");
    sc_trace(tf, si_sink[0], "si_sink0");
    sc_trace(tf, si_sink[1], "si_sink1");
    sc_trace(tf, si_output[0], "si_output0_r0_to_r1");
    sc_trace(tf, si_input[0], "si_input0_r1_to_r0");

    sc_trace(tf, si_ack_src[0], "si_ack_src0");
    sc_trace(tf, si_ack_src[1], "si_ack_src1");
    sc_trace(tf, si_ack_sink[0], "si_ack_sink0");
    sc_trace(tf, si_ack_sink[1], "si_ack_sink1");

    id0.write(0);
    id1.write(1);

    cout << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << "1x2 mesh NoC simulator containing 2 routers" << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << "Source clock = router clock = sink clock = 5 ns" << endl;
    cout << "Controlled traffic: each source sends 20 flits" << endl;
    cout << "Press Return to start simulation..." << endl;
    getchar();

    // run long enough to inject and drain all traffic
    sc_start(2000, SC_NS);

    sc_close_vcd_trace_file(tf);

    int total_sent = source0.pkt_snt + source1.pkt_snt;
    int total_recv = sink0.pkt_recv + sink1.pkt_recv;

    double total_delay = sink0.total_delay_ns + sink1.total_delay_ns;
    double avg_delay = 0.0;
    if (total_recv > 0) {
        avg_delay = total_delay / (double)total_recv;
    }

    cout << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << "End of NoC simulation" << endl;
    cout << "Total flits sent: " << total_sent << endl;
    cout << "Total flits received: " << total_recv << endl;
    cout << "Average flit delay (ns): " << avg_delay << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << "Press Return to end the simulation..." << endl;
    getchar();

    return 0;
}
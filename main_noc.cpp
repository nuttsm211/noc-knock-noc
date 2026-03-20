#include "systemc.h"
#include <iostream>
#include <string>
#include <stdio.h>

#include "packet.h"
#include "source.h"
#include "sink.h"
#include "router.h"

using namespace std;

static const int N = 4;
static const int NUM_NODES = 16;
static const int FLITS_PER_SOURCE = 10;

int sc_main(int argc, char *argv[])
{
    sc_uint<2> selected_mode = 0; // 0 = uniform, 1 = neighbouring

    if (argc > 1) {
        string mode = argv[1];
        if (mode == "neighbour" || mode == "neighbor" || mode == "n") {
            selected_mode = 1;
        } else {
            selected_mode = 0;
        }
    }

    sc_clock s_clock("S_CLOCK", 5, SC_NS, 0.5, 0.0, SC_NS);
    sc_clock r_clock("R_CLOCK", 5, SC_NS, 0.5, 0.0, SC_NS);
    sc_clock d_clock("D_CLOCK", 5, SC_NS, 0.5, 0.0, SC_NS);

    sc_signal<sc_uint<4> > node_id[NUM_NODES];
    sc_signal<sc_uint<2> > traffic_mode_sig;

    sc_signal<packet> local_src_pkt[NUM_NODES];
    sc_signal<packet> local_sink_pkt[NUM_NODES];
    sc_signal<bool>   local_src_ack[NUM_NODES];
    sc_signal<bool>   local_sink_ack[NUM_NODES];

    // Horizontal links
    sc_signal<packet> east_pkt[N][N-1];   // (r,c) -> (r,c+1)
    sc_signal<bool>   east_ack[N][N-1];

    sc_signal<packet> west_pkt[N][N-1];   // (r,c+1) -> (r,c)
    sc_signal<bool>   west_ack[N][N-1];

    // Vertical links
    sc_signal<packet> south_pkt[N-1][N];  // (r,c) -> (r+1,c)
    sc_signal<bool>   south_ack[N-1][N];

    sc_signal<packet> north_pkt[N-1][N];  // (r+1,c) -> (r,c)
    sc_signal<bool>   north_ack[N-1][N];

    // Unique dummy signals for unused boundary ports
    sc_signal<packet> dummy_pkt[NUM_NODES][5];
    sc_signal<bool>   dummy_ack[NUM_NODES][5];

    source* src[NUM_NODES];
    sink* snk[NUM_NODES];
    router* rtr[NUM_NODES];

    for (int i = 0; i < NUM_NODES; i++) {
        string sname = "source" + to_string(i);
        string kname = "sink" + to_string(i);
        string rname = "router" + to_string(i);

        src[i] = new source(sname.c_str());
        snk[i] = new sink(kname.c_str());
        rtr[i] = new router(rname.c_str());
    }

    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            int idx = r * N + c;

            node_id[idx].write(idx);

            src[idx]->packet_out(local_src_pkt[idx]);
            src[idx]->source_id(node_id[idx]);
            src[idx]->traffic_mode(traffic_mode_sig);
            src[idx]->ach_in(local_src_ack[idx]);
            src[idx]->CLK(s_clock);
            src[idx]->max_flits_to_send = FLITS_PER_SOURCE;

            snk[idx]->packet_in(local_sink_pkt[idx]);
            snk[idx]->ack_out(local_sink_ack[idx]);
            snk[idx]->sink_id(node_id[idx]);
            snk[idx]->sclk(d_clock);

            rtr[idx]->router_id(node_id[idx]);
            rtr[idx]->rclk(r_clock);

            // Local source/sink
            rtr[idx]->in0(local_src_pkt[idx]);
            rtr[idx]->out0(local_sink_pkt[idx]);
            rtr[idx]->outack0(local_src_ack[idx]);
            rtr[idx]->inack0(local_sink_ack[idx]);

            // North side: in1 / out1
            if (r > 0) {
                rtr[idx]->in1(south_pkt[r-1][c]);
                rtr[idx]->inack1(south_ack[r-1][c]);
            } else {
                rtr[idx]->in1(dummy_pkt[idx][1]);
                rtr[idx]->inack1(dummy_ack[idx][1]);
            }

            if (r < N - 1) {
                rtr[idx]->out3(south_pkt[r][c]);
                rtr[idx]->outack3(south_ack[r][c]);
            } else {
                rtr[idx]->out3(dummy_pkt[idx][3]);
                rtr[idx]->outack3(dummy_ack[idx][3]);
            }

            // South side: in3 / out3 receives from south neighbor via north_pkt
            if (r < N - 1) {
                rtr[idx]->in3(north_pkt[r][c]);
                rtr[idx]->inack3(north_ack[r][c]);
            } else {
                rtr[idx]->in3(dummy_pkt[idx][4]);
                rtr[idx]->inack3(dummy_ack[idx][4]);
            }

            if (r > 0) {
                rtr[idx]->out1(north_pkt[r-1][c]);
                rtr[idx]->outack1(north_ack[r-1][c]);
            } else {
                rtr[idx]->out1(dummy_pkt[idx][0]);
                rtr[idx]->outack1(dummy_ack[idx][0]);
            }

            // East side: in2 gets packets from east neighbor travelling west
            if (c < N - 1) {
                rtr[idx]->in2(west_pkt[r][c]);
                rtr[idx]->inack2(west_ack[r][c]);
            } else {
                rtr[idx]->in2(dummy_pkt[idx][2]);
                rtr[idx]->inack2(dummy_ack[idx][2]);
            }

            if (c < N - 1) {
                rtr[idx]->out2(east_pkt[r][c]);
                rtr[idx]->outack2(east_ack[r][c]);
            } else {
                rtr[idx]->out2(dummy_pkt[idx][2]);
                rtr[idx]->outack2(dummy_ack[idx][2]);
            }

            // West side: in4 gets packets from west neighbor travelling east
            if (c > 0) {
                rtr[idx]->in4(east_pkt[r][c-1]);
                rtr[idx]->inack4(east_ack[r][c-1]);
            } else {
                rtr[idx]->in4(dummy_pkt[idx][3]);
                rtr[idx]->inack4(dummy_ack[idx][3]);
            }

            if (c > 0) {
                rtr[idx]->out4(west_pkt[r][c-1]);
                rtr[idx]->outack4(west_ack[r][c-1]);
            } else {
                rtr[idx]->out4(dummy_pkt[idx][4]);
                rtr[idx]->outack4(dummy_ack[idx][4]);
            }
        }
    }

    traffic_mode_sig.write(selected_mode);

    sc_trace_file *tf = sc_create_vcd_trace_file("graph");

    sc_trace(tf, s_clock, "s_clock");
    sc_trace(tf, r_clock, "r_clock");
    sc_trace(tf, d_clock, "d_clock");
    sc_trace(tf, traffic_mode_sig, "traffic_mode");

    for (int i = 0; i < NUM_NODES; i++) {
        sc_trace(tf, local_src_pkt[i], "src_pkt_" + to_string(i));
        sc_trace(tf, local_sink_pkt[i], "sink_pkt_" + to_string(i));
        sc_trace(tf, local_src_ack[i], "src_ack_" + to_string(i));
        sc_trace(tf, local_sink_ack[i], "sink_ack_" + to_string(i));
    }

    cout << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << "4x4 mesh NoC simulator containing 16 routers" << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << "Traffic mode: " << (selected_mode == 0 ? "uniform" : "neighbouring") << endl;
    cout << "Each source sends " << FLITS_PER_SOURCE << " flits" << endl;
    cout << "Press Return to start simulation..." << endl;
    getchar();

    const int total_target_flits = NUM_NODES * FLITS_PER_SOURCE;
    int total_recv = 0;
    int safety_steps = 0;

    while (total_recv < total_target_flits && safety_steps < 5000) {
        sc_start(100, SC_NS);
        total_recv = 0;
        for (int i = 0; i < NUM_NODES; i++) {
            total_recv += snk[i]->pkt_recv;
        }
        safety_steps++;
    }

    sc_close_vcd_trace_file(tf);

    int total_sent = 0;
    total_recv = 0;
    double total_delay = 0.0;

    for (int i = 0; i < NUM_NODES; i++) {
        total_sent += src[i]->pkt_snt;
        total_recv += snk[i]->pkt_recv;
        total_delay += snk[i]->total_delay_ns;
    }

    double avg_delay = 0.0;
    if (total_recv > 0) {
        avg_delay = total_delay / (double)total_recv;
    }

    cout << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << "End of 4x4 mesh NoC simulation" << endl;
    cout << "Total flits sent: " << total_sent << endl;
    cout << "Total flits received: " << total_recv << endl;
    cout << "Average flit delay (ns): " << avg_delay << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << "Press Return to end the simulation..." << endl;
    getchar();

    return 0;
}
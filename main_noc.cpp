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
        } else if (mode == "wrap" || mode == "w") {
            selected_mode = 2;
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

    // Directed torus links
    // Each array element is owned by exactly one router output.
    sc_signal<packet> north_pkt[N][N];
    sc_signal<bool>   north_ack[N][N];

    sc_signal<packet> east_pkt[N][N];
    sc_signal<bool>   east_ack[N][N];

    sc_signal<packet> south_pkt[N][N];
    sc_signal<bool>   south_ack[N][N];

    sc_signal<packet> west_pkt[N][N];
    sc_signal<bool>   west_ack[N][N];

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

            int north_r = (r - 1 + N) % N;
            int south_r = (r + 1) % N;
            int west_c  = (c - 1 + N) % N;
            int east_c  = (c + 1) % N;

            node_id[idx].write(idx);

            // Source
            src[idx]->packet_out(local_src_pkt[idx]);
            src[idx]->source_id(node_id[idx]);
            src[idx]->traffic_mode(traffic_mode_sig);
            src[idx]->ach_in(local_src_ack[idx]);
            src[idx]->CLK(s_clock);
            src[idx]->max_flits_to_send = FLITS_PER_SOURCE;

            // Sink
            snk[idx]->packet_in(local_sink_pkt[idx]);
            snk[idx]->ack_out(local_sink_ack[idx]);
            snk[idx]->sink_id(node_id[idx]);
            snk[idx]->sclk(d_clock);

            // Router basic ports
            rtr[idx]->router_id(node_id[idx]);
            rtr[idx]->rclk(r_clock);

            // Local port
            rtr[idx]->in0(local_src_pkt[idx]);
            rtr[idx]->out0(local_sink_pkt[idx]);
            rtr[idx]->outack0(local_src_ack[idx]);
            rtr[idx]->inack0(local_sink_ack[idx]);

            // NORTH input:
            // packet comes from north neighbor's SOUTH output
            rtr[idx]->in1(south_pkt[north_r][c]);
            // ack goes back to north neighbor for that south-directed link
            rtr[idx]->outack1(south_ack[north_r][c]);

            // EAST input:
            // packet comes from east neighbor's WEST output
            rtr[idx]->in2(west_pkt[r][east_c]);
            // ack goes back to east neighbor for that west-directed link
            rtr[idx]->outack2(west_ack[r][east_c]);

            // SOUTH input:
            // packet comes from south neighbor's NORTH output
            rtr[idx]->in3(north_pkt[south_r][c]);
            // ack goes back to south neighbor for that north-directed link
            rtr[idx]->outack3(north_ack[south_r][c]);

            // WEST input:
            // packet comes from west neighbor's EAST output
            rtr[idx]->in4(east_pkt[r][west_c]);
            // ack goes back to west neighbor for that east-directed link
            rtr[idx]->outack4(east_ack[r][west_c]);

            // NORTH output:
            // this router drives its own north-directed link
            rtr[idx]->out1(north_pkt[r][c]);
            // downstream availability for north output
            rtr[idx]->inack1(north_ack[r][c]);

            // EAST output:
            rtr[idx]->out2(east_pkt[r][c]);
            rtr[idx]->inack2(east_ack[r][c]);

            // SOUTH output:
            rtr[idx]->out3(south_pkt[r][c]);
            rtr[idx]->inack3(south_ack[r][c]);

            // WEST output:
            rtr[idx]->out4(west_pkt[r][c]);
            rtr[idx]->inack4(west_ack[r][c]);
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
    cout << "4x4 torus NoC simulator containing 16 routers" << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << "Traffic mode: ";
        if      (selected_mode == 0) cout << "uniform";
        else if (selected_mode == 1) cout << "neighbouring";
        else                         cout << "wrap-stress (diagonal)";
    cout << endl;
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
    cout << "End of 4x4 torus NoC simulation" << endl;
    cout << "Total flits sent: " << total_sent << endl;
    cout << "Total flits received: " << total_recv << endl;
    cout << "Average flit delay (ns): " << avg_delay << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << "Press Return to end the simulation..." << endl;
    getchar();

    return 0;
}
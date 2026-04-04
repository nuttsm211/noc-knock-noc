#undef SC_INCLUDE_FX

#include "packet.h"
#include "arbiter.h"

static sc_uint<3> compute_route(sc_uint<4> router_id, sc_uint<4> dest_id)
{
    int rx = (int)router_id.range(1, 0);
    int ry = (int)router_id.range(3, 2);
    int dx = (int)dest_id.range(1, 0);
    int dy = (int)dest_id.range(3, 2);
    const int N = 4;

    // X dimension: compare direct distance vs wrap distance
    int x_direct = dx - rx;         // positive = go east, negative = go west
    int x_wrap   = x_direct > 0
                   ? x_direct - N   // going east directly, wrap goes west
                   : x_direct + N;  // going west directly, wrap goes east

    // pick shorter X path
    int x_step = 0;
    if (dx != rx) {
        if (abs(x_direct) <= abs(x_wrap))
            x_step = x_direct > 0 ? 1 : -1;   // direct
        else
            x_step = x_wrap > 0 ? 1 : -1;      // wrap is shorter
    }
    

    // Y dimension: compare direct distance vs wrap distance
    int y_direct = dy - ry;
    int y_wrap   = y_direct > 0
                   ? y_direct - N
                   : y_direct + N;

    int y_step = 0;
    if (dy != ry) {
        if (abs(y_direct) <= abs(y_wrap))
            y_step = y_direct > 0 ? 1 : -1;
        else
            y_step = y_wrap > 0 ? 1 : -1;
    }

    // Dimension-ordered: resolve X first, then Y
    // Turn restriction: only allow east and south turns at wrap boundaries
    // This breaks cyclic dependencies while still using shorter wrap paths
    if (x_step == 1)  return 3; // east
    if (x_step == -1) return 5; // west
    if (y_step == 1)  return 4; // south
    if (y_step == -1) return 2; // north
    return 1;                   // local
}

void arbiter::func()
{
    bool connected_input[5];
    sc_uint<3> reserved_route[5];
    int output_owner[5];

    for (int i = 0; i < 5; i++) {
        connected_input[i] = false;
        reserved_route[i] = 0;
        output_owner[i] = -1;
    }

    while (true)
    {
        wait();

        grant0.write(0);
        grant1.write(0);
        grant2.write(0);
        grant3.write(0);
        grant4.write(0);

        sc_uint<15> v_select = 0;

        bool output_free[5];
        output_free[0] = !free_out0.read();
        output_free[1] = !free_out1.read();
        output_free[2] = !free_out2.read();
        output_free[3] = !free_out3.read();
        output_free[4] = !free_out4.read();

        sc_uint<7> reqs[5];
        reqs[0] = req0.read();
        reqs[1] = req1.read();
        reqs[2] = req2.read();
        reqs[3] = req3.read();
        reqs[4] = req4.read();

        for (int i = 0; i < 5; i++)
        {
            bool fifo_empty = (bool)reqs[i][4];
            bool tail_flit  = (bool)reqs[i][5];
            sc_uint<4> dest = reqs[i].range(3, 0);

            if (fifo_empty) {
                continue;
            }

            sc_uint<3> route_code;
            int out_index;

            // Wormhole behavior:
            // header decides and reserves the route
            // body flits reuse the reserved route
            if (!connected_input[i]) {
                route_code = compute_route(arbiter_id.read(), dest);
                out_index = (int)route_code - 1;

                if (out_index < 0 || out_index > 4) {
                    continue;
                }

                if ((!output_free[out_index]) || (output_owner[out_index] != -1)) {
                    continue;
                }

                connected_input[i] = true;
                reserved_route[i] = route_code;
                output_owner[out_index] = i;
            }
            else {
                route_code = reserved_route[i];
                out_index = (int)route_code - 1;

                if (out_index < 0 || out_index > 4) {
                    continue;
                }

                if (output_owner[out_index] != i) {
                    continue;
                }

                if (!output_free[out_index]) {
                    continue;
                }
            }

            if (i == 0) {
                grant0.write(1);
                v_select.range(2, 0) = route_code;
            }
            else if (i == 1) {
                grant1.write(1);
                v_select.range(5, 3) = route_code;
            }
            else if (i == 2) {
                grant2.write(1);
                v_select.range(8, 6) = route_code;
            }
            else if (i == 3) {
                grant3.write(1);
                v_select.range(11, 9) = route_code;
            }
            else if (i == 4) {
                grant4.write(1);
                v_select.range(14, 12) = route_code;
            }

            output_free[out_index] = false;

            if (tail_flit) {
                connected_input[i] = false;
                reserved_route[i] = 0;
                output_owner[out_index] = -1;
            }
        }

        aselect.write(v_select);
    }
}
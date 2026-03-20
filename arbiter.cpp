#undef SC_INCLUDE_FX

#include "packet.h"
#include "arbiter.h"

static sc_uint<3> compute_route(sc_uint<4> router_id, sc_uint<4> dest_id)
{
    sc_uint<2> rx = router_id.range(1, 0);
    sc_uint<2> ry = router_id.range(3, 2);

    sc_uint<2> dx = dest_id.range(1, 0);
    sc_uint<2> dy = dest_id.range(3, 2);

    // Route code mapping must match the crossbar:
    // 1 -> local (o0)
    // 2 -> north (o1)
    // 3 -> east  (o2)
    // 4 -> south (o3)
    // 5 -> west  (o4)

    if (rx < dx) return 3;   // east
    if (rx > dx) return 5;   // west
    if (ry < dy) return 4;   // south
    if (ry > dy) return 2;   // north
    return 1;                // local
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
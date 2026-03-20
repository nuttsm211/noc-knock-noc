#include "buf_fifo.h"

void fifo::packet_in(const packet& data_packet)
{
    if (full) {
        return;
    }

    registers[reg_num++] = data_packet;
    empty = false;

    if (reg_num >= 4) {
        reg_num = 4;
        full = true;
    }
}

packet fifo::packet_out()
{
    packet temp;

    if (empty) {
        temp.data = 0;
        temp.id = 0;
        temp.dest = 0;
        temp.pkt_clk = 0;
        temp.h_t = 0;
        temp.send_time = 0;
        return temp;
    }

    temp = registers[0];
    reg_num--;

    if (reg_num <= 0) {
        reg_num = 0;
        empty = true;
    } else {
        for (int i = 0; i < reg_num; i++) {
            registers[i] = registers[i + 1];
        }
    }

    full = false;
    return temp;
}

void buf_fifo::func()
{
    fifo q0;
    packet b_temp;

    packet init_pkt;
    init_pkt.data = 0;
    init_pkt.id = 0;
    init_pkt.dest = 0;
    init_pkt.pkt_clk = 0;
    init_pkt.h_t = 0;
    init_pkt.send_time = 0;

    re.write(init_pkt);
    ack.write(q0.full);

    sc_uint<7> init_req = 0;
    if (!q0.empty) {
        init_req[5] = q0.registers[0].h_t;
        init_req[4] = q0.empty;
        init_req.range(3, 0) = q0.registers[0].dest;
    } else {
        init_req[5] = 0;
        init_req[4] = 1;
        init_req.range(3, 0) = 0;
    }
    req.write(init_req);

    while (true)
    {
        wait();

        if (wr.event())
        {
            if (!q0.full) {
                q0.packet_in(wr.read());
            }

            ack.write(q0.full);

            sc_uint<7> req_val = 0;
            if (!q0.empty) {
                req_val[5] = q0.registers[0].h_t;
                req_val[4] = q0.empty;
                req_val.range(3, 0) = q0.registers[0].dest;
            } else {
                req_val[5] = 0;
                req_val[4] = 1;
                req_val.range(3, 0) = 0;
            }
            req.write(req_val);
        }

        if (bclk.event())
        {
            if ((grant.read() == 1) && (!q0.empty))
            {
                b_temp = q0.packet_out();
                re.write(b_temp);
            }

            ack.write(q0.full);

            sc_uint<7> req_val = 0;
            if (!q0.empty) {
                req_val[5] = q0.registers[0].h_t;
                req_val[4] = q0.empty;
                req_val.range(3, 0) = q0.registers[0].dest;
            } else {
                req_val[5] = 0;
                req_val[4] = 1;
                req_val.range(3, 0) = 0;
            }
            req.write(req_val);
        }
    }
}
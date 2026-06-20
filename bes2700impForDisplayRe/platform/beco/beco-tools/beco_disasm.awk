#! /usr/bin/awk -f
#
# Copyright 2020 BES Technic

# Description: Helper script to disassemble beco instructions.
#
# Usage:
#  arm-none-eabi-objdump -d beco-test.o -mcortex-m33 | ./beco_disasm.awk
#

/.*:[ \t]e[ce]/ {
    # CDP,MRC,MCR instructions
    b = gensub(/^[ \t]*([^:]*):[ \t]([0-9a-f]{4}) ?([0-9a-f]{4}).*$/, "\\1 \\2 \\3", "g", $0);
    split(b, m, " ");

    inst = strtonum("0x" m[2]m[3]);
    b31_24   = rshift(inst, 24);
    b4       = and( rshift(inst, 4),  1);
    b20      = and( rshift(inst, 20), 1);
    opc23_20 = and( rshift(inst, 20), 0xf);
    opc7_4   = and( rshift(inst, 4),  0xf);
    coproc   = and( rshift(inst, 8),  0xf);
    r19_16   = and( rshift(inst, 16), 0xf);
    r15_12   = and( rshift(inst, 12), 0xf);
    r3_9     = and( rshift(inst, 0),  0xf);
    crm = r3_9;
    crn = r19_16;
    crd = r15_12;
    rt  = crd;
    rt2 = r19_16;
    crimm = or(or(crm, lshift(crd, 4)), lshift(crn, 8));
    s_inst = "unkn";
 #   printf("opc1:%1x opc2:%1x crm:%1x crd:%1x crn:%1x\t", opc23_20, opc7_4,crm, crd, crn);
    if (b31_24 == 0xee && coproc == 0 && b20 == 1 && b4 == 1) {  # MRC
        #    MRC    'CP2'    {opc1, 1'bx} {opc2, 1'bx}
        #   ---------------------------------------+--------------------------------------
        #   | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
        #   |  1  1  1  *| 1  1 1 0|opc1 |1|  CRn  |     Rt    | coproc  |opc2 |1|  CRm  |
        #   ---------------------------------------+--------------------------------------
        opc23_20 = rshift(opc23_20, 1);
        opc7_4   = rshift(opc7_4, 1);
        if (opc23_20 == 0 && opc7_4 == 0) {        #  RDCPID
            s_inst = sprintf("r%d = beco_cpid()", rt);
        }
        else if (opc23_20 == 0 && opc7_4 == 1) {   # RDSTAT
            s_inst = sprintf("r%d = beco_status()", rt);
        }
        else if (opc23_20 == 0 && opc7_4 == 2) {   # RDACC
            s_inst = sprintf("r%d = beco_read_acc(acc%d, lane:%d)", rt, crm, crn);
        }
        else if (opc23_20 == 0 && opc7_4 == 4) {   # RDNXACC
            s_inst = sprintf("r%d = beco_read_next_acc(acc%d, lane:%d)", rt, crm, crn);
        }
        else if (opc23_20 == 0 && opc7_4 == 3) {   # RDCONF
            s_inst = sprintf("r%d = beco_config()", rt);
        }
    }
    else if (b31_24 == 0xee && coproc == 0 && b20 == 0 && b4 == 1) {  # MCR
        #    MCR    'CP2'    {opc1, 1'bx} {opc2, 1'bx}
        #   ---------------------------------------+--------------------------------------
        #   | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
        #   |  1  1  1  *| 1  1 1 0|opc1 |0|  CRn  |     Rt    | coproc  |opc2 |1|  CRm  |
        #   ---------------------------------------+--------------------------------------
        opc23_20 = rshift(opc23_20, 1);
        opc7_4   = rshift(opc7_4, 1);
        if (opc23_20 == 0 && opc7_4 == 0) {        #  WCONF
            s_inst = sprintf("beco_write_config(r%d)", crd);
        }
        else if (opc23_20 == 0 && opc7_4 == 1) {   # MMACGR
            s_inst = sprintf("acc%d = beco_mmacgr(r%d, cr%d)", crm, rt, crn);
        }
        else if (opc23_20 == 0 && opc7_4 == 2) {   # MMACGR4
            s_inst = sprintf("acc%d+ = beco_mmacgr4(r%d, cr%d+)", crm, rt, crn);
        }
    }
    else if (b31_24 == 0xee && coproc == 0 && b4 == 0) {  # CDP

        #    CDP         'CP2'    opc1         {opc2, 1'bx}
        #   ---------------------------------------+--------------------------------------
        #   | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
        #   |  1  1  1  *| 1  1 1 0| opc1  |  CRn  |    CRd    | coproc  |opc2 |0| CRm   |
        #   ---------------------------------------+--------------------------------------
        opc7_4   = rshift(opc7_4, 1);
        if (opc23_20 == 0 && opc7_4 == 0) {        #  WROCONF
            s_inst = sprintf("beco_set_outconfig(0x%03x)", crimm);
        }
        else if (opc23_20 == 0 && opc7_4 == 1) {   # WRACONF
            s_inst = sprintf("beco_set_aluconfig(0x%03x)", crimm);
        }
        else if (opc23_20 == 0 && opc7_4 == 2) {   # CLRACC
            s_inst = sprintf("beco_clear_acc(acc%d)\t; acc%d = 0;", crd, crd);
        }
        else if (opc23_20 == 2 && opc7_4 == 3) {   # PSETACC
            s_inst = sprintf("beco_preload_acc(acc%d, {cr%d, cr%d})\t", crd, crm,crn);
        }
        else if (opc23_20 == 2 && opc7_4 == 2) {   # SETBIAS
            s_inst = sprintf("beco_set_acc_bias(acc%d:%d, {cr%d, cr%d})\t;", crd%4,crd/4, crm,crn);
        }
        else if (opc23_20 == 0 && opc7_4 == 3) {   # MATOR32
            s_inst = sprintf("cr%d = beco_mator32(acc%d, lane:%d)", crd, crm, crn);
        }
        else if (opc23_20 == 2 && opc7_4 == 1) {   # MATOR64
            s_inst = sprintf("cr%d = beco_mator64(acc%d, lane:%d)", crd, crm, crn);
        }
        else if (opc23_20 == 0 && opc7_4 == 7) {   # MRTOT32
            s_inst = sprintf("cr%d = beco_mrtor32(cr%d)", crd, crm);
        }
        else if (opc23_20 == 0 && opc7_4 == 6) {   # MMACR
            s_inst = sprintf("acc%d = beco_mmacr(cr%d, cr%d)", crd, crm, crn);
        }
        else if (opc23_20 == 2 && opc7_4 == 0) {   # MMACRR
            s_inst = sprintf("acc%d+ = beco_mmacrr(cr%d, cr%d)", crd, crm, crn);
        }
        else if (opc23_20 == 1) {                  # BSHIFT
            s_inst = sprintf("cr%d = beco_bshift({cr%d, cr%d} >> %d*8)", crd, crn, crm, opc7_4);
        }
        else if (opc23_20 == 3) {                  # BSHIFT5
            s_inst = sprintf("cr%d+ = beco_bshift5({cr%d+, cr%d+} >> %d*8)", crd, crn, crm, opc7_4);
        }
    }
    else if (b31_24 == 0xec && coproc == 0 && b20 == 0) {  # MCRR
        #    MCRR  'CP2'    4'bx         opc1
        #   ---------------------------------------+--------------------------------------
        #   | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
        #   |  1  1  1  *| 1  1 0 0 0 1 0 0|  Rt2  |     Rt    | coproc  | opc1  | CRm   |
        #   ---------------------------------------+--------------------------------------
        if (opc7_4 == 0) {                        # MMAC
            s_inst = sprintf("acc%d = beco_mmac(r%d, r%d)", crm, rt2, rt);
        }
        else if (opc7_4 == 1) {                   # WRREG
            s_inst = sprintf("cr%d = beco_write_reg({r%d, r%d})", crm, rt2, rt);
        }
    }
    else if (b31_24 == 0xec && coproc == 0 && b20 == 1) {  # MRRC
        #    MRRC  'CP2'    4'bx         opc1
        #   ---------------------------------------+--------------------------------------
        #   | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
        #   |  1  1  1  *| 1  1 0 0 0 1 0 1|  Rt2  |     Rt    | coproc  |  opc1 |  CRm  |
        #   ---------------------------------------+--------------------------------------
        if (opc7_4 == 0) {                        # RDREG
            s_inst = sprintf("{r%d, r%d} = beco_read_reg(cr%d)", rt2, rt, crm);
        }
    }
    $0 = sprintf("%4s:\t%s %s\t%s", m[1], m[2], m[3], s_inst);
#    print(s_inst);
}



{
    print($0);
}

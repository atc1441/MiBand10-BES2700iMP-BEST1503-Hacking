
#ifndef parking_h
#define parking_h

#define BASE_DIM_X  1280
#define BASE_DIM_Y  720

#define REF_DIM_X   1920
#define REF_DIM_Y   1080

#define   OPCODE_END                                               0x00
#define   OPCODE_CLOSE                                             0x01
#define   OPCODE_MOVE                                              0x02
#define   OPCODE_MOVE_REL                                          0x03
#define   OPCODE_LINE                                              0x04
#define   OPCODE_LINE_REL                                          0x05
#define   OPCODE_QUADRATIC                                         0x06
#define   OPCODE_QUADRATIC_REL                                     0x07
#define   OPCODE_CUBIC                                             0x08
#define   OPCODE_CUBIC_REL                                         0x09
#define   OPCODE_BREAK                                             0x0A
#define   OPCODE_HLINE                                             0x0B
#define   OPCODE_HLINE_REL                                         0x0C
#define   OPCODE_VLINE                                             0x0D
#define   OPCODE_VLINE_REL                                         0x0E
#define   OPCODE_SQUAD                                             0x0F
#define   OPCODE_SQUAD_REL                                         0x10
#define   OPCODE_SCUBIC                                            0x11
#define   OPCODE_SCUBIC_REL                                        0x12

#endif /* parking_h */

## address assignment
lui x2, 0x80000  # GPU address
lui x3, 0xC0000  # Key address
lui x4, 0x00001  # RAM address

## global variable
addi x6, x0, 0   # pixel pos in current frame
addi x7, x0, 1   # refresh frame command

sb   x7, 0x100(x2)  # set pixel color
sb   x7, 0x102(x2)  # refresh frame
sb   x0, 0x101(x2)   # set cursor pos = 0

add  x10, x0, x2

LOOP_FOREVER:
lbu  x14,  0(x10)
add  x13, x13, x14
j    LOOP_FOREVER

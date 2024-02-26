## address assignment
lui x2, 0x80000  # GPU address
lui x3, 0xC0000  # Key address
lui x4, 0x00001  # RAM address

## global variable
addi x5, x0, 4   # frame size
addi x6, x0, 0   # pixel pos in current frame
addi x7, x0, 1   # refresh frame command


# init neighbours relative pos (offset address = 0x00, size = 8)
addi x8, x0,  -1   # left
sb   x8, 0(x4)  
addi x8, x0,   1   # right
sb   x8, 1(x4)
addi x8, x0, -17   # top-left
sb   x8, 2(x4)
addi x8, x0, -16   # top
sb   x8, 3(x4)
addi x8, x0, -15   # top-right
sb   x8, 4(x4)
addi x8, x0,  15   # buttom-left
sb   x8, 5(x4)
addi x8, x0,  16   # buttom
sb   x8, 6(x4)
addi x8, x0,  17   # buttom-right
sb   x8, 7(x4)


# init LUT = 0
addi x8, x4, 32

LOOP_CONST:
addi x8, x8, -1
sb   x0, 0x10(x8)
beq  x8, x4, LUT
j    LOOP_CONST


## init state transition LUT (offset address = 0x10, size = 16)
LUT:
addi x9, x4, 0x10
addi x8, x0, 1
sb   x8, 3(x9)   # Any dead cell with three live neighbours becomes a live cell.
sb   x8, 10(x9)  # Any live cell with two   live neighbours survives
sb   x8, 11(x9)  # Any live cell with three live neighbours survives


## init LR edge correciton LUT (offset address = 0x20, size = 16)
addi x9, x4, 0x20
# right edge
addi x8, x0, -16  
sb   x8,  1(x9)  # right
sb   x8,  4(x9)  # top-right
sb   x8,  7(x9)  # buttom-right
# left edge
addi x8, x0, 16
sb   x8,  8(x9)  # left
sb   x8, 10(x9)  # top-left
sb   x8, 13(x9)  # buttom-left


## init world
lui  x8, 0x10000  # RAM address
sb   x0, 0x101(x2)   # set cursor pos = 0

LOOP_INIT:
add  x9, x8, x6
lbu  x15, 0(x9)      # rom value
#lbu  x15, 3(x3)      # random value
sb   x15, 0x100(x2)  # set pixel color
addi x6, x6, 1
beq  x6, x5, GNERATE
j    LOOP_INIT


##################################################
## core
GNERATE:
jal  REFRESH_FRAME
sb   x0, 0x101(x2)   # set cursor pos = 0

# x14: neighbours value
# x13: number of live neighbours
# x12: cell state
# x11: temp var
# x10: neighbours address
# x9 : neighbours index
# x8 : temp var


LOOP_FOREVER:
# do {
LOOP_FRAME:

    # for (x9 = x4 + 7, x13 = 0; x9 >= x4; --x9)
    addi x9, x4, 0x7  # neighbours offset address
    addi x13, x0, 0
LOOP_CELL:
    blt  x9, x4, LOOP_BREAK_CELL
    # {
        lbu  x8, 0(x9)       # get neighbour's offset
        add  x10, x6, x8     # get neighbour's frame pos
        # if current cell is at left/right edge
        addi x8, x6, 1
        andi x8, x8, 0x0F
        addi x11, x0, 2
        bltu x8, x11 IF_LR_EDGE
END_IF_LR_EDGE:
        # x10 = x10 % 256 + x2
        andi x10, x10, 0xFF
        add  x10, x10, x2
        # count live cells
        lbu  x14,  0(x10)
        add  x13, x13, x14
    # }
    addi x9, x9, -1
    j    LOOP_CELL
LOOP_BREAK_CELL:
    # end for

    # x12 = cell current state = Mem[x2 + x6]
    add  x10, x2, x6
    lbu  x12, 0(x10)
    # x12 = cell next state = Mem[x12 << 3 + x13 + (x4 + 0x10)]
    slli x12, x12, 3
    add  x12, x12, x13
    add  x12, x12, x4
    addi x12, x12, 0x10
    lbu  x12, 0(x12)
    # update cell state
    sb   x12, 0x100(x2) 
    
# } while (++x6 < x5)
addi x6, x6, 1
blt  x6, x5, LOOP_FRAME
# end while

jal  REFRESH_FRAME
j    LOOP_FOREVER


### Branches ###
IF_LR_EDGE:    
    # x8 = Mem[((x6 + 1) % 16 << 3 + x9) % 16 + (x4 + 0x20)]
    slli x8, x8, 3
    add  x8, x8, x9
    andi x8, x8, 0x0F
    add  x8, x8, x4
    addi x8, x8, 0x20
    lb   x8, 0(x8)
    # pos correciton: x10 = x10 + x8
    add  x10, x10, x8
    j    END_IF_LR_EDGE


### Funtions ###
REFRESH_FRAME:
    addi x6, x0, 0
    sb   x7, 0x102(x2)  # refresh frame
    ret

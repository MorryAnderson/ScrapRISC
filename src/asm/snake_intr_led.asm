######################################
#  ===================================
#  direction vector table
#  -----------------------------------
#  EF F0 F1
#  FF 00 01
#  0F 10 11
#  ===================================
#
#
#  ===================================
#  gpio register table
#  -----------------------------------
#  0x0: R : left/right
#  0x1: R : top/buttom
#  0x2: R : control = {ESC, OK, Y, X}
#  0x3: R : random
#  0x4: W : led
#  ===================================
#  
#  
#  ===================================
#  control mask table
#  -----------------------------------
#  ESC: 0xC0
#  OK : 0x30
#  Y  : 0x0C
#  X  : 0x03
#  ===================================
#  
#  
#  ===================================
#  game state table
#  -----------------------------------
#  [0] : 
#  [1] : initializing
#  [2] : paused
#  [3] : running
#  [4] : reading key input
#  [5] : rendering
#  [6] : vsync
#  [7] : dead
#  -----------------------------------
#  0x02 : initializing
#  0x04 : paused
#  0x08 : running
#  0x18 : reading key input
#  0x28 : rendering
#  0x48 : vsync
#  0x84 : dead
#  ===================================
#
######################################


## address assignment
lui x2, 0x80000  # GPU address
lui x3, 0x00001  # RAM address
lui x4, 0xC0000  # GPIO address

## global variable
addi  x5, x0, 256 # frame size
addi  x6, x0, 0   # pixel pointer
addi  x7, x0, 1   # constant 1 / command refresh
addi x15, x0, 0   # key input


INIT:
sb    x0, 0x102(x2) # clear frame
addi x14, x0, 0x02
sb   x14, 4(x4)     # led state = 0x02 = initializing

## state variable
addi  x8, x0, 0x84     # head pos
addi  x9, x0, 0x82     # tail pos
addi x10, x0, 0x88     # food pos
addi x11, x0, 0x01     # head dir
lui  x12, 0x10         # input mask (0xFF00 = 0x10000 - 0x100)
addi x12, x12, -0x100  # input mask (0xFF00)

## temp variable
addi  x13, x0, 1  # dir (1, -1, 16, -16)
addi  x14, x0, 1  # pixel value

## init list
mv    x6, x3
sb   x13, 0x82(x6)
sb   x13, 0x83(x6)

## init snake & food
mv    x6, x9         # cursor pos of tail
sb    x6, 0x101(x2)  # set cursor pos
sb   x14, 0x100(x2)  # draw tail
sb   x14, 0x100(x2)  # draw body
sb   x14, 0x100(x2)  # draw head
mv    x6, x10        # cursor pos of food
sb    x6, 0x101(x2)  # set cursor pos
sb   x14, 0x100(x2)  # draw food

## draw first frame
sb    x7, 0x102(x2)  # refresh frame

PAUSED:
addi x14, x0, 0x04 
sb   x14, 4(x4)      # led state = 0x04 = paused


LOOP_OK:
lb   x15, 2(x4)      # get key input (control)
andi x15, x15, 0x30  # "OK" is pressed
beqz x15, LOOP_OK


RUNNING:
addi x14, x0, 0x08 
sb   x14, 4(x4)     # led state = 0x08 = running


LOOP_FRAME:

KEY:
addi x14, x0, 0x18 
sb   x14, 4(x4)     # led state = 0x18 = reading key input

lb   x15, 2(x4)      # get key input (control)
andi x13, x15, 0xC0  # "ESC" is pressed
bnez x13, INIT       # reset game
andi x13, x15, 0x03  # "X" is pressed
bnez x13, PAUSED     # pause game

lh   x15, 0(x4)     # get key input (direction)
and  x15, x15, x12  # direction mask


RENDERING:
addi x14, x0, 0x28 
sb   x14, 4(x4)     # led state = 0x28 = rendering

# convert key (x15) to direction (x13)
srli x13, x15, 4
or   x13, x13, x15
andi x13, x13, 0xFF


#-- for now, x14 and x15 are the availabe temp vars
#-- x11 is the current direction, x13 is the input direction

# whether direction is unchanged
seqz x14, x13       # changed (x14 =  0) | unchanged (x14 = 1)
addi x14, x14, -1   # changed (x14 = -1) | unchanged (x14 = 0)
xor  x12, x12, x14  # if changed, invert direction mask
not  x14, x14       # changed (x14 =  0) | unchanged (x14 = -1)

# calc next direction and posistion
and  x11, x11, x14  # if changed, next direction = input direction
add  x11, x11, x13  #  otherwise, next direction = current direction
add  x15, x8,  x11  # next head pos (x15)
andi x15, x15, 0xFF

#-- for now, x13 and x14 are the availabe temp vars
#-- x8/x15 is the current/next pos of head
#-- x11 is the current direction

# [hit detection]
add  x14, x2, x15   # check next pos
lb   x13, 0(x14)    # next pos value
bnez x13, HIT_SOMETHING  # next pos is not empty


# [hit nothing]
sb    x9, 0x101(x2)  # set cursor pos
sb    x0, 0x100(x2)  # cut tail
add  x13, x3, x9
lb   x14, 0(x13)     # tail direction
add   x9, x9, x14    # next tail pos
andi  x9, x9, 0xFF

# [step forward]
STEP:
add  x13, x3, x8
sb   x11, 0(x13)     # set dir of cur head
mv    x8, x15        # set new head pos
add  x13, x2, x15
sb   x15, 0x101(x2)  # set cursor pos
sb    x7, 0x100(x2)  # draw new head

# [vsync]
addi x14, x0, 0x48 
sb   x14, 4(x4)     # led state = 0x48 = vsync

LOOP_VSYNC:
lb   x13, 0x103(x2)
beqz x13, LOOP_VSYNC

sb    x7, 0x102(x2)  # refresh frame

j    LOOP_FRAME


## Branches
DEAD:
addi x14, x0, 0x84 
sb   x14, 4(x4)     # led state = 0x84 = DEAD
LOOP_DEAD:
lb   x15, 2(x4)      # get key input (control)
andi x15, x15, 0xC0  # "ESC" is pressed
beqz x15, LOOP_DEAD
j    INIT


HIT_SOMETHING:
beq x15, x10, HIT_FOOD
j    DEAD  # if not hit food, die


HIT_FOOD:
lb   x10, 3(x4)      # new food pos
MOVE_FOOD:
addi x10, x10, 0x11  # move food buttom right
andi x10, x10, 0xFF  
add  x14, x2, x10    # check food pos
lb   x14, 0(x14)     # get value of next food pos
bnez x14, MOVE_FOOD  # if next food pos is not empty, move food again
sb   x10, 0x101(x2)  # set cursor pos
sb    x7, 0x100(x2)  # draw food
j    STEP

nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop


lui  t0, 0x01010
addi t0, t0, 0x101

LOOP:
sb   t0, 4(x4)
srli t1, t0, 31
slli t0, t0, 1
or   t0, t0, t1
j LOOP

lui x1, 0x10000  # data ROM address
lui x2, 0x80000  # GPU address

addi x3, x0, 0  
sb   x3, 0x101(x2)   # cursor pos = 0
addi x4, x0, 256 # frame size
addi x5, x1, 0   # pixel address
addi x6, x0, 0   # pixel pos in current frame
addi x7, x0, 1   # refresh frame

LOOP:
lb   x15, 0(x5)  # read pixel color
sb   x15, 0x100(x2)  # set pixel color
addi x5, x5, 1
addi x6, x6, 1

beq x6, x4, IF1
ELSE1:
# nothing here
ENDIF1:

j LOOP


# BRANCH
IF1:  # if (x6 == x4)
addi x6, x0, 0
sb   x7, 0x102(x2)  # refresh frame
j ENDIF1

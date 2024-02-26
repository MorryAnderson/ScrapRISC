j       A
addi    a0, x0, 1        # print_int ecall
addi    a1, x0, 42       # integer 42
ecall

A:
addi    x1, x0, 1
xori    x2, x1, 2
ori     x3, x2, 4
andi    x4, x3, 5
slli    x5, x4, 1
srli    x6, x5, 1
srai    x7, x6, 1
slti    x8, x7, -1
sltiu   x9, x8, -1

B:
add     x1, x1, x1
xor     x2, x2, x2
or      x3, x3, x2
and     x4, x4, x3
sll     x5, x5, x1
srl     x6, x6, x1
sra     x7, x7, x1
slt     x8, x7, x0
sltu    x9, x8, x0

C:
lui     x13, 0x10011
auipc   x14, 0x00110

sb      x1, 0(x13)
sh      x2, 2(x13)
sw      x3, 4(x13)

lb      x10, 0(x13)
lh      x11, 2(x13)
lw      x12, 4(x13)


D:
beq     x1, x0, A
bne     x1, x1, B
blt     x1, x1, C
bltu    x1, x1, C
bge     x0, x1, D
bgeu    x0, x1, D
jal     x15, E

E:
jalr    x15, x0, 4

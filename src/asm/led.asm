lui  gp, 0xC0000  # GPIO address

lui  t0, 0x01010
addi t0, t0, 0x101

LOOP:
sb   t0, 4(gp)
srli t1, t0, 31
slli t0, t0, 1
or   t0, t0, t1
j LOOP
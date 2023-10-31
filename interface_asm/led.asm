.globl main
array: .word 0x80100000, 0x80100008, 0xffffff

main:
la a0, array                  # a0 -> array
lw t1, 4(a0)               # t1 = GPIO_SWs
lw t2, 0(a0)               # t2 = GPIO_LEDs
addi t6,zero,1             # t6 = 1
begin:
lw t3, 0(t1)                            # t3 = SWs
sw t3, 0(t2)                            # leds = t3

lw t4, 8(a0)               # t4 = time interval
loop1:
sub t4,t4,t6
bne t4,zero,loop1

sw zero, 0(t2)

lw t4, 8(a0)               # t4 = time interval
loop2:
sub t4,t4,t6
bne t4,zero,loop2

j begin
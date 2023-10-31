.globl main
array: .word 10, 9, 8, 7, 6, 5, 4, 3, 2, 1

main:
  la a0, array                  # a0 -> array
  addi a1, zero, 10             # a1 -> 10
  
bubblesort:
  addi t0,zero,0                # t0 -> i
  outerloop:
    bge t0, a1, endloop         # 如果 i >= n 则退出循环
    addi t1,zero,0              # t1 -> j
    innerloop:
      sub t3, a1, t0            #
      addi t3, t3, -1           # t3 -> 9-i
      bge t1, t3, nextouter     # if(j >= n-i-1) branch
      slli t2, t1, 2            # t2 -> 4*j
      add t2, t2, a0            # t2 -> array + j
      lw t4, 0(t2)              # t4 -> array[j]
      lw t5, 4(t2)              # t5 -> array[j+1]
      blt t4, t5, nextinner     # if(t4 < t5) branch
      sw t5, 0(t2)              # array[j+1] = array[j]
      sw t4, 4(t2)              # array[j] = array[j+1]
      nextinner:
      addi t1, t1, 1             # j++
      j innerloop
  nextouter:
    addi t0, t0, 1              # i++
    j outerloop
endloop:
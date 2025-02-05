/* Program to Count the number of 1's in a 32-bit word,
located at InputWord */

.global _start
_start:
	
	la t0, InputWord# pointer to inputword
	la t1, Answer	# pointer to answer
	lw t2, (t0)
	li t3, 1
	li t5, 1		# counter for the loop
	li s1, 32
	li s0, 0		# for storing the current number of ones
	
	andi t4, t2, 1	# t4 stores the LSD of t2
	beq t4, t3, isOne
	
	loop:
	srli t2, t2, 1	# t6 stores the shifted right one bit version of inputword
	addi t5, t5, 1
	bgt t5, s1, finished	# counter > 32 the word has been checked fully
	andi t4, t2, 0x1	#t4 stores the LSD of t6
	beq t4, t3, isOne
	j loop
	
	isOne:
	addi s0, s0, 1
	j loop
	
	finished:
	sw s0, (t1)
	
    stop: j stop

.data
InputWord: .word 0x4a01fead

Answer: .word 0

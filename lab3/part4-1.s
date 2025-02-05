/* Program to Count the number of 1's and Zeroes in a sequence of 32-bit words,
and determines the largest of each */

/*The program runs faster on the actual board than CPULator becuase the board
was designed to optimize the execution of instructions on the Nios V board, 
while the simulation uses your computer's hardware which isn't optimized for
this so it runs slower. */

.global _start
_start:

/* Your code here  */
	
	li t4, 0      #Store the largest amount of 1's into t4
	li t5, 0      #Store the largest amount of 0's into t5
	la t6, TEST_NUM   #Store the address of the first num in TEST_NUM in t6
	li t0, 0xFFFFFFFF   #Loads all 1s into register t0

loopMain: 
	lw a0, (t6)   #Store the current word at the memory location pointed to by t6
	beq a0, x0, stop
	j findOnes

findOnes:
	call ONES
	bge t4, a0, findZeros   #If t4 is greater than a0, go branch to findZeros
	mv t4, a0  #Moves the values in a0 into t4
	j findZeros
	
findZeros:
	lw a0, (t6)    #Reload the number into a0
	xor a0, a0, t0    #Use xor to flip the bits of a0
	call ONES
	addi t6, t6, 4   #Moves to the next address location in t6
	bge t5, a0, loopMain   #If t5 is greater than a0, go branch to loopMain
	mv t5, a0  #Moves the values in a0 into t5
	j loopMain


stop:
	mv t0, t4  #Moves the value of t4 into t0
	la t2, LargestOnes
	sw t4, (t2)
	la t2, LargestZeroes
	sw t5, (t2)
	.equ LEDs, 0xFF200000
	la t1, LEDs      #Address of LEDs is put into t1
	sw t0, (t1)      #Stores the value of t0 into the address at t1
	li a0, 20000000	#Loads 100 into register a0
	call delayLoop
	mv t0, t5       #Copies all of t5 into t0
	sw t0, (t1)      #Stores the value of t0 into the address at t1
	li a0, 20000000	#Loads 100 into register a0
	call delayLoop
	j stop


ONES:

	li t1, 1  #Load a 1 into register t1
	li t2, 0  #Register to store the number of 1's
	
loop: 
	and t3, a0, t1   #Store bitwise and of a0 and t1 into t3
	srli a0, a0, 1   #Shift the value in register a0 right by 1
	
	beq t3, x0, loop    #If the bitwise and resulted in 0, go back to loop
	
	addi t2, t2, 1   #Otherwise it means the bit had a 1, so we increment t2
	ble a0, x0, finish   #If the value in a0 reaches 0, go to end
	
	j loop
finish:
	mv a0, t2   #Moves the value stored in t2 into a0
	ret	 	    #Returns to where subroutine was called
	
delayLoop:
	addi a0, a0, -1  #Subtracts 1 from a0
	bne a0, x0, delayLoop  #If a0 is not equal to 0, loop
	ret   #Returns to where subroutine was called
.data
TEST_NUM:  .word 0x4a01fead, 0xF677D671,0xDC9758D5,0xEBBD45D2,0x8059519D
            .word 0x76D8F0D2, 0xB98C9BB5, 0xD7EC3A9E, 0xD9BADC01, 0x89B377CD
            .word 0xfff0fff0, 0x80000001, 0  # end of list 

LargestOnes: .word 0
LargestZeroes: .word 0
.global _start
_start:


.equ HEX_BASE1, 0xff200020
.equ HEX_BASE2, 0xff200030

.equ PUSH_BUTTON, 0xFF200050

la sp, 0x20000		# Initialize the stack pointer
csrw mstatus, zero 	# Turn off interrupts in case an interrupt is called 
					# before correct set up
					
la t1, PUSH_BUTTON	# set up key interrupt mask register
li t0, 0b1111
sw t0, 8(t1)
sw t0, 12(t1)

li t0, 0x40000
csrs mie, t0		# turn bit 18 of mie on, enabling IRQ18

la t0, interrupt_handler
csrw mtvec, t0		# set mtvec pointing to where the handler is

li t0, 0b1000
csrs mstatus, t0	# enable processor interrupts 

IDLE: j IDLE #Infinite loop while waiting on interrupt

interrupt_handler:
	addi sp, sp, -12
	
	sw s0, 0(sp)
	sw s1, 4(sp)
	sw ra, 8(sp)
	
	li s0, 0x40000 #Checking if Interrupt comes from IRQ 18
	csrr s1, mcause
	
	and s1, s1, s0
	bnez s1, end_interrupt
	
	jal KEY_ISR # If so call KEY_ISR
	
	end_interrupt:
	lw s0, 0(sp)
	lw s1, 4(sp)
	lw ra, 8(sp)
	addi sp, sp, 12
	
mret

KEY_ISR: 
	addi sp, sp, -12
	sw ra, (sp)
	sw s0, 4(sp)
	sw s1, 8(sp)
	
	la s0, PUSH_BUTTON
	lw s1, 12(s0)
	li s0, 1
	beq s1, s0, KEY0_Pushed
	li s0, 2
	beq s1, s0, KEY1_Pushed
	li s0, 4
	beq s1, s0, KEY2_Pushed
	li s0, 8
	beq s1, s0, KEY3_Pushed
	
KEY0_Pushed:
	la s0, PUSH_BUTTON
	sw s1, 12(s0)		# reset edge capture
	la s0, HEX_BASE1
	lw s1, (s0)
	andi s1, s1, 0b111111	# take the HEX0 status
	bnez s1, resetHEX0
	li a0, 0			# if the HEX is empty, load value zero to HEX0
	li a1, 0
	call HEX_DISP
	
	lw ra, (sp)
	lw s0, 4(sp)
	lw s1, 8(sp)
	addi sp, sp, 12
	ret
	resetHEX0:		   # if the HEX has value, reset to blank
	li a0, 0b10000
	li a1, 0
	call HEX_DISP
	
	lw ra, (sp)
	lw s0, 4(sp)
	lw s1, 8(sp)
	addi sp, sp, 12
	ret
KEY1_Pushed:
	la s0, PUSH_BUTTON
	sw s1, 12(s0)		# reset edge capture
	la s0, HEX_BASE1
	lw s1, (s0)
	srli s1, s1, 7
	andi s1, s1, 0b1111111	# take the HEX1 status
	bnez s1, resetHEX1
	li a0, 1			# if the HEX is empty, load value one to HEX1
	li a1, 1
	call HEX_DISP
	
	lw ra, (sp)
	lw s0, 4(sp)
	lw s1, 8(sp)
	addi sp, sp, 12
	ret
	resetHEX1:		   # if the HEX has value, reset to blank
	li a0, 0b10000
	li a1, 1
	call HEX_DISP
	
	lw ra, (sp)
	lw s0, 4(sp)
	lw s1, 8(sp)
	addi sp, sp, 12
	ret
KEY2_Pushed:
	la s0, PUSH_BUTTON
	sw s1, 12(s0)		# reset edge capture
	la s0, HEX_BASE1
	lw s1, (s0)
	srli s1, s1, 14
	andi s1, s1, 0b1111111	# take the HEX2 status
	bnez s1, resetHEX2
	li a0, 2			# if the HEX is empty, load value two to HEX2
	li a1, 2
	call HEX_DISP
	
	lw ra, (sp)
	lw s0, 4(sp)
	lw s1, 8(sp)
	addi sp, sp, 12
	ret
	resetHEX2:		   # if the HEX has value, reset to blank
	li a0, 0b10000
	li a1, 2
	call HEX_DISP
	
	lw ra, (sp)
	lw s0, 4(sp)
	lw s1, 8(sp)
	addi sp, sp, 12
	ret
KEY3_Pushed:
	la s0, PUSH_BUTTON
	sw s1, 12(s0)		# reset edge capture
	la s0, HEX_BASE1
	lw s1, (s0)
	srli s1, s1, 21
	andi s1, s1, 0b1111111	# take the HEX3 status
	bnez s1, resetHEX3
	li a0, 3			# if the HEX is empty, load value three to HEX3
	li a1, 3
	call HEX_DISP
	
	lw ra, (sp)
	lw s0, 4(sp)
	lw s1, 8(sp)
	addi sp, sp, 12
	ret
	resetHEX3:		   # if the HEX has value, reset to blank
	li a0, 0b10000
	li a1, 3
	call HEX_DISP
	
	lw ra, (sp)
	lw s0, 4(sp)
	lw s1, 8(sp)
	addi sp, sp, 12
	ret

ret

# subroutine for hexdisplay
# a0 is what to display and a1 is where to display
# bit 4 of a0 set to one can blank the display indicated by a1
# value of a0 and a1 are not preserved as origin after return from this subroutine
HEX_DISP:   
	addi sp, sp, -24
	sw s0,0(sp)
	sw s1,0x4(sp)
	sw s2,0x8(sp)
	sw s3,0xC(sp)
	sw a0,16(sp)
	sw a1,20(sp)
	
	la   s0, BIT_CODES         # starting address of the bit codes
    andi     s1, a0, 0x10	       # get bit 4 of the input into r6
	beq      s1, zero, not_blank 
    mv      s2, zero
	j       DO_DISP
not_blank:
	andi     a0, a0, 0x0f	   # r4 is only 4-bit
    add      a0, a0, s0        # add the offset to the bit codes
    lb      s2, 0(a0)         # index into the bit codes

#Display it on the target HEX display
DO_DISP:    
	la       s0, HEX_BASE1         # load address
	li       s1,  4
	blt      a1,s1, FIRST_SET      # hex4 and hex 5 are on 0xff200030
	sub      a1, a1, s1            # if hex4 or hex5, we need to adjust the shift
	addi     s0, s0, 0x0010        # we also need to adjust the address
FIRST_SET:
	slli     a1, a1, 3             # hex*8 shift is needed
	addi     s3, zero, 0xff        # create bit mask so other values are not corrupted
	sll      s3, s3, a1 
	li     	 a0, -1
	xor      s3, s3, a0  
    sll      a0, s2, a1            # shift the hex code we want to write
	lw    	 a1, 0(s0)             # read current value       
	and      a1, a1, s3            # and it with the mask to clear the target hex
	or       a1, a1, a0	           # or with the hex code
	sw    	 a1, 0(s0)		       # store back
END:			
	mv 		 a0, s2				   # put bit pattern on return register
	lw s0,0(sp)
	lw s1,0x4(sp)
	lw s2,0x8(sp)
	lw s3,0xC(sp)
	lw a0,16(sp)
	lw a1,20(sp)
	addi sp, sp, 24
	ret


.data
BIT_CODES:  .byte     0b00111111, 0b00000110, 0b01011011, 0b01001111
			.byte     0b01100110, 0b01101101, 0b01111101, 0b00000111
			.byte     0b01111111, 0b01100111, 0b01110111, 0b01111100
			.byte     0b00111001, 0b01011110, 0b01111001, 0b01110001

            .end
			

.global _start
_start:

	.equ LEDs,  0xFF200000
	.equ TIMER, 0xFF202000
	.equ KEY_BASE, 0xFF200050

	la sp, 0x20000			# set up the stack pointer
	csrw mstatus, zero		# disable all interrupt to processor
	
	jal    CONFIG_TIMER     # configure the Timer
    jal    CONFIG_KEYS      # configure the KEYs port
	
	/*Enable Interrupts in the NIOS V processor, and set up the address handling
	location to be the interrupt_handler subroutine*/
	li t0, 0x50000
	csrs mie, t0			# enable IRQ18 and IRQ16 for keys and timer 
							# request to pass
							
	la t0, interrupt_handler
	csrw mtvec, t0			# indicate handler address
	
	li t0, 0b1000
	csrs mstatus, t0		# enable processor interrupt
	
	la s0, LEDs
	la s1, COUNT
	LOOP:
		lw s2, 0(s1)		# Get current count
		sw s2, 0(s0)		# Store count in LEDs
	j LOOP

interrupt_handler:
	addi sp, sp, -12
	sw t0, (sp)
	sw t1, 4(sp)
	sw ra, 8(sp)
	
	li t0, 0x7FFFFFFF
	csrr t1, mcause
	and t1, t1, t0			# check which IRQ is requesting interrupt
	li t0, 16
	beq t1, t0, timerInterrupt
	call KEY_ISR
	j endInterrupt
	timerInterrupt:
	call TIMER_ISR
	
	endInterrupt:
	lw t0, (sp)
	lw t1, 4(sp)
	lw ra, 8(sp)
	addi sp, sp, 12
mret

CONFIG_TIMER: 
	la t1, TIMER
	sw zero, (t1)
	li t0, 25000000
	sw t0, 8(t1)
	srli t0, t0, 16
	sw t0, 12(t1)			# set the starting value of timer
	li t0, 0b0111
	sw t0, 4(t1)			# set the timer to request interrupt and starting running right away
ret

CONFIG_KEYS: 
	la t1, KEY_BASE
	li t0, 0b1111
	sw t0, 8(t1)			# set the key to request interrupt
ret

KEY_ISR:
	addi sp, sp, -8
	sw t0, (sp)
	sw t1, 4(sp)
	
	la t0, KEY_BASE
	lw t1, 12(t0)
	sw t1, 12(t0)			# reset edge capture
	
	la t0, RUN
	lw t1, (t0)
	xori t1, t1, 1			# switch run status
	sw t1, (t0)
	
	lw t0, (sp)
	lw t1, 4(sp)
	addi sp, sp, 8
	ret
TIMER_ISR:
	addi sp, sp, -8
	sw t0, (sp)
	sw t1, 4(sp)
	
	la t0, TIMER
	sw zero, (t0)			# reset T0 register
	
	la t0, RUN
	lw t1, (t0)				# t1 has run value
	lw t0, (s1)				# t0 has count value
	add t0, t0, t1			# increment count by run
	li t1, 255
	bgt t0, t1, resetCount	# if the counter is higher than 255, reset to zero
	sw t0, (s1)
	
	lw t0, (sp)
	lw t1, 4(sp)
	addi sp, sp, 8
	ret
	resetCount:
	sw zero, (s1)
	
	lw t0, (sp)
	lw t1, 4(sp)
	addi sp, sp, 8
	ret
	
.data
/* Global variables */
.global  COUNT
COUNT:  .word    0x0            # used by timer

.global  RUN                    # used by pushbutton KEYs
RUN:    .word    0x1            # initial value to increment COUNT

.end

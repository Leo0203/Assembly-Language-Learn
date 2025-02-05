.global _start
_start:

    .equ KEYS_BASE, 0xFF200050
    .equ LEDS, 0xFF200000
	
	li s5, 500000	 # count down
	li s6, 500000   # for comparing only
pollKEYS:
	li s5, 500000
    la t2, KEYS_BASE
    lw t3, 0xC(t2)   # Load edge capture register
    andi t4, t3, 0xF # Mask lower 4 bits (detect any key press)
    beqz t4, pollKEYS

KEYSPressed:
    sw t4, 0xC(t2)   # Reset the edge capture register
	beq s5, s6, CounterLoop
    # If running, stop the timer by returning to pollKEY
    j pollKEYS

LEDLoop:
	li s5, 500000	 # restore count down
    la s3, LEDS
    lw s2, (s3)
    addi s2, s2, 1
    sw s2, (s3)      # Update LED display
    li s4, 255
    ble s2, s4, CounterLoop

    # Reset LED when reaching 255
    sw zero, (s3)

CounterLoop:
	addi s5, s5, -1
	bnez s5, checkKeyDuringLoop
	j LEDLoop

checkKeyDuringLoop:
    la t2, KEYS_BASE
    lw t3, 0xC(t2)   # Load edge capture register
    andi t4, t3, 0xF # Mask lower 4 bits (detect any key press)
    beqz t4, CounterLoop	# return counter loop if no key pressed
    j KEYSPressed

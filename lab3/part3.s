.global _start
_start:
    .equ TIMER_BASE, 0xFF202000
    .equ KEYS_BASE, 0xFF200050
    .equ LEDS, 0xFF200000

    # Initialize Timer
    la t0, TIMER_BASE
    li t1, 25000000
    sw t1, 0x8(t0)   # Load lower 16 bits of start count
    srli t1, t1, 16
    sw t1, 0xC(t0)   # Load higher 16 bits of start count
	
pollKEYS:
    la t2, KEYS_BASE
    lw t3, 0xC(t2)   # Load edge capture register
    andi t4, t3, 0xF # Mask lower 4 bits (detect any key press)
    beqz t4, pollKEYS

KEYSPressed:
    sw t4, 0xC(t2)   # Reset the edge capture register
    lw t5, (t0)
    andi t5, t5, 2   # Check RUN bit
    beqz t5, startTimer  # If timer is not running, start it

    # If running, stop the timer
    li s0, 8
    sw s0, 0x4(t0)   # Stop the timer
    j pollKEYS

startTimer:
    li s1, 0b0110    # Enable continuous and running
    sw s1, 0x4(t0)   # Start the timer
    j LEDLoop

LEDLoop:
    la s3, LEDS
    lw s2, (s3)
    addi s2, s2, 1
    sw s2, (s3)      # Update LED display
    li s4, 255
    ble s2, s4, CounterLoop

    # Reset LED when reaching 255
    sw zero, (s3)

CounterLoop:
    lw s5, (t0)
    andi s5, s5, 1   # Check TO flag
    beqz s5, checkKeyDuringLoop
    sw zero, (t0)    # Clear TO flag
	j LEDLoop

checkKeyDuringLoop:
    la t2, KEYS_BASE
    lw t3, 0xC(t2)   # Load edge capture register
    andi t4, t3, 0xF # Mask lower 4 bits (detect any key press)
    beqz t4, CounterLoop	# return counter loop if no key pressed
    j KEYSPressed

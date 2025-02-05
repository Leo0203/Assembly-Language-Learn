.global _start
_start:
	.equ LEDS, 0xFF200000
	.equ KEYS_BASE, 0xFF200050
	la t0, LEDS
	la t1, KEYS_BASE
	li t3, 1
	li t4, 15
pollKEY0:
	lw t2, (t1)			# load data register to t2
	andi t2, t2, 1		# take right most digit which is data for key0
	beqz t2, pollKEY1	# if not pressed (0) poll key1
	KEY0Pressed:
	lw t2, (t1)			# load data register to t2
	andi t2, t2, 1		# take right most digit which is data for key0
	beqz t2, KEY0Released	# if relased go to released section
	j KEY0Pressed		# keep looping until released
	KEY0Released:
	sw t3, (t0)			# set data of the LED to one
	j pollKEY1			# poll the next key
pollKEY1:
	lw t2, (t1)
	andi t2, t2, 2
	beqz t2, pollKEY2
	KEY1Pressed:
	lw t2, (t1)
	andi t2, t2, 2
	beqz t2, KEY1Released
	j KEY1Pressed
	KEY1Released:
	lw t5, (t0)
	bge t5, t4, pollKEY2
	# if the number is less than 15
	addi t5, t5, 1
	sw t5, (t0)
	j pollKEY2
pollKEY2:
	lw t2, (t1)
	andi t2, t2, 4
	beqz t2, pollKEY3
	KEY2Pressed:
	lw t2, (t1)
	andi t2, t2, 4
	beqz t2, KEY2Released
	j KEY2Pressed
	KEY2Released:
	lw t5, (t0)
	ble t5, t3, KEY2Reset
	# if the number is bigger than 1
	addi t5, t5, -1
	sw t5, (t0)
	j pollKEY3
	# since pressing any key after key3 is pressed should deplay one on the LEDs
	# key2 requires special reset section whereas key0 and key1 would not
	# because key0 is reseting to one and key1 would add one to zero
	KEY2Reset:	
	sw t3, (t0)
	j pollKEY3
pollKEY3:
	lw t2, (t1)
	andi t2, t2, 8
	beqz t2, pollKEY0
	KEY3Pressed:
	lw t2, (t1)
	andi t2, t2, 8
	beqz t2, KEY3Released
	j KEY3Pressed
	KEY3Released:
	sw zero, (t0)
	j pollKEY0
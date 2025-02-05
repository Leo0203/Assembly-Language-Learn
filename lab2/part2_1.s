.global _start
_start:

li s0, 718293	# the student number
la s1, result	# s1 pointer to result
la s2, Snumbers	# s2 pointer to first element of Snumbers
li s3, -1		# s3 stores the grade of the student, -1 if not found
la s4, Grades	# s4 pointer to grade

li t0, 0		# index of elements currently pointing to
li t1, 0		# for checking the end of Snumber

loopSnumber:	#loop to find the correct index of the number stored in s0 in Snumbers
lw s5, 0(s2)		# take the number from the s2 pointer
beq s5, t1, finished
beq s5, s0, loopGrade
addi s2, s2, 4
addi t0, t0 ,1
j loopSnumber

loopGrade:		#loop to find the grade at cooresponding index
lw s3, 0(s4)
addi s4, s4, 4
addi t0, t0, -1
blt t0, t1, finished
j loopGrade

finished:
sw s3, 0(s1)	# store value of s3 into address in s1(result)

iloop: j iloop

/* result should hold the grade of the student number put into s0, or
-1 if the student number isn't found */ 

result: .word 0
		
/* Snumbers is the "array," terminated by a zero of the student numbers  */
Snumbers: .word 10392584, 423195, 644370, 496059, 296800
        .word 265133, 68943, 718293, 315950, 785519
        .word 982966, 345018, 220809, 369328, 935042
        .word 467872, 887795, 681936, 0

/* Grades is the corresponding "array" with the grades, in the same order*/
Grades: .word 99, 68, 90, 85, 91, 67, 80
        .word 66, 95, 91, 91, 99, 76, 68  
        .word 69, 93, 90, 72
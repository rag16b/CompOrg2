	.text
	la	$t7,A1
	la	$t6,A2
	la	$t5,A3
	lw	$s6,0($t6)
	lw	$s5,0($t5)
	addi	$t0,$0,100
	j	TEST
LOOP:	sll	$t1,$t6,2
	add	$s5,$s5,$t1
	add	$t2,$t1,$t7
	sw	$s5,0($t2)
	addi	$t6,$t6,1
TEST:	beq	$s6,$t0,LOOP
	sw	$s6,0($t6)
	sw	$s5,0($t5)
	.data
A1:	.space	400000
A2:	.word	1
A3:	.word	0

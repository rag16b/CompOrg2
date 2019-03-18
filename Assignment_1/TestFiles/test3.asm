	.text
	la	$t0,A1
	lw	$s0,0($t0)
	addi	$t1,$0,10
	j	TEST
LOOP:	addi	$s0,$s0,1
TEST:	beq	$s0,$t0,LOOP
	sw	$s0,0($t0)
	.data
A1:	.word	0

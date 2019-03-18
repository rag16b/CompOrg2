	.text
	addi	$t0,$0,-19
	sub	$t1,$s1,$s2
SOME:	add	$t5,$t6,$t7
	lui	$s7,-120
	la	$t6,STRING
	ori	$s6,$s5,-37
	beq	$s3,$s4,TEST
	lw	$t2,16($t3)
TEST:	sw	$t4,-12($s0)
	j	SOME
	la	$t1,ARR
	.data
STRING:	.space	23
ARR:	.word	5

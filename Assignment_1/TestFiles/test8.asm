	.text
	la	$t1,L1
	ori	$s0,$0,0
	ori	$s1,$s1,0
L2:	lw	$t2,0($t1)
	add	$s0,$s0,$t2
	addi	$s1,$s1,-7
	addi	$t1,$t1,4
	beq	$t2,$0,L2
	lui	$s0,98765
	ori	$t0,$s0,0
	ori	$s0,$s0,-4
	.data
L1:	.word	-1
L3:	.space	15

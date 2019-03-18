	.text
main:	la	$s0,A1
	lw	$t6,0($s0)
	addi	$t7,$t6,1
	sw	$t7,0($s0)
	.data
A1:	.space	4
A2:	.word 	5

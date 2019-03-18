	.text
main:	la 	$s0,arr
	lw 	$t6,0($s0)
	addi	$t7,$t6,1
	sw 	$t7,0($s0)
	.data
arr:	.space	4

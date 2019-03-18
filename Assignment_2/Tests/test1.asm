	.text
	ori	$s0,$0,24
	lw	$t0,0($s0)
	lw	$t1,4($s0)
	add	$t2,$t0,$t1
	sw	$t2,8($s0)
	halt
	.data
Data1:	.word	15
Data2:	.word	25
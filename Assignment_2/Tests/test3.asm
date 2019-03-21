	.text
	add	$t2,$t0,$t1
	sub	$t3,$t1,$t0
	sll	$t4,$t0,1
	andi	$t5,$t0,1
	ori	$s0,%0,24
	halt
	.data
Data1:	.word	15
Data2:	.word	25

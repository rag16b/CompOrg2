	.text
	ori	$s0,$0,52
	lw	$t0,0($s0)
	lw	$t1,4($s0)
	lw	$t2,8($s0)
	lw	$t3,12($s0)
	lw	$t4,16($s0)
	add	$t5,$t1,$t2
	sub	$t6,$t5,$t4
	sll	$t7,$t6,1
	andi	$t8,$t7,4
	noop
	sw	$t8,20($s0)
	halt
	.data
Data1:	.word	12
Data2:	.word	25
Data3:	.word	2345
Data4:	.word	64
Data5:	.word	53

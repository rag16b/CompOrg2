	.text
	ori	$s0,$0,36
	ori	$t5,$0,1
	lw	$t1,0($s0)
	bne	$t5,$t6,LOOP
	sub	$t2,$t0,$t1
	lw	$t5,8($s0)
LOOP:	sw	$t3,8($s0)
	sll	$t0,$t1,5
	halt
	.data
Data1:	.word	30
Data2:	.word	28
Data3:	.word	2

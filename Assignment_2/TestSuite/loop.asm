	.text
	ori	$s0,$0,24
	noop
	lw	$t0,0($s0)
	lw	$t1,4($s0)
LOOP:	add	$t2,$s1,$t1
	sub	$t3,$s1,$s0
	bne	$t1,$t2,LOOP
	halt
	.data
Data1:	.word	1
Data2:	.word	5

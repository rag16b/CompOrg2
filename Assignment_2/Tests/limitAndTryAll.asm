	.text
	ori	$s0,$0,400
	lw	$t0,56($s0)
	add	$t1,$t0,$t0
	add	$t2,$t1,$t0
	add	$t3,$t2,$t0
	add	$t4,$t3,$t0
	add	$t5,$t4,$t0
	add	$t6,$t5,$t0
	add	$t7,$t6,$t0
	add	$t1,$t0,$t0
	add	$t2,$t1,$t0
	add	$t3,$t2,$t0
	add	$t4,$t3,$t0
	add	$t5,$t4,$t0
	add	$t6,$t5,$t0
	add	$t7,$t6,$t0
	add	$t1,$t0,$t0
	add	$t2,$t1,$t0
	add	$t3,$t2,$t0
	add	$t4,$t3,$t0
	add	$t5,$t4,$t0
	add	$t6,$t5,$t0
	add	$t7,$t6,$t0
	add	$t1,$t0,$t0
	add	$t2,$t1,$t0
	add	$t3,$t2,$t0
	add	$t4,$t3,$t0
	add	$t5,$t4,$t0
	add	$t6,$t5,$t0
	add	$t7,$t6,$t0
	add	$t1,$t0,$t0
	add	$t2,$t1,$t0
	add	$t3,$t2,$t0
	add	$t4,$t3,$t0
	add	$t5,$t4,$t0
	add	$t6,$t5,$t0
	add	$t7,$t6,$t0
	add	$t1,$t0,$t0
	add	$t2,$t1,$t0
	add	$t3,$t2,$t0
	add	$t4,$t3,$t0
	add	$t5,$t4,$t0
	add	$t6,$t5,$t0
	add	$t7,$t6,$t0
	add	$t1,$t0,$t0
	add	$t2,$t1,$t0
	add	$t3,$t2,$t0
	add	$t4,$t3,$t0
	add	$t5,$t4,$t0
	add	$t6,$t5,$t0
	add	$t7,$t6,$t0
	add	$t1,$t0,$t0
	add	$t2,$t1,$t0
	add	$t3,$t2,$t0
	add	$t4,$t3,$t0
	add	$t5,$t4,$t0
	add	$t6,$t5,$t0
	add	$t7,$t6,$t0
	add	$t1,$t0,$t0
	add	$t2,$t1,$t0
	add	$t3,$t2,$t0
	add	$t4,$t3,$t0
	add	$t5,$t4,$t0
	add	$t6,$t5,$t0
	add	$t7,$t6,$t0
	add	$t1,$t0,$t0
	add	$t2,$t1,$t0
	add	$t3,$t2,$t0
	add	$t4,$t3,$t0
	add	$t5,$t4,$t0
	add	$t6,$t5,$t0
	add	$t7,$t6,$t0
	add	$t1,$t0,$t0
	add	$t2,$t1,$t0
	add	$t3,$t2,$t0
	add	$t4,$t3,$t0
	add	$t5,$t4,$t0
	add	$t6,$t5,$t0
	add	$t7,$t6,$t0
	add	$t1,$t0,$t0
	add	$t2,$t1,$t0
	add	$t3,$t2,$t0
	add	$t4,$t3,$t0
	add	$t5,$t4,$t0
	add	$t6,$t5,$t0
	add	$t7,$t6,$t0
	add	$t1,$t0,$t0
	add	$t2,$t1,$t0
	add	$t3,$t2,$t0
	add	$t4,$t3,$t0
	add	$t5,$t4,$t0
	add	$t6,$t5,$t0
	add	$t7,$t6,$t0
	noop
	bne	$t0,$t1,SKIP
	sll	$t2,$t2,1
	andi	$t3,$t3,0
SKIP:	sub	$t0,$t1,$t7
	sw	$t0,0($s0)
	halt
	.data
Data1:	.word	1
Data2:	.word	2
Data3:	.word	3
Data4:	.word	4
Data5:	.word	5
Data6:	.word	6
Data7:	.word	7
Data8:	.word	8
Data9:	.word	9
Data10:	.word	10
Data11:	.word	11
Data12:	.word	12
Data13:	.word	13
Data14:	.word	14
Data15:	.word	15
Data16:	.word	16
Data17:	.word	17
Data18:	.word	18
Data19:	.word	19
Data20:	.word	20
Data21:	.word	21
Data22:	.word	22
Data23:	.word	23
Data24:	.word	24
Data25:	.word	25
Data26:	.word	26
Data27:	.word	27
Data28:	.word	28
Data29:	.word	29
Data30:	.word	30
Data31:	.word	31
Data32:	.word	32

	.text
main:	sll	$t1,$t6,2
	add	$s5,$s5,$t1
	la	$s0,arr
	j	skip
	add	$t2,$t1,$t7
	bne	$t5,$t2,skip
	nor	$t2,$t1,$t7
skip:	addi	$t0,$t1,10
	ori	$s0,$s1,10
	bne	$t5,$t2,skip
	lui	$s4,1
	j	skip
	lw      $t6,0($s0)
	sw      $t7,0($s0)
	.data
arr:	.space  4

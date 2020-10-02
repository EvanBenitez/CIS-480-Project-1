.data
Array1: .word 15
.space 4
Array2: .word 15,16,17
.word 1,2,3
.text
add $0,$4,$31
There: sub $0,$4,$31
sll $1,$31,5
srl $1,$31,5
addi $1,$31,5
lui $1,256
ori $1,$31,4
lw $1,100($31)
sw $1,100($31)
la $1,Array1
la $31,Array3
.data
Array3: .word 8
.space 4
Array4: .word 4
.text
beq $1,$31,There
Here: beq $1,$31,Here
beq $1,$31,Yonder
bne $1,$31,There
rightHere: bne $1,$31,rightHere
beq $1,$31,Yonder
add $0,$4,$31
Yonder: add $0,$4,$31
j There
j Here

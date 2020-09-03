//THis structure contains the lable, instruction/directive, and the opperands.
//a non existant element is denoted by an empty string.
struct mips_line {
  char lable[40];     //mips line lable
  char inst_dir[15];  //mips directive or instruction
  char op1[40];      //first operand
  char op2[40];      //second operand
  char op3[40];      //third operand
};

//loads one line of a mips file into the given mips_line.
//returns 1 if a line is loaded and 0 if end of file.
int mips_par(FILE *,struct mips_line *);

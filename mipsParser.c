#include <stdio.h>
#include <stdlib.h>

//THis structure contains the lable, instruction/directive, and the opperands.
//a non existant element is denoted by an empty string.
struct mips_line {
  char lable[40];     //mips line lable
  char inst_dir[15];  //mips directive or instruction
  char op1[40];      //first operand
  char op2[40];      //second operand
  char op3[40];      //third operand
};

//parse a single line of mips file, returns 1 if line read or 0 if end of file
//loads one line of a mips file into the given mips_line.
//returns 1 if a line is loaded and 0 if end of file.
int mips_par(FILE *in,struct mips_line *line){
  int index = 0;
  char sample[100];
  char lable = 0;

  //set null values for mips_line fields
  line->lable[0]=0;
  line->inst_dir[0]=0;
  line->op1[0]=0;
  line->op2[0]=0;
  line->op3[0]=0;

  //return 0 if end of FILE
  //all struct values are empty strings
  if(!fread(&sample[index],1,1,in)){
    return 0;
  }

  index++;

  //reads file into sample. checks for end of line of file
  while(fread(&sample[index],1,1,in) && sample[index] != '\n'){
    //set lable flag if lable present inf the line
    if(sample[index] == ':'){
      lable = 1;
    }
    //replace ',' with ' ' for sscanf funciton compatibilty
    if(sample[index] == ','){
      sample[index] = ' ';
    }
    index++;
  }
  sample[index] = 0;

  //slightly different structure of sscanf if lable present
  if(lable == 0){
    sscanf(sample,"%s %s %s %s",line->inst_dir, line->op1, line->op2, line->op3);
  }
  else{
    sscanf(sample,"%s %s %s %s %s",line->lable, line->inst_dir, line->op1, line->op2, line->op3);
  }
  return 1;
}

//test for mips_par
int main(int argc,char *argv[]){
  if( argc == 2){
    struct mips_line line;
    FILE *mips_file;
    if((mips_file = fopen(argv[1],"r")) == NULL){
      perror("Unable to open file");
      exit(0);
    }

    while(mips_par(mips_file, &line)){
      printf("%s \n",line.lable);
      printf("%s \n",line.inst_dir);
      printf("%s \n",line.op1);
      printf("%s \n",line.op2);
      printf("%s \n",line.op3);
    }
  }
  else{
    printf("Improper input");
  }
}

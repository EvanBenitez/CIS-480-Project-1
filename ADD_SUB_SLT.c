#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct {
  const char *name;
  const unsigned int address;
} instMap[] = {
  {"add", 32 },
  {"sub", 34 },
  {"slt", 42},
  { NULL, 0},
};

int embedder(int,int,int);

//THis structure contains the lable, instruction/directive, and the opperands.
//a non existant element is denoted by an empty string.
struct mips_line {
  char lable[40];     //mips line lable
  char inst_dir[15];  //mips directive or instruction
  char op1[40];      //first operand
  char op2[40];      //second operand
  char op3[40];      //third operand
};

int convertToMachCode(struct mips_line line){

  unsigned int inst;

  //checks if any instruction matches and sets the decimal to inst
  for(int i=0; instMap[i].name != NULL; i++) {
    if(strcmp(line.inst_dir, instMap[i].name) == 0) {
      inst = instMap[i].address;
    }
  }

  int opt = 0;
  int op1;
  //convert the string into a int and strip out the $
  //first operand is just a number with a $ in front
  if(line.op1[2] == 0){
    opt = line.op1[1] - 48;
  }
  else{
    opt = line.op1[2] - 48 + (line.op1[1] - 48) * 10;
  }
  inst = embedder(inst,opt,11);

  //ditto for op2
  if(line.op2[2] == 0){
    opt = line.op2[1] - 48;
  }
  else{
    opt = line.op2[2] - 48 + (line.op2[1] - 48) * 10;
  }
  inst = embedder(inst,opt,16);

  //and op3
  if(line.op3[2] == 0){
    opt = line.op3[1] - 48;
  }
  else{
    opt = line.op3[2] - 48 + (line.op3[1] - 48) * 10;
  }
  inst = embedder(inst,opt,21);

  return inst;
}



//insert a number into an int, starting at the offset
int embedder(int base,int embed,int off){
  embed = embed << off;
  base = base | embed;

  return base;
}

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

//print an check as binary
void binary_checker(int check){
  char out[33];
  out[32] = 0;

  int i;
  for(i = 31; i >= 0; i--){
    int dig = check & 1;
    check = check >> 1;
    if(dig == 1){
      out[i] = '1';
    }
    else{
      out[i] = '0';
    }
  }

  for(i = 0; i < 32; i++){
    printf("%c", out[i]);
    if((i+1)%4 == 0){
      printf(" ");
    }
  }
  printf("\n");
}

//testing ADD
int main(int argc,char *argv[]){
  if( argc == 2){
    struct mips_line line;
    FILE *mips_file;
    if((mips_file = fopen(argv[1],"r")) == NULL){
      perror("Unable to open file");
      exit(0);
    }

    while(mips_par(mips_file, &line)){
      for(int i=0; instMap[i].name != NULL; i++)
      if( 0 == strcmp(line.inst_dir, instMap[i].name)){
        binary_checker(convertToMachCode(line));
      }
    }
  }
  else{
    printf("Improper input");
  }
}

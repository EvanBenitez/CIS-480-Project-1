#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct mips_line {
  char lable[40];     //mips line lable
  char inst_dir[15];  //mips directive or instruction
  char op1[40];      //first operand
  char op2[40];      //second operand
  char op3[40];      //third operand
};

struct symbol_table{
  int position;
  char lable[11];
};
static int lable_count = 0;

struct symbol_table *new_table(int size){
  //try to allocate space foe symbol_table array
  struct symbol_table *table = malloc(sizeof(struct symbol_table) * size);

  if(table == NULL){
    return NULL;
  }

  return table;
}

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







struct symbol_table *create_table(FILE *code){
  struct symbol_table *table = new_table(10);
  struct mips_line line;
  char segment = 'd';
  int data_counter = 0;
  int text_counter = 512;

  while(mips_par(code,&line)){
    if(0 == strcmp(line.inst_dir,".data")){
      segment = 'd';
    }
    else if(0 == strcmp(line.inst_dir,".text")){
      segment = 't';
    }
    else if(segment == 'd'){
      if(line.lable[0] != 0){
        strcpy(table[lable_count].lable,line.lable);
        table[lable_count].position = data_counter;
        table[lable_count].lable[strlen(table[lable_count].lable)-1] = 0;
        lable_count++;
      }
      if(0 == strcmp(line.inst_dir,".word")){
        data_counter += 4;
      }
      else if(0 == strcmp(line.inst_dir,".space")){
        int inc;
        if(line.op1[1] == 'x'){
          inc = strtol(line.op1,NULL,0);
        }
        else{
          inc = strtol(line.op1,NULL,10);
        }
        data_counter += inc;
      }
    }
    else if(segment == 't'){
      if(line.lable[0] != 0){
        strcpy(table[lable_count].lable,line.lable);
        table[lable_count].position = text_counter;
        table[lable_count].lable[strlen(table[lable_count].lable)-1] = 0;
        lable_count++;
      }
      if(0 == strcmp(line.inst_dir,"la")){
        text_counter += 8;
      }
      else{
        text_counter += 4;
      }
    }
    if(lable_count - 10 >= 0){
      table = realloc(table,sizeof(struct symbol_table) * (lable_count + 10));
    }
  }
  fseek(code,0,SEEK_SET);
  return table;
}

int main(int argc, char *argv[]){
  if(argc == 2){
    struct symbol_table *table;
    FILE *code;

    if((code = fopen(argv[1],"r")) == NULL){
      perror("Unable to open file \n");
      exit(0);
    }

    table = create_table(code);
    for(int i = 0; i < lable_count; i++){
      printf("%s %d\n",table[i].lable,table[i].position);
    }
  }
  else{
    printf("Incorrect number of arguments");
  }
}

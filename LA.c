#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int embedder(int,int,int);

struct symbol_table{
  int position;
  char lable[11];
};
static int lable_count = 0;

//THis structure contains the lable, instruction/directive, and the opperands.
//a non existant element is denoted by an empty string.
struct mips_line {
  char lable[40];     //mips line lable
  char inst_dir[15];  //mips directive or instruction
  char op1[40];      //first operand
  char op2[40];      //second operand
  char op3[40];      //third operand
};


struct symbol_table *new_table(int size){
  //try to allocate space foe symbol_table array
  struct symbol_table *table = malloc(sizeof(struct symbol_table) * size);

  if(table == NULL){
    return NULL;
  }

  return table;
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


int LUI(struct mips_line line){
  unsigned int inst = 0x3c000000;
  int opt ;

  //convert the string into a int and strip out the $
  //first operand is just a number with a $ in front
  if(line.op1[2] == 0){
    opt = line.op1[1] - 48; //number start at 48 on the ascii table
  }
  else{
    opt = line.op1[2] - 48 + (line.op1[1] - 48) * 10;
  }
  inst = embedder(inst,opt,16);

  //check for formate (either hex or dec)
  if(line.op2[1] == 'x'){
    opt = (int) strtol(line.op2,NULL,0);
  }
  else{
    opt = (int) strtol(line.op2,NULL,10);
  }
  inst = embedder(inst,opt,0);



  return inst;
}

struct {
  const char *name;
  const unsigned int address;
} typeII[] = {
  {"addi", 0x20000000},
  {"ori", 0x34000000},
  {NULL, 0}
};

int typeII_func(struct mips_line line){
  unsigned int inst;
  for(int i = 0; typeII[i].name != NULL; i++){
    if(strcmp(typeII[i].name,line.inst_dir) == 0){
      inst = typeII[i].address;
    }
  }
  int opt;

  //convert the string into a int and strip out the $
  //first operand is just a number with a $ in front
  if(line.op1[2] == 0){
    opt = line.op1[1] - 48; //number start at 48 on the ascii table
  }
  else{
    opt = line.op1[2] - 48 + (line.op1[1] - 48) * 10;
  }
  inst = embedder(inst,opt,16);

  //ditto for op2
  if(line.op2[2] == 0){
    opt = line.op2[1] - 48;
  }
  else{
    opt = line.op2[2] - 48 + (line.op2[1] - 48) * 10;
  }
  inst = embedder(inst,opt,21);

  //check for formate (either hex or dec)
  if(line.op3[1] == 'x'){
    opt = (int) strtol(line.op3,NULL,0);
  }
  else{
    opt = (int) strtol(line.op3,NULL,10);
  }
  if(opt < 0){
    opt = opt & 0x0000ffff;
  }
  inst = embedder(inst,opt,0);


  return inst;
}

void int_string(int num, char *string){
  int i = 0;
  int j;
  char temp[17];

  while(num != 0){
    temp[i] = num % 10 + 48;
    num = num / 10;
    i++;
  }
  string[i] = 0;
  i--;
  j = i;
  while(i >= 0){
    string[j-i] = temp[i];
    i--;
  }
}

long LA(struct mips_line line, struct symbol_table *table){
  long pseudo;
  struct mips_line portion;   //create a mips_line to call the 2 sub instructions

  //for LUI instruction
  strcpy(portion.inst_dir, "lui");
  strcpy(portion.op1, line.op1);

  for(int i = 0; i < lable_count; i++){
    if(strcmp(line.op2,table[i].lable)==0){
      int_string((table[i].position & 0xffff0000) >> 16, portion.op2);
      i = lable_count;
    }
  }
  pseudo = ((long) LUI(portion)) << 32;

  //for ORI instruction
  strcpy(portion.inst_dir, "ori");
  strcpy(portion.op2, "$0");

  for(int i = 0; i < lable_count; i++){
    if(strcmp(line.op2,table[i].lable)==0){
      int_string((table[i].position & 0xffff), portion.op3);
      i = lable_count;
    }
  }
  pseudo = pseudo | ((long) typeII_func(portion));

  return pseudo;
}


//insert a number into an int, starting at the offset
int embedder(int base,int embed,int off){
  embed = embed << off;
  base = base | embed;

  return base;
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

void long_checker(long check){
  char out[64];

  int i;
  for(i = 63; i >= 0; i--){
    int dig = check & 1;
    check = check >> 1;
    if(dig == 1){
      out[i] = '1';
    }
    else{
      out[i] = '0';
    }
  }

  for(i = 0; i < 64; i++){
    printf("%c", out[i]);
    if((i+1)%4 == 0){
      printf(" ");
    }
    if((i+1)%32 == 0){
      printf("- ");
    }
  }
  printf("\n");
}

int main(int argc,char *argv[]){
  if( argc == 2){
    struct mips_line line;
    FILE *mips_file;
    if((mips_file = fopen(argv[1],"r")) == NULL){
      perror("Unable to open file");
      exit(0);
    }

    struct symbol_table *table = create_table(mips_file);
    while(mips_par(mips_file, &line)){
      if( 0 == strcmp(line.inst_dir, "la")){
        long_checker(LA(line,table));
      }
    }
  }
  else{
    printf("Improper input");
  }
}

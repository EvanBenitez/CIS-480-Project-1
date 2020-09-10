#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//structure for a symbol_table element
struct symbol_table{
  int position;
  char lable[11];
};
static int lable_count = 0; //counter for number of symbol table elements

//THis structure contains the lable, instruction/directive, and the opperands.
//a non existant element is denoted by an empty string.
struct mips_line {
  char lable[40];     //mips line lable
  char inst_dir[15];  //mips directive or instruction
  char op1[40];      //first operand
  char op2[40];      //second operand
  char op3[40];      //third operand
};

//structure for the R-type instructions
struct {
  const char *name;
  const unsigned int address;
} r_type_inst[] = {
  {"add", 32 },
  {"sub", 34 },
  {"slt", 42},
  {"sll", 0},
  {"srl", 2},
  { NULL, 0},
};

//structure for some of the elements of the I-type instructions
struct {
  const char *name;
  const unsigned int address;
} typeII[] = {
  {"addi", 0x20000000},
  {"ori", 0x34000000},
  {NULL, 0}
};

struct {
  const char *name;
  const unsigned int address;
} i_type_inst[] = {
  {"lw", 0x8c000000},
  {"sw", 0xac000000},
  { NULL, 0},
};

//convert a int into a string (necessary for function compatibiltiy)
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

//allocates space for a symbol_table array and return a pointer to it
struct symbol_table *new_table(int size){
  //try to allocate space foe symbol_table array
  struct symbol_table *table = malloc(sizeof(struct symbol_table) * size);

  if(table == NULL){
    return NULL;
  }

  return table;
}

//creates a symbol_table array with all the lables
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

//function for r-type instructions (add,sub,slt,sll,srl)
int r_type_machine_code(struct mips_line line){

  unsigned int inst;

  //checks if any instruction matches and sets the decimal to inst
  for(int i=0; r_type_inst[i].name != NULL; i++) {
    if(strcmp(line.inst_dir, r_type_inst[i].name) == 0) {
      printf("%s: ", r_type_inst[i].name); //just adding for convinience in testing
      inst = r_type_inst[i].address;
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

  if(line.op3[0] != 36) {
    inst = embedder(inst,opt,16);
  }
  else{
    inst = embedder(inst,opt,21);
  }

  //and op3
  //36 = ASCII code for "$"
  //if no dollar sign, it will start its offset at shamt
  //else at rd
  if(line.op3[0] != 36){
    if(line.op3[1] == 'x'){
      opt = (int) strtol(line.op3,NULL,0);
    }
    else{
      opt = (int) strtol(line.op3,NULL,10);
    }
  inst = embedder(inst,opt,6);
  }
  else{
    if(line.op3[2] == 0){
      opt = line.op3[1] - 48;
    }
    else{
      opt = line.op3[2] - 48 + (line.op3[1] - 48) * 10;
    }
  inst = embedder(inst,opt,16);
  }

  return inst;
}

//this handles some of the I-type instructions (addi and ori)
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

//this handles sw and lw
int i_type_machine_code(struct mips_line line){
  unsigned int inst;
  //checks if any instruction matches and sets the decimal to inst
  for(int i=0; i_type_inst[i].name != NULL; i++) {
    if(strcmp(line.inst_dir, i_type_inst[i].name) == 0) {
      printf("%s: ", i_type_inst[i].name); //just adding for convinience in testing
      inst = i_type_inst[i].address;
    }
  }

  int opt = 0;
  int op1;

  //rd
  if(line.op1[2] == 0){
    opt = line.op1[1] - 48;
  }
  else{
    opt = line.op1[2] - 48 + (line.op1[1] - 48) * 10;
  }
  inst = embedder(inst,opt,16);

  //rs
  for(int i=0; i < strlen(line.op2); i++){
    if(line.op2[i] == 40){ //"("
      if(line.op2[i+3] == 0 || 41) { //")"
        opt = line.op2[i+2] - 48;
      }
      else{
        opt = line.op2[i+3] - 48 + (line.op2[i+2] - 48) * 10;
      }
    }
  }
  inst = embedder(inst,opt,21);

  //Offset Address
  opt = strtol(line.op2, NULL, 10);
  inst = embedder(inst,opt,32);

  return inst;
}

//function for lui instruction
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

//function for pseudo instruction la, returns a long not an int so beware
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




int main(int argc,char *argv[]){
  if(argc == 2){
    FILE *input;
    struct mips_line line;

    if((input = fopen(argv[1],"w")) == NULL){
      perror("Cannot open file");
      exit(0);
    }
    struct symbol_table *table = create_table(input);
    FILE *out;

    //create name for out file.
    for(int i = 0; i < strlen(argv[1]); i++){
      if(argv[1][i] == '.'){
        argv[1][i] = 0;
        i += 3;
      }
    }
    strcat(argv[1],".out");

    if(out = fopen(argv[1], "w")){
      int text = 512;
      int data = 0;
      char segment = 'd';

      while(mips_par(input, &line)){
        printf("here");
        if(strcmp(line.inst_dir,".data") == 0){
          segment = 'd';
          fseek(out,data,SEEK_SET);
        }
        else if(strcmp(line.inst_dir,".text") == 0){
          segment = 't';
          fseek(out,text,SEEK_SET);
        }
        else if(segment == 't'){
          if(strcmp(line.inst_dir,"lui") == 0){
            int temp = LUI(line);
            fwrite(&temp,sizeof(int),1,out); //potential issues
            text += 4;
          }
          else if(strcmp(line.inst_dir,"la") == 0){
            long temp = LA(line,table);
            fwrite(&temp,sizeof(long),1,out);
            text += 8;
          }
          else{
            for(int i = 0; r_type_inst[i].name != NULL; i++){
              if(i <= 1){
                if(strcmp(typeII[i].name,line.inst_dir) == 0){
                  int temp = typeII_func(line);
                  fwrite(&temp,sizeof(int),1,out);
                  text += 4;
                }
                else if(strcmp(i_type_inst[i].name,line.inst_dir) == 0){
                  int temp = i_type_machine_code(line);
                  fwrite(&temp,sizeof(int),1,out);
                  text += 4;
                }
              }
              if(strcmp(r_type_inst[i].name,line.inst_dir) == 0){
                int temp = r_type_machine_code(line);
                fwrite(&temp,sizeof(int),1,out);
                text += 4;
              }


            }
          }
        }
        else{
          if(strcmp(line.inst_dir,".word") == 0){
            int temp;
            if(line.op1[1] != 'x'){
              temp = strtol(line.op1,NULL,10);
            }
            else{
              temp = strtol(line.op1,NULL,0);
            }
            fwrite(&temp,sizeof(int),1,out);
            data += 4;
          }
          else if(strcmp(line.inst_dir,".space") == 0){
            int temp;
            if(line.op1[1] != 'x'){
              temp = strtol(line.op1,NULL,10);
            }
            else{
              temp = strtol(line.op1,NULL,0);
            }
            char zero = 0;
            for(int i = 0; i < temp; i++){
              fwrite(&zero,sizeof(char),1,out);
            }
            data += temp;
          }
        }
      }
      //fill out extra space in file
      char fill = 0;
      fseek(out,data,SEEK_SET);
      for(int i = data; i < 512; i++){
        fwrite(&fill,sizeof(char),1,out);
      }

      int finish = 0;
      fseek(out,text,SEEK_SET);
      for(int i = text; i < 1024; i=i+4){
        fwrite(&finish,sizeof(int),1,out);
      }


    }
    else{
      perror("Unable to create .out file");
    }

    free(table);
    fclose(out);
    fclose(input);
  }
  else{
    printf("Incorrect number of arguments.");
  }
}

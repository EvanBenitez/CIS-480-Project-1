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
  char op1[80];      //first operand
  char op2[15];      //second operand
  char op3[15];      //third operand
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
  char words[100];
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

  words[index] = sample[index];//.word contingincy
  index++;

  //reads file into sample. checks for end of line of file
  while(fread(&sample[index],1,1,in) && sample[index] != '\n'){
    //.word contingincy
    words[index] = sample[index];
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
  words[index] = 0;//.word contingincy

  //slightly different structure of sscanf if lable present
  if(lable == 0){
    sscanf(sample,"%s %s %s %s",line->inst_dir, line->op1, line->op2, line->op3);
    //after the fact fix for multiple items on single .word directive
    if(0==strcmp(line->inst_dir,".word")){
      sscanf(words,"%s %s",line->inst_dir, line->op1);
    }
    
  }
  else{
    sscanf(sample,"%s %s %s %s %s",line->lable, line->inst_dir, line->op1, line->op2, line->op3);
    //after the fact fix for multiple items on single .word directive
    if(0==strcmp(line->inst_dir,".word")){
      sscanf(words,"%s %s %s",line->lable, line->inst_dir, line->op1);
    }
  }

  return 1;
}

//allocates space for a symbol_table array and return a pointer to it
struct symbol_table *new_table(int size){
  //try to allocate space foe symbol_table array
  struct symbol_table *table = malloc(sizeof(struct symbol_table) * size);

  if(table == NULL){
    printf("failed to allocate\n");
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
        for (int i = strlen(line.op1) - 1; i > 0; i--){
          if(line.op1[i] == ',')
            data_counter += 4;
        }
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
      inst = r_type_inst[i].address;
    }
  }

  int opt = 0;
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

  if(strcmp(line.inst_dir,"sll") == 0 || strcmp(line.inst_dir,"srl") == 0){
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
      inst = i_type_inst[i].address;
    }
  }

  int opt = 0;

  //rt
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
      if(line.op2[i+3] == 0 || line.op2[i+3] == 41) { //")"
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
  //adjust for negetives
  if(opt < 0){
    opt = opt & 0x0000FFFF;
  }
  inst = embedder(inst,opt,0);

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
  strcpy(portion.op2, line.op1);

  for(int i = 0; i < lable_count; i++){
    if(strcmp(line.op2,table[i].lable)==0){
      int_string((table[i].position & 0xffff), portion.op3);
      i = lable_count;
    }
  }
  pseudo = pseudo | ((long) typeII_func(portion));

  return pseudo;
}

//function for branching insturctions beq & bne
int branch(struct mips_line line, struct symbol_table *table, int counter){
  int inst;
  int opt;

  if(strcmp(line.inst_dir, "beq") == 0){
    inst = 0x10000000;
  }
  else{
    inst = 0x14000000;
  }

  if(line.op1[2] == 0){
    opt = line.op1[1] - 48;
  }
  else{
    opt = line.op1[2] - 48 + (line.op1[1] - 48) * 10;
  }
  inst = embedder(inst,opt,21);

  if(line.op2[2] == 0){
    opt = line.op2[1] - 48;
  }
  else{
    opt = line.op2[2] - 48 + (line.op2[1] - 48) * 10;
  }
  inst = embedder(inst,opt,16);

  //get lable Address
  for(int i = 0;i < lable_count; i++){
    if(strcmp(line.op3, table[i].lable) == 0){
      opt = table[i].position;
      i = lable_count;
    }
  }
  opt = (opt - counter - 4)/4;
  //trunkate to 16 bits for negative numbers
  opt = opt & 0x0000ffff;
  inst = embedder(inst,opt,0);

  return inst;
}

int jump(struct mips_line line, struct symbol_table *table){
  unsigned int inst = 0x08000000;
  int opt;

  //get lable Address
  for(int i = 0; i < lable_count; i++){
    if(strcmp(line.op1, table[i].lable) == 0){
      opt = table[i].position;
    }
  }
  opt = (opt & 0x0fffffff) >> 2;
  inst = embedder(inst,opt,0);
  return inst;
}

//Reverse the bytes of an int for file output in big endian order
int big_end(int little){
  int big = 0;

  big = embedder(big,little & 0x000000FF ,24);
  big = embedder(big,(little & 0x0000FF00) >> 8 ,16);
  big = embedder(big,(little & 0x00FF0000) >> 16,8);
  big = embedder(big,(little & 0xFF000000) >> 24,0);

  return big;
}

//print list of words to a file
void print_word(char num[], int *pc, FILE *out){
  int i = 0;
  int complete = 0;
  char neg = 0;

  do{
    if(num[i] == '-'){
      neg = 1;
      i++;
    }
    if(num[i] != ',' && num[i] != 0){
      complete *= 10;
      complete += num[i] - 48;
    }
    else{
      if(neg == 1){
        complete *= -1;
      }
      complete = big_end(complete);
      if(fwrite(&complete,sizeof(int),1,out) != 1){
        perror("Writing unsuccessful\n");
        exit(1);
      }
      *pc += 4;
      complete = 0;
    }
    i++;
  }while(num[i-1] != 0);
}

int main(int argc,char *argv[]){
  if(argc == 2){
    FILE *input;
    struct mips_line line;

    if((input = fopen(argv[1],"r")) == NULL){
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
        int temp;
        if(strcmp(line.inst_dir,".data") == 0){
          segment = 'd';
          fseek(out,data,SEEK_SET);
        }
        else if(strcmp(line.inst_dir,".text") == 0){
          segment = 't';
          fseek(out,text,SEEK_SET);
        }
        else if(segment == 't'){
          //implement pseudo instructions
          if(strcmp(line.inst_dir,"la") == 0){
            long pseudo = LA(line,table);
            temp = (int)(pseudo >> 32);
            temp = big_end(temp);
            if(fwrite(&temp,sizeof(int),1,out) != 1){
              perror("Writing unsuccessful\n");
              exit(1);
            }
            temp = (int)(pseudo & 0x00000000ffffffff);
            temp = big_end(temp);
            if(fwrite(&temp,sizeof(int),1,out) != 1){
              perror("Writing unsuccessful\n");
              exit(1);
            }
            text += 8;
          }
          else{
            if(strcmp(line.inst_dir,"lui") == 0){
              temp = LUI(line);
            }
            else if(strcmp(line.inst_dir,"beq") == 0 || strcmp(line.inst_dir,"bne") == 0){
              temp = branch(line,table,text);
            }
            else if(strcmp(line.inst_dir,"j") == 0){
              temp = jump(line,table);
            }
            else{
              for(int i = 0; r_type_inst[i].name != NULL; i++){
                if(i <= 1){
                  if(strcmp(typeII[i].name,line.inst_dir) == 0){
                    temp = typeII_func(line);
                  }
                  else if(strcmp(i_type_inst[i].name,line.inst_dir) == 0){
                    temp = i_type_machine_code(line);
                  }
                }
                if(strcmp(r_type_inst[i].name,line.inst_dir) == 0){
                  temp = r_type_machine_code(line);;
                }
              }
            }
            temp = big_end(temp);
            if(fwrite(&temp,sizeof(int),1,out) !=1){
              perror("Fail to write\n");
              exit(1);
            }
            text += 4;
          }
        }
        else{
          if(strcmp(line.inst_dir,".word") == 0){
            print_word(line.op1,&data,out);
          }
          else if(strcmp(line.inst_dir,".space") == 0){
            int temp;
            if(line.op1[1] != 'x'){
              temp = strtol(line.op1,NULL,10);
            }
            else{
              temp = strtol(line.op1,NULL,0);
            }
            if(temp % 4 != 0)
              temp += 4 - (temp % 4);

            char zero = 0;
            for(int i = 0; i < temp; i++){
              if(fwrite(&zero,sizeof(char),1,out) != 1){
                perror("Fail to write .space\n");
                exit(1);
              }
            }
            data += temp;
          }
        }
      }
      //fill out extra space in file
      int fill = 0;
      fseek(out,data,SEEK_SET);
      for(int i = data; i < 512; i=i+4){
        if(fwrite(&fill,sizeof(int),1,out) != 1){
          perror("failed to finalize the data segment\n");
          exit(1);
        }
      }

      fseek(out,text,SEEK_SET);
      for(int i = text; i < 1024; i=i+4){
        if(fwrite(&fill,sizeof(int),1,out) != 1){
          perror("failed to finalize the text segment\n");
        }
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

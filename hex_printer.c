#include <stdio.h>

int main(int argc,char *argv[]){
  if(argc == 2){
    FILE *file;
    if((file = fopen(argv[1],"r")) == NULL){
      perror("Failed to open");
    }
    char byte;
    int count = 0;
    while(fread(&byte,1,1,file)){
      if(count % 4 == 0){
        printf("%08x: ",count);
      }
      printf("%02x", byte & 0xFF);
      count++;
      if(count % 4 == 0){
        printf("\n");
      }
    }
    fclose(file);
  }
  else{
    printf("include .out file in argument line.");
  }
}

#include <stdio.h>

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

//enter negative number in off to quit
int main(int argc,char *argv[]){
  int base;
  int embed;
  int off;

  sscanf(argv[1],"%d", &base);

 do{
    printf("embed:");
    scanf("%d", &embed);

    printf("offset:");
    scanf("%d", &off);

    base = embedder(base,embed,off);
    printf("%u\n", base);

    binary_checker(base);
  }while(off >= 0);
}

/**
  Operating Systems Phase 1
  manthan patil B2 rollno 20

  Creates a virtual machine with 400 bytes of memory divided into 100 words
  With a single 4 byte general purpose register  

  Assumes Instruction set has no errors ()
  Instruction set

  User executable- 
    LRxx
    SRxx
    BT
    CP
    H
    
  Kernal Only-
    GDxx
    PDxx
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void INIT();
void READ();
void WRITE();
void EUP();
void MOS();
void LOAD();
void START();
void TERMINATE(int);

int ALLOCATE();
int ADDRESSMAP(int VA);


// The virtual Machine
struct Machine {
  char mem[300][4];
  char IR[4];
  char C;
  int IC;
  char R[4];
  int SI;
  int TI;
  int PI;
} M;


// Process controll block P
struct PCB {
  int id;
  int TTL;
  int TLL;
  int TTC;
  int LLC;
  int PTR;
} P;


// IO cards
FILE* ProgramCard;
FILE* LinePrinter;

// 32 Bit unsigned number to keep track of free frames in memory
u_int32_t freebitmap = 0;


/**
  Helper functions
*/

// Copies from B to A 4 bytes
void assign(char* A, const char* B) {
  memcpy(A, B, 4);
}

// Get oprand two bytes form 
int num(char IR[4]) {
  return (IR[2]-'0')*10+(IR[3]-'0');
}

// Get oprand two bytes form 
int numb(char IR[4]) {
  return (IR[0]-'0')*1000+(IR[1]-'0')*100+(IR[2]-'0')*10+(IR[3]-'0');
}

void memdump() {
  // Debug dump the program section of memory
  for(int i=0; i<20; i++) {
    printf("%d\t", i+1);
    for(int j=0; j<4; j++) {
      printf("%c", M.mem[i][j]);
    }
    printf("\n");
  }
}

void mempeak(int n) {
  // Debug dump the program section of memory
  for(int i=0; i<20; i++) {
    printf("%d\t", i+1);
    for(int j=0; j<4; j++) {
      printf("%c", M.mem[i+n][j]);
    }
    printf("\n");
  }
}


/**
  Module definations
*/
// Loads input.txt
void LOAD() {
  char Buffer[45];

  int load_program =0;
  int rp =0;
  while(fgets(Buffer, 45, ProgramCard)) {
    if(Buffer[0] == '$') {
       if(Buffer[1] == 'A'){

         // INITIATE THE PCB
         INIT();
         P.id = (Buffer[5]-'0')*100 + (Buffer[6]-'0')*10 + (Buffer[7]-'0');
         P.TTL = (Buffer[9]-'0')*100 + (Buffer[10]-'0')*10 + (Buffer[11]-'0');
         P.TLL = (Buffer[13]-'0')*100 + (Buffer[14]-'0')*10 + (Buffer[15]-'0');


         // Allocate the Page table
         P.PTR = ALLOCATE();
         // for(int i=0; i<20; i++) {
         //   M.mem[P.PTR+i][0] = 1;
         //   M.mem[P.PTR+i][2] = '0' + i / 10;
         //   M.mem[P.PTR+i][3] = '0' + i%10;
         // }
         // printf("PTR IS %d\n", P.PTR);
         // mempeak(P.PTR);
         load_program=1;
       }
       if(Buffer[1] == 'D') {
         // printf("PTR IS %d\n", P.PTR);
         // mempeak(P.PTR);
         START();
         break;
       };
       if(Buffer[1] == 'E') LOAD();
       continue;
    }  

    if(load_program) {

      // Allocate a program page
      int add = ALLOCATE()/10;
      char* t= M.mem[P.PTR+rp/10];
      t[0] = 1;
      t[2] = '0' + add/10;
      t[3] = '0' + add%10;

      // Load the program
      char *r = Buffer;
      int l=0;
      int i=0;
      while(1) {
        char *m = M.mem[ADDRESSMAP(rp+i)];
        switch (*r) {
          case 'G' : l=4;break; // GD
          case 'P' : l=4;break; // PD
          case 'L' : l=4;break; // LR
          case 'S' : l=4;break; // SR
          case 'C' : l=4;break; // CP
          case 'B' : l=4;break; // BT
          case 'H' : l=1;break; // H
        }
        if(*r == 'H'){
          *m = 'H';
          break;
        }
        for(int i=0; i<l ;i++, r++) {
          *(m+i) = *r;
        }
        if(i >= 10) break;
        i++;
      }
      rp+= i;
    }
  }
}

void READ() {
  // Reads 40 bytes from DTA section of input.txt (Program Card)
  char Buffer[45];
  fgets(Buffer, 45, ProgramCard);
  // Cascade to allow $, $E, $EN in datacard
  if(Buffer[0] == '$') 
    if(Buffer[1] == 'E')
      if(Buffer[2] == 'N')
        if(Buffer[3] == 'D') {
          TERMINATE(1); return;
        }
  
  int mem = ADDRESSMAP(num(M.IR));
  if(mem == -1){ M.PI= 3; MOS(); return;}
  int c=0;
  while(Buffer[c] != '\n' && c<40) {
    M.mem[mem+c/4][c%4] = Buffer[c];
    c++;
  }
  EUP();
}

void WRITE() {
  // Writes 40 bytes to output.txt (Line printer)
  if(P.LLC >= P.TLL) { TERMINATE(2); return; }
  P.LLC++;
  int mem = ADDRESSMAP(num(M.IR));
  for(int i=0; i<10; i++) {
    for(int j=0; j<4; j++)
      fputc(M.mem[mem+i][j], LinePrinter);
  }
  fputc('\n', LinePrinter);
  EUP();
}

void clearint() {
  //RESET INTERRUPTS
  M.SI = M.PI = M.TI = 0;
}


// ALLOCATES A PAGE IN MEMORY returns real address
int ALLOCATE() {
  int t = rand() % 30;
  while(((freebitmap>>t)&1) == 1) t = rand() % 30;
  freebitmap|=(1<<t);
  return t*10;
}


// Maps Virtual Address to Real Address
int ADDRESSMAP(int va) {
  // return va;
  // Page fault
  if(M.mem[P.PTR + va/10][0] != 1) {
    M.PI = 3;
    return -1;
  }

  return num(M.mem[P.PTR+(va/10)])*10 + (va%10);
}


void MOS() {
  if(M.SI == 3) TERMINATE(0); 
  if(M.TI == 2) TERMINATE(3);
  if(M.SI == 1) READ();
  if(M.SI == 2) WRITE();
  if(M.PI == 2) TERMINATE(5);
  if(M.PI == 3) {
    // printf("PAGEFAULT\n");
    if((M.IR[0] == 'G' && M.IR[1] == 'D') || (M.IR[0] == 'S' && M.IR[1] == 'R')) {
      M.IC--;
      int nu = num(M.IR) /10;
      int pagenum = ALLOCATE()/10;
      M.mem[P.PTR+nu][0]=1;
      M.mem[P.PTR+nu][2]=pagenum/10 + '0';
      M.mem[P.PTR+nu][3]=pagenum%10 + '0';
      // mempeak(P.PTR);
      EUP();
      return;
    } 
    TERMINATE(6);
  }
}

void EUP() {
    clearint();
    // Fetch Intruction
    assign(M.IR, M.mem[ADDRESSMAP(M.IC)]);
    // Increment the Instruction counter
    M.IC++;

    if (M.IR[0] == 'H') M.SI = 3;
    // PreFetch operand address
    int raad = ADDRESSMAP(num(M.IR));
    if(M.PI) {
      MOS();
      return;
    }
    // SIMULAE
    P.TTC ++;
    if (P.TTC > P.TTL) M.TI = 2;

    if (M.IR[0] == 'G' && M.IR[1] == 'D') M.SI = 1;
    else if (M.IR[0] == 'P' && M.IR[1] == 'D') M.SI = 2;
    else if (M.IR[0] == 'L' && M.IR[1] == 'R') assign(M.R, M.mem[raad]);
    else if (M.IR[0] == 'S' && M.IR[1] == 'R') assign(M.mem[raad], M.R);
    else if (M.IR[0] == 'C' && M.IR[1] == 'R') M.C = memcmp(M.mem[raad], M.R, 4) == 0;
    else if (M.IR[0] == 'B' && M.IR[1] == 'T'){ if(M.C) { M.IC = num(M.IR); M.C = 0; } }
    else M.PI = 2;


    if(M.SI || M.TI || M.PI)
      MOS();
    else 
      EUP();
}

void INIT() {
  // Zero out the entire struct
  memset(&M, 0, sizeof(struct Machine));
  memset(&P, 0, sizeof(struct PCB));
}

void START() {
  // Sets IC to 0 and Starts program execution
  M.IC = 0;
  EUP();
}

void TERMINATE(int error_code) {

  // Log the Job ID
  fprintf(LinePrinter, "JOB ID\t:\t%d\n", P.id);

  if(M.SI==3)P.TTC++;
    
  // Log the error type
  switch(error_code) {
    case 0 : fputs(" NO ERROR\n", LinePrinter); break;
    case 1 : fputs(" OUT OF DATA\n", LinePrinter); break;
    case 2 : fputs(" LINE LIMIT EXCEEDED\n", LinePrinter); break;
    case 3 : fputs(" TIME LIMIT EXCEEDED\n", LinePrinter); break;
    case 4 : fputs(" OPERATION CODE ERROR\n", LinePrinter); break;
    case 5 : fputs(" OPERAND ERROR\n", LinePrinter); break;
    case 6 : fputs(" INVALID PAGE FAULT\n", LinePrinter); break;
  }
  fprintf(LinePrinter, "IC\t:\t%d\n", M.IC);
  fprintf(LinePrinter, "IR\t:\t%s\n", M.IR);
  fprintf(LinePrinter, "TTC\t:\t%d\n", P.TTC);
  fprintf(LinePrinter, "LLC\t:\t%d", P.LLC);
  
  // Writes two blank lines to output.txt (Line printer) and LOADS next program
  fputc('\n', LinePrinter);
  fputc('\n', LinePrinter);

  INIT();

  // Loads the next program if any
  LOAD();
}

int main() {
  ProgramCard = fopen("./input.txt", "r");
  LinePrinter = fopen("./output.txt", "w");

  if(ProgramCard == NULL || LinePrinter == NULL) {
    printf("ERROR OPENING input and output files, program exsiting");
    exit(0);
  }
  LOAD();
}

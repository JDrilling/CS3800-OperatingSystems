
#ifndef HW2_H
#define HW2_H

int MAX_WORDS = 512;

struct Program
{
  int pid;
  int numPages;
  int* pageTable;
  
  ~Program(){if(pageTable != NULL) delete [] pageTable;}
};

enum PageMethod{Demand, Pre};

bool isPowTwo(int number);
int fifo(list<Program> &processes, ifstream &finProgramTrace, PageMethod pageMethod, int pageSize);
int clock(list<Program> &processes, ifstream &finProgramTrace, PageMethod pageMethod, int pageSize);
int lru(list<Program> &processes, ifstream &finProgramTrace, PageMethod pageMethod, int pageSize);
void initialLoad(list<Program> &processes, int numPages, int* memory);

#endif
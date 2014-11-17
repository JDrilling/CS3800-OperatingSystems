
#ifndef HW2_H
#define HW2_H

unsigned int MAX_WORDS = 512;

struct Program
{
  unsigned int pid;
  unsigned int numPages;
  unsigned int* pageTable;
  
  ~Program(){if(pageTable != NULL) delete [] pageTable;}
};

enum PageMethod{Demand, Pre};

bool isPowTwo(int number);

void initialLoad(list<Program> &processes, unsigned int numPages, int* memory);

int runSimulation(list<Program> &processes, ifstream &finProgramTrace, PageMethod pageMethod, unsigned int pageSize, string replaceAlgo);


void clock(list<Program>::iterator process, bool* usedRecently, int* memory, unsigned int& pagePointer, unsigned int numPages, unsigned int pageNeed);
unsigned int lru(list<Program>::iterator process, unsigned int* lastUsed, int* memory, unsigned int numPages, unsigned int pageNeed, unsigned int ticks);
void fifo(list<Program>::iterator process, int* memory, unsigned int& pagePointer, unsigned int numPages, unsigned int pageNeed);

#endif
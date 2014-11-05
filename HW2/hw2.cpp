#include <iostream>
#include <cstdlib>
#include <fstream>
#include <list>
using namespace std;

#include "hw2.h"

int main(int argc, char* argv[])
{
  string programList, programTrace, replaceAlgo;
  int pageSize, pageFaults = -1;
  PageMethod pageMethod;
  ifstream finProgramList, finProgramTrace;
  
  list<Program> processes;
  
  /*-------------- Handle Error's first--------------*/
  try
  {
    if(argc != 6)
    {
      throw string("Error! Please use this format for the command\n"
                   "memTest <Program List File> <Program Trace File> <Page Size>\n"
                   "\t\t <Page Replacement Algorithm> <Paging method>\n"
                   "Page Replacement Algorithms: clock, lru, fifo\n"
                   "Paging Methods: 1(Prepaging), 0(Demand Paging)");
    }
    
    programList = argv[1];
    programTrace = argv[2];
    pageSize = atoi(argv[3]);
    replaceAlgo = argv[4];
    
    /*------------Check If files oopen-----------*/
    finProgramList.open(programList.c_str());
    if(!finProgramList.good())
      throw string("Error! Program List Failed to Load.");   
     
    finProgramTrace.open(programTrace.c_str());
    if(!finProgramTrace.good())
      throw string("Error! Program Trace Failed to Load.");
      
    /*----------Files are good, Check other params--------------*/  
      
    if(!isPowTwo(pageSize))
      throw string("Error! Page Size Must be a Power of Two.");
      
    if(replaceAlgo != "clock" && replaceAlgo != "lru" && replaceAlgo != "fifo")
      throw string("Error! Invalid Replacement Algorithm. Must be \"clock\" , \"lru\" or \"fifo\"");
      
    if(atoi(argv[5]) == 1)
      pageMethod = Pre;
    else if(atoi(argv[5]) == 0)
      pageMethod = Demand;
    else
      throw string("Error! Invalid Paging Method. Must be \"1\" for Prepaging or \"0\" for Demand Paging");
      
    /*------Params are good, Errors should have been thrown if bad--------*/

    /*---------Read in processes -- Error handling Over-----------*/
    
    int pid, memory, pageNum = 0;
    Program myProgram;
    while(finProgramList >> pid)
    {
      finProgramList >> memory;
      myProgram.pid = pid;
      myProgram.numPages = (memory - 1)/pageSize + 1;
      myProgram.pageTable = new int[myProgram.numPages];
      
      /*------------------------------------------*/
      for(int i = 0; i < myProgram.numPages; i++, pageNum++)
        myProgram.pageTable[i] = pageNum;
        
      processes.push_back(myProgram);
    }
    //When push_back copies myProgram, it copies values
    //Therefore, it copies the value of the pointer.
    myProgram.pageTable = NULL; //Must be NULL to prevent Double Free.

    /*-------------- Processes Read -------------*/
    
    /*-- Debug Message to see that processes are read correctly --
    for (list<Program>::iterator it = processes.begin(); it != processes.end(); it++)
      std::cout << it->pid << ' ' << it->numPages << ' ' << it->pageTable[0] << ' ' << it->pageTable[it->numPages - 1] << endl;
    ----------------------------------------------*/
      
    /*----------- Run algorithm Requested --------*/
      
    if(replaceAlgo == "clock")
      pageFaults = clock(processes, finProgramTrace, pageMethod, pageSize);
    else if(replaceAlgo == "lru")
      pageFaults = lru(processes, finProgramTrace, pageMethod, pageSize);
    else if(replaceAlgo == "fifo")
      pageFaults = fifo(processes, finProgramTrace, pageMethod, pageSize);
      
    /*----------------Final Output----------------*/
    cout << "Replacement Algorithm: " << replaceAlgo << endl;
    cout << "Paging Algorithm: " << (pageMethod == Pre ? "Pre-Paging" : "Demand Paging") << endl;
    cout << "Page Size: " << pageSize << endl;
    cout << "Page Faults: " << pageFaults << endl;
    /*--------------------------------------------*/
    
  } catch(string err)
  {
    cout << err << endl << endl;
    cout << "Now exiting..." << endl;
  }
  return 0;
}

bool isPowTwo(int number)
{
  int count = 0;
  
  while(number && count <= 1)
  {
    if(number & 1)
      count++;
    number >>= 1;
  }
  
  return count == 1;
}

void initialLoad(list<Program> &processes, int numPages, int* memory)
{
  unsigned short currentPage = 0;
  
  /*----------Initial Load------------*/
  list<Program>::iterator processLoading = processes.begin();
  int processPages = 0;
  while(currentPage < numPages && processLoading != processes.end())
  {
    if(processPages == processLoading->numPages)
    {
      processLoading++;
      processPages = 0;
    }
    
    memory[currentPage] = processLoading->pageTable[processPages];
    
    processPages++;
    currentPage++;
  }
  currentPage %= numPages;
  
  /*---------- Debug Messages --------------
  cout << endl << endl;
  
  cout << "currentPage" << endl;
  for(currentPage; currentPage < numPages; currentPage++)
    cout << memory[currentPage] << endl;
    
  cout << endl << currentPage << endl;
  
  currentPage %= numPages;
  -----------------------------------------*/
  return;
}

int fifo(list<Program> &processes, ifstream &finProgramTrace, PageMethod pageMethod, int pageSize)
{
  unsigned short numPages = MAX_WORDS/pageSize;
  int *memory = new int[numPages];
  int pid, wordNeed, pageNeed, pageFaults = 0, topPage = 0;
  bool inMemory;
  
  initialLoad(processes, numPages, memory);
  /*---- Contiue with Algorithm, Load Complete --------*/
  
  while(finProgramTrace >> pid)
  {
    finProgramTrace >> wordNeed;
    pageNeed = wordNeed/pageSize;
    inMemory = false;
    
    list<Program>::iterator process = processes.begin();
    
    while(pid != process->pid && process != processes.end())
      process++;
    
    /*-------------- Find it in memory ----------------*/
    int pageLoc = -1;
    while(!inMemory && pageLoc < numPages)
    {
      pageLoc++;
      inMemory = (process->pageTable[pageNeed] == memory[pageLoc]);
    }
    
    //*---- if not in Memory, its a page fault----------- */
    if(!inMemory)
    {
      pageFaults ++;
      memory[topPage] = process->pageTable[pageNeed];
      topPage ++;
      topPage %= numPages;
      
      if(pageMethod == Pre && process->numPages > 1 && numPages > 1)
      {
        //Mod so that it wraps around
        memory[topPage] = process->pageTable[pageNeed % process->numPages];
          
        topPage++;
        topPage %= numPages;
      }
    }
    
    /* ---------- Massive Debug Info -----------------
    cout << "PageNeeded: " << pageNeed << " Found? " << (inMemory?"True":"False");
    cout << (inMemory?" Found":" Replaced") << " At: " << (inMemory?pageLoc+1:topPage);
    cout << " Page Faults: " << pageFaults << endl;
    cout << "--------------------------------------------------" << endl;
    /*-------------------------------------------------*/
  }
  /*------------ Delete Memory -----------*/
  delete [] memory;
  return pageFaults;
}

int clock(list<Program> &processes, ifstream &finProgramTrace, PageMethod pageMethod, int pageSize)
{
  unsigned short numPages = MAX_WORDS/pageSize;
  int *memory = new int[numPages];
  bool *usedRecently = new bool[numPages];
  int pid, wordNeed, pageNeed, pageFaults = 0, currentPage = 0;
  bool inMemory;
  
  initialLoad(processes, numPages, memory);
  for(int i = 0; i < numPages; i++)
    usedRecently[i] = true;
  /*---- Contiue with Algorithm, Load Complete --------*/
  
  while(finProgramTrace >> pid)
  {
    finProgramTrace >> wordNeed;
    pageNeed = wordNeed/pageSize;
    inMemory = false;
    
    list<Program>::iterator process = processes.begin();
    
    while(pid != process->pid && process != processes.end())
      process++;
    
    int pageLoc = -1;
    while(!inMemory && pageLoc < numPages)
    {
      pageLoc++;
      inMemory = (process->pageTable[pageNeed] == memory[pageLoc]);
    }
    
    //*---- if not in Memory, its a page fault----------- */
    if(!inMemory)
    {
      pageFaults ++;
      //While we're finding pages that have been used recently
      while(usedRecently[currentPage])
      {
        usedRecently[currentPage] = false;
        currentPage++;
        currentPage %= numPages;
      }
      
      //Found one that hasn't been used recently
      memory[currentPage] = process->pageTable[pageNeed];
      usedRecently[currentPage] = true;
      
      currentPage++;
      currentPage %= numPages;
      
      if(pageMethod == Pre && process->numPages > 1 && numPages > 1)
      {
        memory[currentPage] = process->pageTable[(pageNeed + 1) % process->numPages];
        usedRecently[currentPage] = true;
        currentPage++;
        currentPage %= numPages;
      }
    }
    else //If it is in memory, set usedRecently to true;
      usedRecently[pageLoc] = true;
  } 
  
  /*------------ Delete Memory -----------*/
  delete [] usedRecently;
  delete [] memory;
  return pageFaults;
}

int lru(list<Program> &processes, ifstream &finProgramTrace, PageMethod pageMethod, int pageSize)
{
  unsigned short numPages = MAX_WORDS/pageSize;
  int *memory = new int[numPages];
  int *lastUsed = new int[numPages];
  int pid, wordNeed, pageNeed, pageFaults = 0;
  int oldest, oldestIndex, ticks = 1;
  bool inMemory;
  
  initialLoad(processes, numPages, memory);
  for(int i = 0; i < numPages; i++)
    lastUsed[i] = 0;
  /*---- Contiue with Algorithm, Load Complete --------*/
  
  while(finProgramTrace >> pid)
  {
    finProgramTrace >> wordNeed;
    pageNeed = wordNeed/pageSize;
    inMemory = false;
    
    list<Program>::iterator process = processes.begin();
    
    while(pid != process->pid && process != processes.end())
      process++;
    /*---------- Have Correct Process -----------*/
    
    int pageLoc = -1;
    while(!inMemory && pageLoc < numPages)
    {
      pageLoc++;
      inMemory = (process->pageTable[pageNeed] == memory[pageLoc]);
    }
    
    //*---- if not in Memory, its a page fault----------- */
    if(!inMemory)
    {
      pageFaults ++;
      oldest = lastUsed[0];
      oldestIndex = 0;
      
      for(int i = 1; i < numPages; i++)
        if(lastUsed[i] < oldest)
        {
          oldest = lastUsed[i];
          oldestIndex = i;
        }
      
      memory[oldestIndex] = process->pageTable[pageNeed];
      lastUsed[oldestIndex] = ticks;
      
      if(pageMethod == Pre && process->numPages > 1 && numPages > 1)
      {
        memory[(oldestIndex+1)%numPages] = process->pageTable[(pageNeed + 1) % process->numPages];
        lastUsed[(oldestIndex+1)%numPages] = ticks;
      }
    }
    else
      lastUsed[pageLoc] = ticks;
    
    /* ---------- Massive Debug Info -----------------
    cout << "PageNeeded: " << pageNeed << " Found? " << (inMemory?"True":"False");
    cout << (inMemory?" Found":" Replaced") << " At: " << (inMemory?pageLoc:oldestIndex) << endl;
    cout << "--------------------------------------------------" << endl;
    /*-------------------------------------------------*/
    
    
    ticks++;
  } 
  
  /*------------ Delete Memory -----------*/
  delete [] lastUsed;
  delete [] memory;
  return pageFaults;
}
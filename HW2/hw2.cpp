/*Created by Jacob Drilling
  Homework #2 for CS3800-Operating Systems
  Description: This Program simulates paging algorithms in Operating systems.
*/

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <list>
using namespace std;

#include "hw2.h"

int main(int argc, char* argv[])
{
  string programList, programTrace, replaceAlgo;
  unsigned int pageSize, pageFaults = 0;
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
      
    if(pageSize < 0 || pageSize > MAX_WORDS)
      throw string("Error! Page size must be between 0 and MAX_WORDS");
      
    if(replaceAlgo != "clock" && replaceAlgo != "lru" && replaceAlgo != "fifo")
      throw string("Error! Invalid Replacement Algorithm. Must be \"clock\" , \"lru\" or \"fifo\"");
      
    if(atoi(argv[5]) == 1)
      pageMethod = Pre;
    else if(atoi(argv[5]) == 0)
      pageMethod = Demand;
    else
      throw string("Error! Invalid Paging Method. Must be \"1\" for Prepaging or \"0\" for Demand Paging");
      } catch(string err)
  {
    cout << err << endl << endl;
    cout << "Now exiting..." << endl;
    exit (EXIT_FAILURE);
  }
  /*------Params are good, Errors should have been thrown if bad--------*/
  
  /*---------Read in processes -- Error handling Over-----------*/
  
  int pid, memory, pageNum = 0;
  Program myProgram;
  while(finProgramList >> pid)
  {
    finProgramList >> memory;
    myProgram.pid = pid;
    myProgram.numPages = (memory - 1)/pageSize + 1;
    myProgram.pageTable = new unsigned int[myProgram.numPages];
    
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
  /*----------------------------------------------*/
    
  /*----------- Run algorithm Requested --------*/
  pageFaults = runSimulation(processes, finProgramTrace, pageMethod, pageSize, replaceAlgo);
    
  /*----------------Final Output----------------*/
  cout << "Replacement Algorithm: " << replaceAlgo << endl;
  cout << "Paging Algorithm: " << (pageMethod == Pre ? "Pre-Paging" : "Demand Paging") << endl;
  cout << "Page Size: " << pageSize << endl;
  cout << "Page Faults: " << pageFaults << endl;
  /*--------------------------------------------*/
  
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

void initialLoad(list<Program> &processes, unsigned int numPages, int* memory)
{
  unsigned int currentPage = 0;
  unsigned int numProcesses= processes.size();
  unsigned int pagesPerProcess = numPages/numProcesses;
  unsigned int extraPages = numPages % numProcesses;
  list<Program>::iterator processLoading = processes.begin();
  
  while(processLoading != processes.end())
  {
    for(unsigned int i = 0; i < pagesPerProcess + (extraPages > 0?1:0); i++)
    {
      if(i < processLoading->numPages)
        memory[currentPage] = processLoading->pageTable[i];
      else
        memory[currentPage] = -1;
        
      currentPage ++;
    }
    processLoading ++;
    if(extraPages > 0)
      extraPages--;
  }
  return;
}

int runSimulation(list<Program> &processes, ifstream &finProgramTrace, PageMethod pageMethod, unsigned int pageSize, string replaceAlgo)
{
  
  unsigned int numPages = MAX_WORDS/pageSize;
  
  int *memory = new int[numPages];
  unsigned int *lastUsed = new unsigned int[numPages];
  bool *usedRecently = new bool[numPages];
  
  unsigned int pid, wordNeed, pageNeed, pageFaults = 0, pagePointer = 0;
  unsigned long ticks = 1;
  unsigned int oldestIndex;
  bool inMemory;
  unsigned int pageLoc = 0;
  
  for(unsigned int i = 0; i < numPages; i++)
  {
    lastUsed[i] = 0;
    usedRecently[i] = true;
  }

  initialLoad(processes, numPages, memory);
  
  while(finProgramTrace >> pid)
  {
    finProgramTrace >> wordNeed;
    pageNeed = wordNeed/pageSize;
    inMemory = false;
    pageLoc = 0;
    
    list<Program>::iterator process = processes.begin();
    
    while(pid != process->pid && process != processes.end())
      process++;
    
    /*-------------- Find it in memory ----------------*/
    
    do
    {
      inMemory = (process->pageTable[pageNeed] == memory[pageLoc]);
      if(!inMemory)
        pageLoc++;
    }while(!inMemory && pageLoc < numPages);

    if(!inMemory && !(pageLoc < numPages))
    {
      pageFaults ++;
      /*-------------------- CLOCK ---------------------*/
      if(replaceAlgo == "clock")
      {
        clock(process, usedRecently, memory, pagePointer, numPages, pageNeed);
        
        if(pageMethod == Pre && process->numPages > 1 && numPages > 1)
        {
          inMemory = false;
          unsigned int prePageLoc = 0;
          do
          {
            inMemory = (process->pageTable[(pageNeed + 1) % process->numPages] == memory[prePageLoc]);
            if(!inMemory)
              prePageLoc++;
          }while(!inMemory && prePageLoc < numPages);
        
          if(!inMemory)
          {
            memory[pagePointer] = process->pageTable[(pageNeed + 1) % process->numPages];
            usedRecently[pagePointer] = true;
            pagePointer++;
            pagePointer %= numPages;
          }
          
        }
        
      }
      /*------------------------ LRU -------------------------- */
      else if(replaceAlgo == "lru")
      {
        oldestIndex = lru(process, lastUsed, memory, numPages, pageNeed, ticks);
          
        if(pageMethod == Pre && process->numPages > 1 && numPages > 1)
        {
          inMemory = false;
          unsigned int prePageLoc = 0;
          do
          {
            inMemory = (process->pageTable[(pageNeed + 1) % process->numPages] == memory[prePageLoc]);
            if(!inMemory)
              prePageLoc++;
          }while(!inMemory && prePageLoc < numPages);
        
          if(!inMemory)
          {
            memory[(oldestIndex+1)%numPages] = process->pageTable[(pageNeed + 1) % process->numPages];
            lastUsed[(oldestIndex+1)%numPages] = ticks;
          }
        }
      }
      /*------------------------ FIFO -------------------------- */
      else if(replaceAlgo == "fifo")
      {
        fifo(process, memory, pagePointer, numPages, pageNeed);
      
        if(pageMethod == Pre && process->numPages > 1 && numPages > 1)
        {
          inMemory = false;
          unsigned int prePageLoc = 0;
          do
          {
            inMemory = (process->pageTable[(pageNeed + 1) % process->numPages] == memory[prePageLoc]);
            if(!inMemory)
              prePageLoc++;
          }while(!inMemory && prePageLoc < numPages);
        
          if(!inMemory)
          {
            //Mod so that it wraps around
            memory[pagePointer] = process->pageTable[(pageNeed + 1) % process->numPages];
            pagePointer++;
            pagePointer %= numPages;
          }
        }
      }
      
    }
    else
    {
      usedRecently[pageLoc] = true;
      lastUsed[pageLoc] = ticks;
    }
    
    ticks++;
  }
  
  delete [] usedRecently;
  delete [] lastUsed;
  delete [] memory;
  return pageFaults;
}



void clock(list<Program>::iterator process, bool* usedRecently, int* memory, unsigned int& pagePointer, unsigned int numPages, unsigned int pageNeed)
{
  //While we're finding pages that have been used recently
  while(usedRecently[pagePointer] && memory[pagePointer] != -1)
  {
    usedRecently[pagePointer] = false;
    pagePointer++;
    pagePointer %= numPages;
  }
  
  //Found one that hasn't been used recently
  memory[pagePointer] = process->pageTable[pageNeed];
  usedRecently[pagePointer] = true;
  
  pagePointer++;
  pagePointer %= numPages;
  
  return;
}

unsigned int lru(list<Program>::iterator process, unsigned int* lastUsed, int* memory, unsigned int numPages, unsigned int pageNeed, unsigned int ticks)
{
  unsigned int oldest = lastUsed[0];
  unsigned int oldestIndex = 0;

  for(unsigned int i = 1; i < numPages; i++)
    if(lastUsed[i] < oldest)
    {
      oldest = lastUsed[i];
      oldestIndex = i;
    }

  memory[oldestIndex] = process->pageTable[pageNeed];
  lastUsed[oldestIndex] = ticks;
  
  return oldestIndex;
}

void fifo(list<Program>::iterator process, int* memory, unsigned int& pagePointer, unsigned int numPages, unsigned int pageNeed)
{
  memory[pagePointer] = process->pageTable[pageNeed];
  pagePointer ++;
  pagePointer %= numPages;
  
  return;
}

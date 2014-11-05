filename="pageSimulation.out"
compileCommand="g++ *.cpp -O2 -o $filename"
echo "Compiling... $compileCommand"
$compileCommand
echo "This test runs every algorithm (clock,LRU,FIFO)"
echo "For every algorithm it will run Demand paging then PrePaging"
echo "For both Paging methods it will run page sizes 1,2,4,8,16"
echo "This is a total of 30 runs (3*2*5)"
echo ""

pagingMethods="clock lru fifo"
pageSize=1

for algorithm in $pagingMethods; do
  pageSize=1
  while [ $pageSize -lt 17 ]; do
    pagingMethod=0
    while [ $pagingMethod -lt 2 ]; do
      ./$filename programlist.txt programtrace.txt $pageSize $algorithm $pagingMethod
      echo ""
      
      pagingMethod=$((pagingMethod + 1))
    done
    pageSize=$((pageSize * 2))
  done
done


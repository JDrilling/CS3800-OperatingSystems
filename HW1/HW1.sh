#!/bin/bash 
##Jacob Drilling 
##CS3800 
##Assignment 1 - Section A. 

option=0

echo Welcome!

while [ $option -ne 4 ] 
do 
  echo Please select a menu option. 
  echo ---------------------------- 
  echo 1. Ancestry History. 
  echo 2. Who is Online. 
  echo 3. What process any user is running. 
  echo 4. Quit.

  read option 


  case $option in
    1) ps -ef > proc.tmp
       currentProc=$$

        echo
        echo

       while [ $currentProc -ne "1" ]
       do
         echo $currentProc
         echo " |"
 
         currentProc=$(awk -v currentProc="$currentProc" '{if($2 == currentProc)print $3;}' proc.tmp)
       done

       echo $currentProc
       ;;
    2) echo
       echo
       who | uniq | awk '{print $1}'
       ;;
    3) count=1 
       userID=0
       validUser=false
       if [ -e users.tmp ] 
       then
         rm users.tmp
       fi


       echo
       echo
       echo Select a User by Number.
       echo "-----------------------"

       who | uniq |while read user 
       do
         user=$(echo $user | awk '{print $1}')

         echo $count") "$user >> users.tmp
         echo $count") "$user

         count=$(expr $count + 1)
       done

       read userID

       userID=$userID\)
	   

       user=$(awk -v userID="$userID" '{if($1 == userID)print $2;}' users.tmp)

       echo
       echo
       if [ -e proc.tmp ]
       then
         rm proc.tmp
       fi
       
       ps -ef > proc.tmp
      
       grep $user proc.tmp

     
      ;;
    4) echo Goodbye! ;;
    *) echo Please select a valid option. ;;
  esac

  echo
  echo
   
done

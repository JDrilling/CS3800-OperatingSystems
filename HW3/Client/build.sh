if [ -e Client ]
then
  rm a.out
fi

echo "Compiling... gcc *.c -lpthread -o Client"
gcc *.c -lpthread -o Client

if [ -e Client ]
then
  echo "Comilation successfull, Ignore the Nasty errors."
  echo "Run with \"Client <Hostname>\"."
fi

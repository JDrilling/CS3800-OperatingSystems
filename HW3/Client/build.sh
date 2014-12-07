if [ -e Client.o ]
then
  rm Client.o
fi

echo "Compiling... gcc *.c -lpthread -o Client"
gcc *.c -lpthread -o Client.o

if [ -e Client.o ]
then
  echo "Comilation successfull, Ignore the Nasty errors."
  echo "Run with \"Client <Hostname>\"."
fi

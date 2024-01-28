#!/bin/bash
# CHANGE THE SHEBANG (ABOVE) TO YOUR BASH LOCATION!
rm -f -- a.out
gcc compiler.c 
if [ $? -ne 0 ]; then 
	echo -e "\nCompile of compiler.c failed"
	echo "Good bye!"
	exit 1
fi
EXE="./a.out"


echo -e "\nCompile of compiler.c succeded.\n"

for NUM in {1..67}
do
   echo "-> Case #$NUM - in$NUM.txt == base$NUM.txt"
	eval $EXE in$NUM.txt > out$NUM.txt
	diff -w out$NUM.txt base$NUM.txt
done

echo -e "\n"

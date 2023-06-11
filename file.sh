#!/bin/bash
# Example to check if file is empty or not
# test command with -s flag to check if file
# is empty or not
# creating new file with the given name
filename="I_LOVE_GEEKSFORGEEKS.txt"

# touch command is used to create empty file
touch $filename

# checking if file is empty or not
if test -s $filename
then
   echo "File is not empty"
else
   echo "File is empty"
fi

# adding to the newly created text file 
echo "I love GeeksforGeeks" >> $filename

# checking again if file is empty or not
if test -s $filename
then
   echo "File is not empty now"
else
   echo "File is empty"
  
fi
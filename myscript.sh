#!/bin/bash







function listCommand()
{
while read FILE_PATH; do
    if [[ ! -f "$FILE_PATH" ]]
    then
      echo "$FILE_PATH" | awk -F 'OBHIC-' 'BEGIN { ORS="" }; { print "find " $1 "OBHIC-"; split($2,b,"/"); print b[1]; print " -name "; split($3,a,"/"); print a[2] "\n" }'
    fi
  done < <(cat $1 | awk '{ print $2 }' | grep -v -e "----------")
}


while read COMMAND; do
  echo $COMMAND
done < <(listCommand $1) 






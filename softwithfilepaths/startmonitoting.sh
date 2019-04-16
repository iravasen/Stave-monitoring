#!/bin/bash
echo "Compiling the software for the database"
make clean
make

echo "Refreshing the kerberos ticket"
kinit aliceitstorino
eosfusebind

echo "Getting information from the database"
./GetFromDatabase

echo "Running the analysis of the data"
root -l -b <<EOF
.L StartMonitoring.C++
StartMonitoring($1, $2, "$3")
.q
EOF


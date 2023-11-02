#!/bin/bash

echo "Counting lines"
find . -name \*.[c\|h] -not -name "limine.h" -not -path "./src/lai/*" -not -path "./src/flanterm/*" -print | xargs wc -l | tail -n 1

echo "Counting words"
find . -name \*.[c\|h] -not -name "limine.h" -not -path "./src/lai/*" -not -path "./src/flanterm/*" -print | xargs wc -w | tail -n 1

echo "Counting characters"
find . -name \*.[c\|h] -not -name "limine.h" -not -path "./src/lai/*" -not -path "./src/flanterm/*" -print | xargs wc -c | tail -n 1


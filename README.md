# OS course porject phase 1
A simple batch operating system inside a virtual machine with a simple instruction set

GDxx \
PDxx \
LR \
SR \
CP \
BT 

virtual machine consists of 400 bytes of ram split into 100 words (ram is a word (4byte) addresible) 
the machine consists of a single 4 byte general purpose register R


# Running
assuming git is installed
```
git clone http://github.com/manthanabc/OSCP1
cd OSCP1
./a.out
```
write your batch jobs after $AMI section in input.txt
and the output will be stored in output.txt

# Building
requres some c compiler
```
git clone http://github.com/manthanabc/OSCP1
cd OSCP1
cc osp1.c
```

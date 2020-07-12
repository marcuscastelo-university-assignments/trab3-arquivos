INC = $(foreach i,$(shell find ./headers -type d),$(shell echo "-I $i"))
SRC = ./src
COMP = gcc
FLAGS = -Wall -g

SRC_RULES = binary header registry utils csv b_tree

all: $(SRC_RULES)
	@ $(COMP) *.o $(SRC)/main.c -o prog $(INC) $(FLAGS) && \
	echo 'Compiled Successfully' || \
	echo 'There were compilation errors'
	@ rm *.o

run:
	./prog

$(SRC_RULES):
	@ $(COMP) -c $(SRC)/$@/*.c $(INC) $(FLAGS)

zip:
	@ rm trab3.zip 2>/dev/null || cat < /dev/null
	@ zip -r trab3.zip src headers Makefile

deb: all
	./prog < 1.in &> 1.out
	code 1.out

deb_hex: all
	./prog < 1.in &> 1.out
	hexdump -vC 1.out > 1.hexdump
	code 1.hexdump

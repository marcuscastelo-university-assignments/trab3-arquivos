INC = $(foreach i,$(shell find ./headers -type d),$(shell echo "-I $i"))
SRC = ./src
COMP = gcc
FLAGS = -Wall -g

SRC_RULES = binary header registry data_manager utils csv b_tree

all: $(SRC_RULES)
	@ $(COMP) *.o $(SRC)/main.c -o prog $(INC) $(FLAGS) && \
	echo 'Compiled Successfully' || \
	echo 'There were compilation errors'

run:
	./prog

$(SRC_RULES):
	@ $(COMP) -c $(SRC)/$@/*.c $(INC) $(FLAGS)

zip:
	@ zip -r trab2.zip src headers Makefile

deb: all
	./prog < 1.in > 1.out
	code 1.out

deb_hex:
	./prog < 1.in > 1.out
	hd -vC 1.out > 1.hexdump
	code 1.hexdump

CC = gcc
OBJS = amount_set.o matamazom.o matamazom_print.o matamazom_tests.o matamazom_main.o
EXEC = matamazom
DEBUG_FLAG = # now empty, assign -g for debug
COMP_FLAG = -std=c99 -Wall -Werror -pedantic-errors -DNDEBUG

$(EXEC) : $(OBJS)
	$(CC) $(COMP_FLAG) $(DEBUG_FLAG) $(OBJS) -o $@ -L. -lmtm

amount_set.o : amount_set.c amount_set.h
	$(CC) $(COMP_FLAG) -c $(DEBUG_FLAG) amount_set.c

matamazom.o : matamazom.c matamazom.h amount_set.h set.h matamazom_print.h
	$(CC) $(COMP_FLAG) -c  $(DEBUG_FLAG) matamazom.c

matamazom_print.o : matamazom_print.c matamazom_print.h
	$(CC) $(COMP_FLAG) -c $(DEBUG_FLAG) matamazom_print.c

matamazom_tests.o : tests/matamazom_tests.c tests/matamazom_tests.h matamazom.h tests/test_utilities.h
	$(CC) $(COMP_FLAG) -c $(DEBUG_FLAG) tests/matamazom_tests.c

matamazom_main.o : tests/matamazom_main.c matamazom.h tests/matamazom_tests.h
	$(CC) $(COMP_FLAG) -c $(DEBUG_FLAG) tests/matamazom_main.c


amount_set :
	$(CC) $(COMP_FLAG) $(DEBUG_FLAG) -o amount_set amount_set*.c tests/amount_set*.c tests/test_utilities.h

clean:
	rm -f $(OBJS)

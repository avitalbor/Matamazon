CC = gcc
OBJS = amount_set.o amount_set_main.o amount_set_tests.o
EXEC = AmountSet
DEBUG_FLAG = # now empty, assign -g for debug
CFLAGS = -std=c99 -Wall -Werror

$(EXEC) : $(OBJS)
	$(CC) $(DEBUG_FLAG) $(OBJS) -o $@

amount_set.o : amount_set.c amount_set.h
amount_set_main.o : amount_set_main.c amount_set_tests.h test_utilities.h
amount_set_tests.o : amount_set_tests.c amount_set_tests.h amount_set.h test_utilities.h

clean:
	rm -f $(OBJS) $(EXEC)
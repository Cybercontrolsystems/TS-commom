# Fairly generic cross-compilation makefile for simple programs
CC=$(CROSSTOOL)/$(ARM)/bin/gcc
NAME=common
TARGET=$(NAME)
all: $(TARGET)
OBJS=$(NAME).o 

$(NAME).o: $(NAME).c common.h

clean:
	rm -f $(NAME) $(OBJS)

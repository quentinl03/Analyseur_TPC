# Makefile TP Flex

# $@ : the current target
# $^ : the current prerequisites
# $< : the first current prerequisite

CC=gcc
CFLAGS=-Wall
LDFLAGS=-Wall -lfl
EXEC=tpc

# Filenames
PARSER=tpc
LEXER=tpc

FLEX_FLAGS=
BISON_FLAGS=-Wcounterexamples

OBJS_DIR=obj
BIN_DIR=bin
SRC_DIR=src

OBJS=$(wildcard $(OBJS_DIR)/*.tab.* $(OBJS_DIR)/*.yy.* $(OBJS_DIR)/*.o)
# $(info $(OBJS))

all: $(BIN_DIR)/$(EXEC)

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)/$(LEXER).yy.c: $(SRC_DIR)/$(LEXER).lex $(OBJS_DIR)/$(PARSER).tab.h
	flex $(FLEX_FLAGS) --outfile=$@ $<

$(OBJS_DIR)/$(PARSER).tab.h $(OBJS_DIR)/$(PARSER).tab.c &: $(SRC_DIR)/$(PARSER).y
	bison $(BISON_FLAGS) -d $< -o $(OBJS_DIR)/$(PARSER).tab.c

$(BIN_DIR)/$(EXEC): $(OBJS_DIR)/$(LEXER).yy.o $(OBJS_DIR)/$(PARSER).tab.o
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean distclean

distclean:
	rm -f $(OBJS)

clean: distclean
	rm -f $(BIN_DIR)/$(EXEC)

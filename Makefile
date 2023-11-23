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

SRC_DIR=src
OBJS_DIR=obj
BIN_DIR=bin
OUT_DIRS=$(OBJS_DIR) $(BIN_DIR)

MODULES=$(patsubst %.c, $(OBJS_DIR)/%.o, tree.c)
OBJS=$(wildcard $(OBJS_DIR)/*.tab.* $(OBJS_DIR)/*.yy.* $(OBJS_DIR)/*.o)
# $(info $(OBJS))

all: $(BIN_DIR)/$(EXEC)

obj/tree.o: src/tree.c src/tree.h
obj/$(PARSER).o: obj/$(PARSER).c src/tree.h

# https://www.gnu.org/software/make/manual/html_node/Prerequisite-Types.html
# Evite de créer les dossiers à chaque fois, et n'impacte pas le message "Up to date"
$(OUT_DIRS):
	@mkdir -p $@

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c | $(OUT_DIRS)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)/$(LEXER).yy.c: $(SRC_DIR)/$(LEXER).lex $(OBJS_DIR)/$(PARSER).tab.h | $(OUT_DIRS)
	flex $(FLEX_FLAGS) --outfile=$@ $<

$(OBJS_DIR)/$(PARSER).tab.h $(OBJS_DIR)/$(PARSER).tab.c &: $(SRC_DIR)/$(PARSER).y $(MODULES) | $(OUT_DIRS)
	bison $(BISON_FLAGS) -d $< --output=$(OBJS_DIR)/$(PARSER).tab.c

$(BIN_DIR)/$(EXEC): $(OBJS_DIR)/$(LEXER).yy.o $(OBJS_DIR)/$(PARSER).tab.o $(MODULES) | $(OUT_DIRS)
	$(CC) $^ -o $@ $(LDFLAGS)

.PHONY: clean distclean dir

distclean:
	rm -f $(OBJS)

clean: distclean
	rm -f $(BIN_DIR)/$(EXEC)

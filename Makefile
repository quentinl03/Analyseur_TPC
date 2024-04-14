# Makefile TP Flex

# $@ : the current target
# $^ : the current prerequisites
# $< : the first current prerequisite

CC=gcc
CFLAGS=-Wall -g3 -O0
LDFLAGS=-Wall -lfl -Werror -Wfatal-errors
EXEC=tpcc

# Filenames
PARSER=tpc
LEXER=tpc

FLEX_FLAGS=
BISON_FLAGS=

SRC_DIR=src
OBJ_DIR=obj
BIN_DIR=bin
TESTS_DIR=test
REPORT_DIR=rep
OUT_DIRS=$(OBJ_DIR) $(BIN_DIR)

MODULES=$(patsubst %.c, $(OBJ_DIR)/%.o, tree.c parser.c main.c symbol.c symboltable.c arraylist.c registers.c treeReader.c codeWriter.c error.c)
OBJS=$(wildcard $(OBJ_DIR)/*.tab.* $(OBJ_DIR)/*.yy.* $(OBJ_DIR)/*.o)

TAR_CONTENT=$(SRC_DIR)/ $(TESTS_DIR)/ $(REPORT_DIR)/ $(OBJ_DIR)/ $(BIN_DIR) Makefile README.md
TAR_NAME=ProjetCompilationL3_LABORDE_SEBAN

# $(info $(OBJS))

all: $(BIN_DIR)/$(EXEC)

obj/tree.o: src/tree.c src/tree.h
obj/parser.o: src/parser.c src/parser.h
obj/$(PARSER).o: obj/$(PARSER).c src/tree.h

# https://www.gnu.org/software/make/manual/html_node/Prerequisite-Types.html
# Evite de créer les dossiers à chaque fois, et n'impacte pas le message "Up to date"
$(OUT_DIRS):
	@mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OUT_DIRS)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/$(LEXER).yy.c: $(SRC_DIR)/$(LEXER).lex $(OBJ_DIR)/$(PARSER).tab.h | $(OUT_DIRS)
	flex $(FLEX_FLAGS) --outfile=$@ $<

$(OBJ_DIR)/$(PARSER).tab.h $(OBJ_DIR)/$(PARSER).tab.c &: $(SRC_DIR)/$(PARSER).y $(MODULES) | $(OUT_DIRS)
	bison $(BISON_FLAGS) -d $< --output=$(OBJ_DIR)/$(PARSER).tab.c

$(BIN_DIR)/$(EXEC): $(OBJ_DIR)/$(LEXER).yy.o $(OBJ_DIR)/$(PARSER).tab.o $(MODULES) | $(OUT_DIRS)
	$(CC) $^ -o $@ $(LDFLAGS)

# TODO : Faire un sous dossier pour les programmes compilés par le compilateur
ASMFLAGS=-g -F dwarf -f elf64
$(OBJ_DIR)/builtins.o: $(SRC_DIR)/builtins.asm
	nasm $(ASMFLAGS) -o $@ $<

produced_asm: $(OBJ_DIR)/builtins.o
	nasm $(ASMFLAGS) -o $(OBJ_DIR)/_anonymous.o _anonymous.asm
	$(CC) $(OBJ_DIR)/builtins.o $(OBJ_DIR)/_anonymous.o -o $(OBJ_DIR)/_anonymous -nostartfiles -no-pie -m64 -g3

rapport: $(REPORT_DIR)/rapport.pdf

rep/rapport.pdf: $(REPORT_DIR)/rapport.md
	@mkdir --parents $(REPORT_DIR)/logos
	@wget --quiet --show-progress --no-clobber -O rep/logos/LogoLIGM.png "https://drive.google.com/uc?export=download&confirm=yes&id=1cZjxS6Rwp8LU4Eyahqz0eUS8aH0_VrVB" || true
	@wget --quiet --show-progress --no-clobber -O rep/logos/namedlogoUGE.png "https://drive.google.com/uc?export=download&confirm=yes&id=1YGm1N7griuDbJhC6rSgBHrrcOsHKM5xg" || true
	pandoc --toc $^ -o $@ --metadata-file=rep/metadata.yaml 

.PHONY: clean distclean dir test

distclean:
	rm -f $(OBJS)
	rm -rf .pytest_cache/
	rm -rf $(TESTS_DIR)/__pycache__/

clean: distclean
	rm -f $(BIN_DIR)/$(EXEC)

# The top three tar args assure that files don't have usernames, groups, and no executable bit
tar_assignment:
	tar --owner=0 --group=0 --mode='a=r,u+rw,a+X' -czf $(TAR_NAME).tar.gz --transform 's,^,$(TAR_NAME)/,' $(TAR_CONTENT)

assignment: clean rapport
	@$(MAKE) --no-print-directory tar_assignment

test: $(BIN_DIR)/$(EXEC)
	python3 test/test.py

safe_rendu:
	@$(MAKE) --no-print-directory clean
	@$(MAKE) --no-print-directory test
	@$(MAKE) --no-print-directory assignment

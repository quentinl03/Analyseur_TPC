# Makefile TP Flex

# $@ : the current target
# $^ : the current prerequisites
# $< : the first current prerequisite

CC=gcc
CFLAGS=-Wall -g -O0
LDFLAGS=-Wall -lfl -Werror -Wfatal-errors
EXEC=tpcc

# Filenames
PARSER=tpc
LEXER=tpc

FLEX_FLAGS=
BISON_FLAGS=

SRC_DIR=src
OBJS_DIR=obj
BIN_DIR=bin
TESTS_DIR=test
REPORT_DIR=rep
OUT_DIRS=$(OBJS_DIR) $(BIN_DIR)

MODULES=$(patsubst %.c, $(OBJS_DIR)/%.o, tree.c parser.c main.c symbol.c symboltable.c arraylist.c registers.c treeReader.c codeWriter.c)
OBJS=$(wildcard $(OBJS_DIR)/*.tab.* $(OBJS_DIR)/*.yy.* $(OBJS_DIR)/*.o)

TAR_CONTENT=$(SRC_DIR)/ $(TESTS_DIR)/ $(REPORT_DIR)/ $(OBJS_DIR)/ $(BIN_DIR) Makefile README.md
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

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c | $(OUT_DIRS)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)/$(LEXER).yy.c: $(SRC_DIR)/$(LEXER).lex $(OBJS_DIR)/$(PARSER).tab.h | $(OUT_DIRS)
	flex $(FLEX_FLAGS) --outfile=$@ $<

$(OBJS_DIR)/$(PARSER).tab.h $(OBJS_DIR)/$(PARSER).tab.c &: $(SRC_DIR)/$(PARSER).y $(MODULES) | $(OUT_DIRS)
	bison $(BISON_FLAGS) -d $< --output=$(OBJS_DIR)/$(PARSER).tab.c

$(BIN_DIR)/$(EXEC): $(OBJS_DIR)/$(LEXER).yy.o $(OBJS_DIR)/$(PARSER).tab.o $(MODULES) | $(OUT_DIRS)
	$(CC) $^ -o $@ $(LDFLAGS)

rapport: $(REPORT_DIR)/rapport.pdf

rep/rapport.pdf: $(REPORT_DIR)/rapport.md
	@mkdir --parents $(REPORT_DIR)/logos
	@wget --quiet --show-progress --no-clobber -O rep/logos/LogoLIGM.png "https://drive.google.com/uc?export=download&confirm=yes&id=1cZjxS6Rwp8LU4Eyahqz0eUS8aH0_VrVB" || true
	@wget --quiet --show-progress --no-clobber -O rep/logos/namedlogoUGE.png "https://drive.google.com/uc?export=download&confirm=yes&id=1YGm1N7griuDbJhC6rSgBHrrcOsHKM5xg" || true
	pandoc --toc $^ -o $@ --metadata-file=rep/metadata.yaml 

.PHONY: clean distclean dir

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

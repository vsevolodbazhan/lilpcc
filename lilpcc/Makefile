CC=gcc
CFLAGS=-O2 -std=c99 -Wall -Wextra

INC_DIR=-I ./inc/
SRC_DIR=src
BLD_DIR=build

all: $(BLD_DIR)/lilpcc

$(BLD_DIR):
	@mkdir $(BLD_DIR)

$(BLD_DIR)/lexer.c: $(SRC_DIR)/lexer.l $(BLD_DIR)/parser.tab.h
	flex -t $< > $@

$(BLD_DIR)/lexer.o: $(BLD_DIR)/lexer.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BLD_DIR)/parser.tab.c $(BLD_DIR)/parser.tab.h: $(SRC_DIR)/parser.y
	bison -d $< -o $(BLD_DIR)/parser.tab.c

$(BLD_DIR)/parser.tab.o: $(BLD_DIR)/parser.tab.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BLD_DIR)/list.o: $(SRC_DIR)/list.c
	$(CC) $(INC_DIR) $(CFLAGS) -c $< -o $@

$(BLD_DIR)/stack.o: $(SRC_DIR)/stack.c
	$(CC) $(INC_DIR) $(CFLAGS) -c $< -o $@

$(BLD_DIR)/syntax.o: $(SRC_DIR)/syntax.c
	$(CC) $(INC_DIR) $(CFLAGS) -c $< -o $@

$(BLD_DIR)/symbol_table.o: $(SRC_DIR)/symbol_table.c
	$(CC) $(INC_DIR) $(CFLAGS) -c $< -o $@

$(BLD_DIR)/context.o: $(SRC_DIR)/context.c
	$(CC) $(INC_DIR) $(CFLAGS) -c $< -o $@

$(BLD_DIR)/assembly.o: $(SRC_DIR)/assembly.c
	$(CC) $(INC_DIR) $(CFLAGS) -c $< -o $@

$(BLD_DIR)/lilpcc: $(BLD_DIR) $(BLD_DIR)/lexer.o $(BLD_DIR)/parser.tab.o $(BLD_DIR)/list.o $(BLD_DIR)/stack.o $(BLD_DIR)/syntax.o $(BLD_DIR)/symbol_table.o $(BLD_DIR)/context.o $(BLD_DIR)/assembly.o
	$(CC) $(INC_DIR) $(CFLAGS) $(SRC_DIR)/lilpcc.c $(BLD_DIR)/*.o -o $@

.PHONY: clean
clean:
	rm -rf $(BLD_DIR)

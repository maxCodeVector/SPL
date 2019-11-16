CC=g++ -std=c++11
FLEX=flex
BISON=bison


SRC_DIR=src
OUT_DIR=bin
OBJS=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

TEST_DIR=test
TEST_SOURCE=$(sort $(wildcard $(TEST_DIR)/*.spl))

.lex: $(SRC_DIR)/lex.l
	cd $(SRC_DIR) && $(FLEX) lex.l
.syntax: $(SRC_DIR)/syntax.y
	cd $(SRC_DIR) && $(BISON) -t -v -d syntax.y


splc:.lex .syntax
	test -d bin || mkdir bin
	$(CC) $(wildcard $(SRC_DIR)/*.cpp) $(SRC_DIR)/syntax.tab.c -lfl -ly -o $(OUT_DIR)/splc
	@chmod +x bin/splc


test: bin/splc
	@$(foreach var, $(TEST_SOURCE),\
		$(OUT_DIR)/splc $(var) > $(patsubst %.spl,%.res,$(var)) 2>&1; \
		test -f $(patsubst %.spl,%.out,$(var)) || echo $(var) "ignore test";\
		test -f $(patsubst %.spl,%.out,$(var)) && \
		echo $(var) "test start" && \
		diff $(patsubst %.spl,%.res,$(var)) $(patsubst %.spl,%.out,$(var)) && \
		echo $(var) success; \
		echo ; \
	 )

.PHONY: clean
clean:
	@rm -rf bin/
	@cd src && rm -f lex.yy.* syntax.tab* *.out *.so syntax.output
	@-rm $(TEST_DIR)/*.res
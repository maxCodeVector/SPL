CC=g++ -std=c++11
FLEX=flex
BISON=bison


SRC_DIR=src
OUT_DIR=bin
SOURCE=$(wildcard $(SRC_DIR)/*.cpp)
OBJS=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
INCLUDE=$(wildcard $(SRC_DIR)/*.h) $(wildcard $(SRC_DIR)/*.hpp)

TEST_DIR=test
TEST_SOURCE=$(sort $(wildcard $(TEST_DIR)/*.spl))


splc:$(SRC_DIR)/syntax.tab.c $(OBJS)
	test -d bin || mkdir bin
	$(CC) $(OBJS) $(SRC_DIR)/syntax.tab.c -lfl -ly -o $(OUT_DIR)/splc
	@chmod +x bin/splc

%.o: %.cpp $(INCLUDE)
	$(CC) -c $< -o $@

$(SRC_DIR)/syntax.tab.c:$(SRC_DIR)/lex.l $(SRC_DIR)/syntax.y
	cd $(SRC_DIR) && $(FLEX) lex.l && $(BISON) -t -v -d syntax.y


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
	@cd src && rm -f lex.yy.* syntax.tab* *.out *.so syntax.output *.o
	@-rm $(TEST_DIR)/*.res
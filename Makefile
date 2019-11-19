CC=g++ -std=c++11
FLEX=flex
BISON=bison


SRC_DIR=src
OUT_DIR=bin
BISON_SRC=src/parse

SOURCE=$(wildcard $(SRC_DIR)/*.cpp)
SOURCE+=$(wildcard $(BISON_SRC)/*.cpp)
OBJS=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
INCLUDE=$(wildcard $(SRC_DIR)/*.h) $(wildcard $(SRC_DIR)/*.hpp)

TEST_DIR=test
TEST_SOURCE=$(sort $(wildcard $(TEST_DIR)/*.spl))


splc:$(BISON_SRC)/syntax.tab.o $(OBJS)
	test -d bin || mkdir bin
	$(CC) -s $(OBJS) $(BISON_SRC)/syntax.tab.o -lfl -ly -o $(OUT_DIR)/splc
	@chmod +x bin/splc

%.o: %.cpp $(INCLUDE)
	$(CC) -c -g $< -o $@

parse:$(BISON_SRC)/syntax.tab.o

$(BISON_SRC)/syntax.tab.o:$(BISON_SRC)/lex.l $(BISON_SRC)/syntax.y
	cd $(BISON_SRC) && $(FLEX) lex.l && $(BISON) -t -v -d syntax.y && \
	$(CC) -c -g syntax.tab.c -o syntax.tab.o


debug:$(BISON_SRC)/syntax.tab.o $(OBJS)
	$(CC) -g $(OBJS) $(BISON_SRC)/syntax.tab.o -lfl -ly -o $(SRC_DIR)/splc.out


test: bin/splc
	@$(foreach var, $(TEST_SOURCE),\
		$(OUT_DIR)/splc $(var) > $(patsubst %.spl,%.res,$(var)); \
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
	@-cd src && rm *.o *.out
	@-cd src/parse && rm -f lex.yy.* syntax.tab* *.so syntax.output *.o
	@-rm $(TEST_DIR)/*.res
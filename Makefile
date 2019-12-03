CC=g++ -std=c++11
FLEX=flex
BISON=bison


SRC_DIR=src
OUT_DIR=bin
BISON_SRC=src/parse
NODE_SRC=src/astnode

SOURCE=$(wildcard $(SRC_DIR)/*/*.cpp)
SOURCE+=$(wildcard $(SRC_DIR)/*.cpp)

OBJS=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
BUILD_OBJS=$(subst $(SRC_DIR), $(OUT_DIR)/build, $(OBJS))
INCLUDE=$(wildcard $(SRC_DIR)/*.h) $(wildcard $(SRC_DIR)/*.hpp)
INCLUDE+=$(wildcard $(BISON_SRC)/*.h) $(wildcard $(BISON_SRC)/*.hpp)
INCLUDE+=$(wildcard $(NODE_SRC)/*.h) $(wildcard $(NODE_SRC)/*.hpp)

TEST_DIR=test
TEST_SOURCE=$(sort $(wildcard $(TEST_DIR)/*.spl))


splc:$(BISON_SRC)/syntax.tab.o $(BUILD_OBJS)
	test -d bin || mkdir bin
	$(CC) -s $(BUILD_OBJS) $(BISON_SRC)/syntax.tab.o -lfl -ly -o $(OUT_DIR)/splc
	@chmod +x bin/splc

bin/build/%.o:src/%.cpp $(INCLUDE)
	@test -d $(dir $@) || mkdir -p $(dir $@)
	$(CC) -c -g $< -o $@

parse:$(BISON_SRC)/syntax.tab.o

$(BISON_SRC)/syntax.tab.o:$(BISON_SRC)/lex.l $(BISON_SRC)/syntax.y
	cd $(BISON_SRC) && $(FLEX) lex.l && $(BISON) -t -v -d syntax.y && \
	$(CC) -c -g syntax.tab.c -o syntax.tab.o


debug:$(BISON_SRC)/syntax.tab.o $(BUILD_OBJS)
	$(CC) -g $(BUILD_OBJS) $(BISON_SRC)/syntax.tab.o -lfl -ly -o $(OUT_DIR)/splc.out


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


wc:
	find . -name "*.cpp" |xargs cat|wc -l
	find . -name "*.h" |xargs cat|wc -l
	find . -name "*.l" |xargs cat|wc -l
	find . -name "*.y" |xargs cat|wc -l


.PHONY: clean
clean:
	@rm -rf bin/
	@-cd src/parse && rm -f lex.yy.* syntax.tab* *.so syntax.output *.o
	@-rm $(TEST_DIR)/*.res
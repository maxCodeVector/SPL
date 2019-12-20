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

check:
	cd $(TEST_DIR)/diffc && gcc break_continue.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc empty_if.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc nested_struct_array.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc short_circuit.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc testb1.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc testb2.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc testb3.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc testb4.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc testb5.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc testr1.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc testr2.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc testr3.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc testr4.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc testr5.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc testr6.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc testr7.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc testr8.c && ./a.out < input.txt
	-cd $(TEST_DIR)/diffc && gcc testr9.c && ./a.out < input.txt
	cd $(TEST_DIR)/diffc && gcc testr10.c && ./a.out < input.txt
	rm $(TEST_DIR)/diffc/a.out


run:
	@$(foreach var, $(TEST_SOURCE),\
		spim -file $(patsubst %.spl,%.s,$(var)); \
		echo $(patsubst %.spl,%.s,$(var));\
		echo "";\
	)
	@$(foreach var, $(TEST_SOURCE),\
		irsim $(patsubst %.spl,%.ir,$(var)) -i 5,15,6,19,3,7; \
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
	@-rm $(TEST_DIR)/*.res  $(TEST_DIR)/*.ir
CC=g++ -std=c++11
FLEX=flex
BISON=bison


SRC_DIR=src
OUT_DIR=bin
OBJS=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

TEST_DIR=proj1-test
TEST_SOURCE=$(sort $(wildcard $(TEST_DIR)/*.spl))

.lex: lex.l
	$(FLEX) lex.l
.syntax: syntax.y
	$(BISON) -t -d syntax.ypp

splc:
	test -d bin || mkdir bin
	cd $(SRC_DIR) && \
	$(BISON) -t -v -d syntax.ypp && \
	$(FLEX) lex.l
	$(CC) $(wildcard $(SRC_DIR)/*.cpp) -lfl -ly -o $(OUT_DIR)/splc
	@chmod +x bin/splc
clean:
	@rm -rf bin/
	@cd src && rm -f lex.yy.* syntax.tab* *.out *.so syntax.output
	@rm $(TEST_DIR)/*.res
.PHONY: splc
.PHONY: test


test: bin/splc
	@$(foreach var, $(TEST_SOURCE),\
		bin/splc $(var) > $(patsubst %.spl,%.res,$(var)) 2>&1; \
		test -f $(patsubst %.spl,%.out,$(var)) || echo $(var) "ignore test"; \
		test -f $(patsubst %.spl,%.out,$(var)) && \
		echo $(var) "test start" && \
		diff $(patsubst %.spl,%.res,$(var)) $(patsubst %.spl,%.out,$(var)) && \
		echo $(var) success; \
		echo ; \
	 )
CC=g++ -std=c++11
FLEX=flex
BISON=bison


TEST_DIR=test
SOURCE=$(sort $(wildcard $(TEST_DIR)/*.spl))
OBJS=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

.lex: lex.l
	$(FLEX) lex.l
.syntax: syntax.y
	$(BISON) -t -d syntax.ypp
	


splc:
	test -d bin || mkdir bin
	cd src && \
	$(BISON) -t -v -d syntax.y && \
	$(FLEX) lex.l  && \
	$(CC) deliver.cpp syntax.tab.c -lfl -ly -o ../bin/splc
	@chmod +x bin/splc
clean:
	@rm -rf bin/
	@cd src && rm -f lex.yy.* syntax.tab* *.out *.so syntax.output
	@rm $(TEST_DIR)/*.res
.PHONY: splc
.PHONY: test


test: bin/splc
	@$(foreach var, $(SOURCE),\
		bin/splc $(var) > $(patsubst %.spl,%.res,$(var)) 2>&1; \
		test -f $(patsubst %.spl,%.out,$(var)) || echo $(var) "ignore test"; \
		test -f $(patsubst %.spl,%.out,$(var)) && \
		echo $(var) "test start" && \
		diff $(patsubst %.spl,%.res,$(var)) $(patsubst %.spl,%.out,$(var)) && \
		echo $(var) success; \
		echo ; \
	 )
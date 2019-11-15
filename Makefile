CC=g++ -std=c++11
FLEX=flex
BISON=bison

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
	cd src && rm -f lex.yy.* syntax.tab* *.out *.so syntax.output
.PHONY: splc
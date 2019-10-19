CC=gcc
FLEX=flex
BISON=bison

.lex: lex.l
	$(FLEX) lex.l
.syntax: syntax.y
	$(BISON) -t -d syntax.y
	


splc:
	test -d bin || mkdir bin
	cd src && \
	$(BISON) -t -v -d syntax.y && \
	$(FLEX) lex.l  && \
	$(CC) deliver.c syntax.tab.c -lfl -ly -o ../bin/splc
	@chmod +x bin/splc
clean:
	@rm -rf bin/
	cd src && rm -f lex.yy.c syntax.tab* *.out *.so syntax.output
.PHONY: splc

check:
	for testf in 1 2 3 4 5 6 7 8 9 10 11 12 13; do \
		echo ""; \
		echo test/test_1_r0$$testf.spl; \
		bin/splc test/test_1_r0$$testf.spl > test/test_1_r0$$testf.res ; \
		test -f test/test_1_r0$$testf.out && diff test/test_1_r0$$testf.out test/test_1_r0$$testf.res; \
	done
	@rm -f /tmp/my.out
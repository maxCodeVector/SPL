%{
    extern "C" int yylex();
    #define YYSTYPE struct AttrNode*
    #include"syntax.tab.h"
    #include "deliver.h"
    int has_error = 0;
    /* library inclusions */
    int yycolno = 1;
    #define YY_USER_ACTION \
        yylloc.first_line = yylineno; \
        yylloc.first_column = yycolno; \
        yylloc.last_line = yylineno; \
        yylloc.last_column = yycolno + yyleng; \
        yycolno += yyleng;
    #define _DUPTEXT  yylval->value = strdup(yytext)
    #define RECORD_LINENO  yylval->lineNo = yylineno;yylval->isTerminal=1
    #define REPORT_LEX_ERROR \
        fprintf(stderr, "Error type A at Line %d: unknown lexeme %s\n", yylineno, yytext); \
        has_error++


%}
%option yylineno
letter [a-zA-Z]
digits [0-9]
non_zero_digits [1-9]
under_line _
HEX ([0-9]|[A-Fa-f])
NONE_ZERO_HEX ({non_zero_digits}|[a-fA-F])
HEXCHAR_SEQUENCE ("\\x"{HEX}{HEX})
HEXINT_SEQUENCE (0[xX](0|{NONE_ZERO_HEX}{HEX}*))
INT_PART ({non_zero_digits}{digits}*|0)
INT_Pattern     {INT_PART}|{HEXINT_SEQUENCE}
FLOAT_Pattern   ({INT_PART}\.([0-9]+))
IDENTIFIER      ([_a-zA-Z][_a-zA-Z0-9]*)
BAD_IDENTIFIER      ([0-9][_a-zA-Z0-9]+)
BAD_CHAR (\'[^\']*\')
WHITESPACE      ([ \t]*)
STRING_LITERAL \"[^\"]*\"
%%
"struct" {yylval = make_node("STRUCT"); RECORD_LINENO;return STRUCT;}
"if"    {yylval = make_node("IF"); RECORD_LINENO;return IF;}
"else"  {yylval = make_node("ELSE"); RECORD_LINENO;return ELSE;}
"while" {yylval = make_node("WHILE"); RECORD_LINENO;return WHILE;}
"return"    {yylval = make_node("RETURN"); RECORD_LINENO;return RETURN;}
"." {yylval = make_node("DOT"); RECORD_LINENO;return DOT;}
";" {yylval = make_node("SEMI"); RECORD_LINENO;return SEMI;}
"," {yylval = make_node("COMMA"); RECORD_LINENO;return COMMA;}
"=" {yylval = make_node("ASSIGN"); RECORD_LINENO;return ASSIGN;}
"<" {yylval = make_node("LT"); RECORD_LINENO;return LT;}
"<="    {yylval = make_node("LE"); RECORD_LINENO;return LE;}
">" {yylval = make_node("GT"); RECORD_LINENO;return GT;}
">="    {yylval = make_node("GE"); RECORD_LINENO;return GE;}
"!="    {yylval = make_node("NE"); RECORD_LINENO;return NE;}
"=="    {yylval = make_node("EQ"); RECORD_LINENO;return EQ;}
"+" {yylval = make_node("PLUS"); RECORD_LINENO;return PLUS;}
"-" {yylval = make_node("MINUS"); RECORD_LINENO;return MINUS;}
"*" {yylval = make_node("MUL"); RECORD_LINENO;return MUL;}
"/" {yylval = make_node("DIV'"); RECORD_LINENO;return DIV;}
"&&" {yylval = make_node("AND"); RECORD_LINENO;return AND;}
"||" {yylval = make_node("OR"); RECORD_LINENO;return OR;}
"!" {yylval = make_node("NOT"); RECORD_LINENO;return NOT;}
"(" {yylval = make_node("LP"); RECORD_LINENO;return LP;}
")" {yylval = make_node("RP"); RECORD_LINENO;return RP;}
"[" {yylval = make_node("LB"); RECORD_LINENO;return LB;}
"]" {yylval = make_node("RB"); RECORD_LINENO;return RB;}
"{" {yylval = make_node("LC"); RECORD_LINENO;return LC;}
"}" {yylval = make_node("RC"); RECORD_LINENO;return RC;}
"\n"        {yycolno = 1;}
"//" { 
        char c; 
        while((c=yyinput()) != '\n'); 
        unput('\n');
        // printf("apperaed a one line comment in line %d!\n", yylineno);
    }
"/*" { 
        int start_line_no = yylineno;
        char c = yyinput();
        int new_line_no = 0;
        int comment_end = 0;
        int nested = 0;
        while(comment_end!=1 && c!=0){
            switch(c){
                case '\n':new_line_no ++;break;
                case '*':
                    c = yyinput();
                    if(c=='/'){
                        //printf("apperaed a multiline comment from line %d to %d\n", start_line_no, yylineno);
                        comment_end = 1;
                    }else unput(c);
                    break;
                 case '/':
                    c = yyinput();
                    if(c=='*'){
                        //printf("apperaed a nested multiline comment in line %d\n", yylineno);
                        nested = yylineno; // nested comment
                    }else unput(c);
                    break;
            }
            if(comment_end!=0){
                break;
            }
            c = yyinput();
        }
        for(int i=0;i<new_line_no;i++){
            unput('\n');
        }
        if(nested!=0){
            fprintf(stderr, "Error type B at Line %d: %s\n", nested, "syntax error, nested multiline comment");
            has_error ++;
        }
        if(comment_end!=1){
            fprintf(stderr, "Error type B at Line %d: %s\n", start_line_no, "syntax error, unfinished comment");
            has_error ++;
            // return TOKENERROR;
        }
    }
{WHITESPACE}    { /* ignore every whitespace */ }

"int"|"float"|"char" {yylval = make_node("TYPE");_DUPTEXT; RECORD_LINENO;return TYPE;}
"\'"."\'"|"\'"{HEXCHAR_SEQUENCE}"\'"    {yylval = make_node("CHAR");_DUPTEXT;yylval->ch=yytext[0]; RECORD_LINENO;return CHAR;}
{INT_Pattern}   {yylval = make_node("INT");_DUPTEXT; RECORD_LINENO;return INT;}
{FLOAT_Pattern} {yylval = make_node("FLOAT");_DUPTEXT; RECORD_LINENO;return FLOAT;}
{IDENTIFIER}    {yylval = make_node("ID");_DUPTEXT; RECORD_LINENO;return ID;}
{STRING_LITERAL} {
        if(yytext[yyleng-2] == '\\') {
            yyless(yyleng-1);
            yymore();
        } else {
            yylval = make_node("STRING");
            _DUPTEXT;
            RECORD_LINENO;
            return STRING;
        }
    }
{BAD_CHAR} {
        REPORT_LEX_ERROR;
        yylval = make_node("CHAR");
        _DUPTEXT;
        yylval->ch=yytext[0]; 
        RECORD_LINENO;
        return TOKENERROR;
    }
{BAD_IDENTIFIER}    {REPORT_LEX_ERROR;yylval = make_node("ID");_DUPTEXT; RECORD_LINENO;return TOKENERROR;}
.   {
        yylval = make_node("ERROR");
        _DUPTEXT;
        RECORD_LINENO;
        REPORT_LEX_ERROR; 
        return TOKENERROR;
    }

%%

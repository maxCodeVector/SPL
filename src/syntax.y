%{
    #include "deliver.h"
    #include"lex.yy.c"
    void yyerror(const char*);
    int result;
    int has_error = 0;
    //yydebug = 1;
%}

%locations
%error-verbose
%token TYPE
%token WHILE
%token RETURN STRUCT
%token SEMI COMMA
%token INT FLOAT CHAR ID
%right ASSIGN
%left OR
%left AND
%left NE
%left EQ
%left GE
%left GT
%left LE
%left LT
%left MINUS
%left PLUS
%left DIV
%left MUL
%right NOT
%left DOT
%left LC RC
%left LB RB
%left LP RP
%nonassoc IF
%nonassoc ELSE
%nonassoc TOKENERROR
%%
/* high-level definition */
Program: ExtDefList {
                $$ = make_parent($1, "Program");
                if(has_error==0){
                    show_sytax_tree($$);
                }
            }
    ;
ExtDefList: ExtDef ExtDefList  {
                $$ = make_parent($1, "ExtDefList");
                add_childs($$, $2);
            }
    |   {$$ = make_node("ExtDefList");$$->lineNo=-1;}
    ;
ExtDef: Specifier ExtDecList SEMI  {
        $$ = make_parent($1, "ExtDef");
        add_childs($$, $2);
        add_childs($$, $3);
    }
    | Specifier SEMI {
        $$ = make_parent($1, "ExtDef");
        add_childs($$, $2);
    }
    | Specifier FunDec CompSt {
        $$ = make_parent($1, "ExtDef");
        add_childs($$, $2);
        add_childs($$, $3);
    }
    ;
ExtDecList: VarDec  {
        $$ = make_parent($1, "ExtDecList");
        }
    | VarDec COMMA ExtDecList {
        $$ = make_parent($1, "ExtDecList");
        add_childs($$, $2);
        add_childs($$, $3);
    }
    ;

/* specifier */
Specifier: TYPE {
                    $$ = make_parent($1, "Specifier");    
                }
    | StructSpecifier {
                    $$ = make_parent($1, "Specifier");
                }
    ;
StructSpecifier: STRUCT ID LC DefList RC {
        $$ = make_parent($1, "StructSpecifier");
        add_childs($$, $2);
        add_childs($$, $3);
        add_childs($$, $4);
        add_childs($$, $5);
    }
    | STRUCT ID {
        $$ = make_parent($1, "StructSpecifier");
        add_childs($$, $2);
    }
    ;

/* declarator */
VarDec: ID   {
                $$ = make_parent($1, "VarDec");
            }
    | VarDec LB INT RB {
                $$ = make_parent($1, "VarDec");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
            }
    ;
FunDec: ID LP VarList RP {
                $$ = make_parent($1, "FunDec");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
            }
    | ID LP RP {
                $$ = make_parent($1, "FunDec");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | ID LP error
    ;
VarList: ParamDec COMMA VarList {
                $$ = make_parent($1, "VarList");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | ParamDec{
                $$ = make_parent($1, "VarList");
            }
    ;
ParamDec: Specifier VarDec {
                $$ = make_parent($1, "ParamDec");
                add_childs($$, $2);
            }
    ;

/* statement */
CompSt: LC DefList StmtList RC {
                $$ = make_parent($1, "CompSt");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
            }
    ;
StmtList: Stmt StmtList {
                $$ = make_parent($1, "StmtList");
                add_childs($$, $2);
            }
    | {$$ = make_node("StmtList");$$->lineNo=-1;}
    ;
Stmt: Exp SEMI{
                $$ = make_parent($1, "Stmt");
                add_childs($$, $2);
            }
    | CompSt{
                $$ = make_parent($1, "Stmt");
            }
    | RETURN Exp SEMI {
                $$ = make_parent($1, "Stmt");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | IF LP Exp RP Stmt{
                $$ = make_parent($1, "Stmt");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
                add_childs($$, $5);
            }
    | IF LP Exp RP Stmt ELSE Stmt{
                $$ = make_parent($1, "Stmt");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
                add_childs($$, $5);
                add_childs($$, $6);
                add_childs($$, $7);
            }
    | WHILE LP Exp RP Stmt{
                $$ = make_parent($1, "Stmt");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
                add_childs($$, $5);
            }
    | Exp error
    | RETURN Exp error
    ;

/* local definition */
DefList: Def DefList{
                $$ = make_parent($1, "DefList");
                add_childs($$, $2);
            }
    | {$$ = make_node("DefList");$$->lineNo=-1;}
    ;
Def: Specifier DecList SEMI{
                $$ = make_parent($1, "Def");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    ;
DecList: Dec{
                $$ = make_parent($1, "DecList");
            }
    | Dec COMMA DecList {
                $$ = make_parent($1, "DecList");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    ;
Dec: VarDec{
                $$ = make_parent($1, "Dec");
            }
    | VarDec ASSIGN Exp {
                $$ = make_parent($1, "Dec");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    ;

/* Expression */
Exp: Exp ASSIGN Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | Exp AND Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | Exp OR Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | Exp LT Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | Exp LE Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | Exp GT Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | Exp GE Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | Exp NE Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | Exp EQ Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | Exp PLUS Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | Exp MINUS Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | Exp MUL Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | Exp DIV Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | LP Exp RP{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | MINUS Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
            }
    | NOT Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
            }
    | ID LP Args RP{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
            }
    | ID LP RP{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | Exp LB Exp RB{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
            }
    | Exp DOT ID{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | ID   {
                $$ = make_parent($1, "Exp"); 
            }
    | INT{
                $$ = make_parent($1, "Exp");
            }
    | FLOAT{
                $$ = make_parent($1, "Exp");
            }
    | CHAR{
                $$ = make_parent($1, "Exp");
            }
    | TOKENERROR {$$ = make_parent($1, "Exp");has_error++;}
    ;
Args: Exp COMMA Args{
                $$ = make_parent($1, "Args");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | Exp{
                $$ = make_parent($1, "Args");
            }
    ;
%%
void yyerror(const char *s){
    has_error++;
    fprintf(stderr, "ERROR type B at Line: %d: %s\n", yylloc.first_line, s);
}
int main(int argc, char **argv){
    yyin = fopen(argv[1], "r");
    yyparse();
    fclose(yyin);
}


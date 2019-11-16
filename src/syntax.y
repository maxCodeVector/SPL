%{
    #ifndef _SYNTAX
    #define _SYNTAX
    #include "deliver.h"
    #include "sematic.h"
    #include"lex.yy.c"
    void yyerror(const char*);
    int result;
    static AttrNode* root;
    //yydebug = 1;
    #endif
%}

%locations
%error-verbose
%token TYPE
%token WHILE
%token RETURN STRUCT
%token SEMI COMMA
%token INT FLOAT CHAR ID STRING
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
                root = $$;
                // if(has_error==0){
                //     show_sytax_tree($$);
                // }
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
    // | Specifier ExtDecList error
    // | Specifier error
    ;
ExtDecList: VarDec  {
        $$ = make_parent($1, "ExtDecList");
        }
    | VarDec COMMA ExtDecList {
        $$ = make_parent($1, "ExtDecList");
        add_childs($$, $2);
        add_childs($$, $3);
    }
    // |  VarDec error ExtDecList 
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
    // | STRUCT error LC DefList RC
    // | STRUCT ID error DefList RC
    // | STRUCT ID LC DefList error
    // | STRUCT error
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
    | TOKENERROR {
                $$ = make_parent($1, "VarDec");
            }
    // | VarDec error INT RB
    // | VarDec LB INT error
    // | VarDec LB error RB
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
    // | ID LP error
    // | ID error RP
    // | ID error
    ;
VarList: ParamDec COMMA VarList {
                $$ = make_parent($1, "VarList");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | ParamDec{
                $$ = make_parent($1, "VarList");
            }
    // | ParamDec COMMA error
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
    // | LC DefList StmtList error
    // | error DefList StmtList RC
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
    // | Exp error
    | CompSt{
                $$ = make_parent($1, "Stmt");
            }
    | RETURN Exp SEMI {
                $$ = make_parent($1, "Stmt");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | RETURN error SEMI
    // | RETURN Exp error
    | IF LP Exp RP Stmt{
                $$ = make_parent($1, "Stmt");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
                add_childs($$, $5);
            }
    // | IF error Exp RP Stmt
    // | IF LP Exp error Stmt
    // | IF LP error RP Stmt
    // | IF LP Exp RP error
    | IF LP Exp RP Stmt ELSE Stmt{
                $$ = make_parent($1, "Stmt");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
                add_childs($$, $5);
                add_childs($$, $6);
                add_childs($$, $7);
            }
    // | IF error Exp RP Stmt ELSE Stmt
    // | IF LP Exp error Stmt ELSE Stmt
    // | IF LP error RP Stmt ELSE Stmt
    // | IF LP Exp RP error ELSE Stmt
    // | IF LP Exp RP Stmt ELSE error
    | WHILE LP Exp RP Stmt{
                $$ = make_parent($1, "Stmt");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
                add_childs($$, $5);
            }
    // | WHILE error Exp RP Stmt
    // | WHILE LP Exp error Stmt
    // | WHILE LP Exp RP error
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
    // | Specifier DecList error
    ;
DecList: Dec{
                $$ = make_parent($1, "DecList");
            }
    | Dec COMMA DecList {
                $$ = make_parent($1, "DecList");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | Dec error DecList
    ;
Dec: VarDec{
                $$ = make_parent($1, "Dec");
            }
    | VarDec ASSIGN Exp {
                $$ = make_parent($1, "Dec");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | VarDec ASSIGN error
    ;

/* Expression */
Exp: Exp ASSIGN Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | Exp ASSIGN error
    | Exp AND Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | Exp AND error
    | Exp OR Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | Exp OR error
    | Exp LT Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | Exp LT error
    | Exp LE Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | Exp LE error
    | Exp GT Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | Exp GT error
    | Exp GE Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | Exp GE error
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
    // | Exp EQ error
    | Exp PLUS Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | Exp PLUS error
    | Exp MINUS Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | Exp MINUS error
    | Exp MUL Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | Exp MUL error
    | Exp DIV Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | Exp DIV error
    | LP Exp RP{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | error Exp RP
    // | LP Exp error
    | MINUS Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
            }
    // | MINUS error
    | NOT Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
            }
    // | NOT error
    | ID LP Args RP{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
            }
    // | ID LP Args error
    // | ID error Args RP
    | ID LP RP{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | Exp error RP
    // | Exp LP error
    | Exp LB Exp RB{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
            }
    // | Exp error Exp RB
    // | Exp LB Exp error
    | Exp DOT ID{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    // | Exp DOT error
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
    | Exp TOKENERROR Exp {
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | TOKENERROR {$$ = make_parent($1, "Exp");}
    ;
Args: Exp COMMA Args{
                $$ = make_parent($1, "Args");
                add_childs($$, $2);
                add_childs($$, $3);
            }
    | Exp{
                $$ = make_parent($1, "Args");
            }
    // | Exp error Args
    ;
%%
void yyerror(const char *s){
    has_error ++;
    fprintf(stderr, "Error type B at Line %d: %s\n", yylloc.first_line, s);
}
int main(int argc, char **argv){
    yyin = fopen(argv[1], "r");
    if(yyin==0){
       fprintf(stderr, "file not found:%s\n", argv[1]);
       return 1;
    }
    yyparse();
    fclose(yyin);
    if(has_error==0){
        show_sytax_tree(root);
        sematic_analysis(root);
    }else
        fprintf(stderr, "total error %d\n", has_error);
        
}

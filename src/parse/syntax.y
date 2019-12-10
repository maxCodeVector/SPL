%{
    #ifndef _SYNTAX
    #define _SYNTAX
    #include "../ast.h"
    #include "lex.yy.c"
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
%left MINUS PLUS
%left DIV MUL
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
                AST* ast = new AST($1);
                root -> baseNode = ast;
                // if(has_error==0){
                //     show_sytax_tree($$);
                // }
            }
    ;
ExtDefList: ExtDef ExtDefList  {
                $$ = make_parent($1, "ExtDefList");
                add_childs($$, $2);
                BaseNode* var = $1->baseNode;
                var->setNext($2);
                $$->baseNode = var;
            }
    |   {$$ = make_node("ExtDefList");$$->lineNo=-1;}
    ;
ExtDef: Specifier ExtDecList SEMI  {
        $$ = make_parent($1, "ExtDef");
        add_childs($$, $2);
        add_childs($$, $3);
        Variable* var = (Variable*)$2->baseNode;
        var->setType($1);
        $$->baseNode = var; // variable/function
    }
    | Specifier SEMI {
        $$ = make_parent($1, "ExtDef");
        add_childs($$, $2);
        DeclaredVariableType* declare = new DeclaredVariableType($1);
        $$ -> baseNode = declare;
    }
    | Specifier FunDec CompSt {
        $$ = make_parent($1, "ExtDef");
        add_childs($$, $2);
        add_childs($$, $3);
        Function* func = (Function*)$2->baseNode;
        func->setBody($3);
        func->setReturnType($1);
        $$->baseNode = func;
    }
    // | Specifier ExtDecList error
    // | Specifier error
    ;
ExtDecList: VarDec  {
        $$ = make_parent($1, "ExtDecList");
        $$->baseNode = $1->baseNode;	//variable
    }
    | VarDec COMMA ExtDecList {
        $$ = make_parent($1, "ExtDecList");
        add_childs($$, $2);
        add_childs($$, $3);
        BaseNode* var = $1->baseNode;
        var->setNext($3);
        $$->baseNode = var;
    }
    // |  VarDec error ExtDecList 
    ;

/* specifier */
Specifier: TYPE {
		    $$ = make_parent($1, "Specifier");
		    VariableType* var = getVariableType($1->value);
		    $$ -> baseNode = var;
                }
    | StructSpecifier {
                    $$ = make_parent($1, "Specifier");
		    $$ -> baseNode = $1->baseNode;
                }
    ;
StructSpecifier: STRUCT ID LC DefList RC {
        $$ = make_parent($1, "StructSpecifier");
        add_childs($$, $2);
        add_childs($$, $3);
        add_childs($$, $4);
        add_childs($$, $5);
	Struct* st = new Struct($2, $4);
	$$ -> baseNode = st;
    }
    | STRUCT ID {
        $$ = make_parent($1, "StructSpecifier");
        add_childs($$, $2);
        Struct* st = new Struct($2);
	$$ -> baseNode = st;
    }
    // | STRUCT error LC DefList RC
    // | STRUCT ID error DefList RC
    // | STRUCT ID LC DefList error
    // | STRUCT error
    ;

/* declarator */
VarDec: ID   {
                $$ = make_parent($1, "VarDec");
                Variable* var = new Variable($1);
                $$->baseNode = var;
            }
    | VarDec LB INT RB {
                $$ = make_parent($1, "VarDec");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
	 	Variable* var = (Variable*)$1->baseNode;
	 	var->addDimension($3);
		$$->baseNode = var;
            }
    | TOKENERROR {
                $$ = make_parent($1, "VarDec");
                //todo with AST
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
		Function* func = new Function($1, $3);
		$$->baseNode = func;
            }
    | ID LP RP {
                $$ = make_parent($1, "FunDec");
                add_childs($$, $2);
                add_childs($$, $3);
                Function* func = new Function($1);
		$$->baseNode = func;
            }
    // | ID LP error
    // | ID error RP
    // | ID error
    ;
VarList: ParamDec COMMA VarList {
                $$ = make_parent($1, "VarList");
                add_childs($$, $2);
                add_childs($$, $3);
		BaseNode* var = $1->baseNode;
		var-> setNext($3);
		$$->baseNode = var;
            }
    | ParamDec{
                $$ = make_parent($1, "VarList");
                $$ -> baseNode = $1->baseNode;
            }
    // | ParamDec COMMA error
    ;
ParamDec: Specifier VarDec {
                $$ = make_parent($1, "ParamDec");
                add_childs($$, $2);
                Variable* var = (Variable*)$2->baseNode;
                var->setType($1);
                $$->baseNode = var;
            }
    ;

/* statement */
CompSt: LC DefList StmtList RC {
                $$ = make_parent($1, "CompSt");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
                Body* body = new Body($2, $3);
                $$ -> baseNode = body;
            }
    // | LC DefList StmtList error
    // | error DefList StmtList RC
    ;
StmtList: Stmt StmtList {
                $$ = make_parent($1, "StmtList");
                add_childs($$, $2);
                BaseNode* stmt = $1->baseNode;
                stmt->setNext($2);
                $$-> baseNode = stmt;
            }
    | {$$ = make_node("StmtList");$$->lineNo=-1;}
    ;
Stmt: Exp SEMI{
                $$ = make_parent($1, "Stmt");
                add_childs($$, $2);
                Statement* stmt = new Statement($1);
                $$ -> baseNode = stmt;
            }
    // | Exp error
    | CompSt{
                $$ = make_parent($1, "Stmt");
                $$-> baseNode = $1->baseNode;
            }
    | RETURN Exp SEMI {
                $$ = make_parent($1, "Stmt");
                add_childs($$, $2);
                add_childs($$, $3);
                ReturnStatement* stmt = new ReturnStatement($2);
                $$ -> baseNode = stmt;
            }
    // | RETURN error SEMI
    // | RETURN Exp error
    | IF LP Exp RP Stmt{
                $$ = make_parent($1, "Stmt");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
                add_childs($$, $5);
                IfStatement* stmt = new IfStatement($3, $5);
                $$ -> baseNode = stmt;
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
                IfStatement* stmt = new IfStatement($3, $5, $7);
		$$ -> baseNode = stmt;
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
                WhileStatement* stmt = new WhileStatement($3, $5);
                $$->baseNode = stmt;
            }
    // | WHILE error Exp RP Stmt
    // | WHILE LP Exp error Stmt
    // | WHILE LP Exp RP error
    ;

/* local definition */
DefList: Def DefList{
                $$ = make_parent($1, "DefList");
                add_childs($$, $2);
                BaseNode* var = $1->baseNode;
		var -> setNext($2);
		$$ -> baseNode = var;
            }
    | {$$ = make_node("DefList");$$->lineNo=-1;}
    ;
Def: Specifier DecList SEMI{
                $$ = make_parent($1, "Def");
                add_childs($$, $2);
                add_childs($$, $3);
		Variable* var = (Variable*)$2->baseNode;
		var -> setType($1);
		$$ -> baseNode = var;
            }
    // | Specifier DecList error
    ;
DecList: Dec{
                $$ = make_parent($1, "DecList");
                $$->baseNode = $1->baseNode;
            }
    | Dec COMMA DecList {
                $$ = make_parent($1, "DecList");
                add_childs($$, $2);
                add_childs($$, $3);
                BaseNode* var = $1->baseNode;
                var->setNext($3);
                $$ -> baseNode = var;
            }
    // | Dec error DecList
    ;
Dec: VarDec{
                $$ = make_parent($1, "Dec");
                $$ -> baseNode = $1 -> baseNode;
            }
    | VarDec ASSIGN Exp {
                $$ = make_parent($1, "Dec");
                add_childs($$, $2);
                add_childs($$, $3);
                Variable* var = (Variable*)$1->baseNode;
                var->setExp($3);
		$$ -> baseNode = var;
            }
    // | VarDec ASSIGN error
    ;

/* Expression */
Exp: Exp ASSIGN Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                BinaryExp* exp = new BinaryExp($1, $3, ASSIGN_OP);
		$$ -> baseNode = exp;
            }
    // | Exp ASSIGN error
    | Exp AND Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                BinaryExp* exp = new BinaryExp($1, $3, AND_OP);
		$$ -> baseNode = exp;
            }
    // | Exp AND error
    | Exp OR Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                BinaryExp* exp = new BinaryExp($1, $3, OR_OP);
		$$ -> baseNode = exp;
            }
    // | Exp OR error
    | Exp LT Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                BinaryExp* exp = new BinaryExp($1, $3, LT_OP);
		$$ -> baseNode = exp;
            }
    // | Exp LT error
    | Exp LE Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                BinaryExp* exp = new BinaryExp($1, $3, LE_OP);
		$$ -> baseNode = exp;
            }
    // | Exp LE error
    | Exp GT Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                BinaryExp* exp = new BinaryExp($1, $3, GT_OP);
		$$ -> baseNode = exp;
            }
    // | Exp GT error
    | Exp GE Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                BinaryExp* exp = new BinaryExp($1, $3, GE_OP);
		$$ -> baseNode = exp;
            }
    // | Exp GE error
    | Exp NE Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                BinaryExp* exp = new BinaryExp($1, $3, NE_OP);
		$$ -> baseNode = exp;
            }
    | Exp EQ Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                BinaryExp* exp = new BinaryExp($1, $3, EQ_OP);
		$$ -> baseNode = exp;
            }
    // | Exp EQ error
    | Exp PLUS Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                BinaryExp* exp = new BinaryExp($1, $3, ADD_OP);
		$$ -> baseNode = exp;
            }
    // | Exp PLUS error
    | Exp MINUS Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
		BinaryExp* exp = new BinaryExp($1, $3, SUB_OP);
		$$ -> baseNode = exp;
            }
    // | Exp MINUS error
    | Exp MUL Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
	 	BinaryExp* exp = new BinaryExp($1, $3, MUL_OP);
		$$ -> baseNode = exp;
            }
    // | Exp MUL error
    | Exp DIV Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                BinaryExp* exp = new BinaryExp($1, $3, DIV_OP);
                $$ -> baseNode = exp;
            }
    // | Exp DIV error
    | LP Exp RP{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                $$->baseNode = $2->baseNode;
            }
    // | error Exp RP
    // | LP Exp error
    | MINUS Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                UnaryExp* exp = new UnaryExp($2, SUB_OP);
                $$->baseNode = exp;
            }
    // | MINUS error
    | NOT Exp{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                UnaryExp* exp = new UnaryExp($2, NOT_OP);
                $$->baseNode = exp;
            }
    // | NOT error
    | ID LP Args RP{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
		InvokeExp* exp = new InvokeExp($1, $3);
		$$ -> baseNode = exp;
            }
    // | ID LP Args error
    // | ID error Args RP
    | ID LP RP{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                InvokeExp* exp = new InvokeExp($1);
                $$ -> baseNode = exp;
            }
    // | Exp error RP
    // | Exp LP error
    | Exp LB Exp RB{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
                add_childs($$, $4);
                BinaryExp* exp = new BinaryExp($1, $3, ARRAY_INDEX_OP);
		$$ -> baseNode = exp;
            }
    // | Exp error Exp RB
    // | Exp LB Exp error
    | Exp DOT ID{
                $$ = make_parent($1, "Exp");
                add_childs($$, $2);
                add_childs($$, $3);
    		GetAttributeExp* exp = new GetAttributeExp($1, $3->value);
		$$->baseNode = exp;
            }
    // | Exp DOT error
    | ID   {
                $$ = make_parent($1, "Exp");
	 	Exp* exp = new Exp($1, REF_TYPE);
		$$->baseNode = exp;
            }
    | INT{
                $$ = make_parent($1, "Exp");
                Exp* exp = new Exp($1, INT_TYPE);
                $$->baseNode = exp;
            }
    | FLOAT{
                $$ = make_parent($1, "Exp");
		Exp* exp = new Exp($1, FLOAT_TYPE);
		$$->baseNode = exp;
            }
    | CHAR{
                $$ = make_parent($1, "Exp");
                Exp* exp = new Exp($1, CHAR_TYPE);
		$$->baseNode = exp;
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
                BaseNode* exp = $1->baseNode;
                exp->setNext($3);
                $$->baseNode = exp;  // link_list of Exp
            }
    | Exp{
                $$ = make_parent($1, "Args");
		$$->baseNode = $1->baseNode;
            }
    // | Exp error Args
    ;
%%
void yyerror(const char *s){
    has_error ++;
    fprintf(stderr, "Error type B at Line %d: %s\n", yylloc.first_line, s);
}

AttrNode* get_parse_tree(int argc, char **argv){
    yyin = fopen(argv[1], "r");
    if(yyin==0){
       fprintf(stderr, "file not found:%s\n", argv[1]);
       return NULL;
    }
    yyparse();
    fclose(yyin);
    if(has_error!=0){
        fprintf(stderr, "total error %d\n", has_error);
        return NULL;
    }
    return root;
}


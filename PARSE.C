/****************************************************/
/* File: parse.c                                    */
/* The parser implementation for the TINY compiler  */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

static TokenType token; /* holds current token */

/* function prototypes for recursive calls */
static TreeNode* declarations(void);
static TreeNode* decl(void);
static TreeNode* stmt_sequence(void);
static TreeNode* statement(void);
static TreeNode* int_stmt(void);
static TreeNode* bool_stmt(void);
static TreeNode* string_stmt(void);
static TreeNode* if_stmt(void);
static TreeNode* repeat_stmt(void);
static TreeNode* while_stmt(void);
static TreeNode* assign_stmt(void);
static TreeNode* read_stmt(void);
static TreeNode* write_stmt(void);
static TreeNode* exp(void);
static TreeNode* simple_exp(void);
static TreeNode* term(void);
static TreeNode* factor(void);

static void syntaxError(char* message)
{
	fprintf(listing, "\n>>> ");
	fprintf(listing, "Syntax error at line %d: %s", lineno, message);
	Error = TRUE;
}

static void match(TokenType expected)
{
	if (token == expected) token = getToken();
	else {
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		fprintf(listing, "      ");
	}
}

TreeNode* program(void)
{
	TreeNode* t = newRootNode();
	t->child[0] = declarations();
	t->child[1] = stmt_sequence();
	return t;
}

TreeNode* declarations(void)
{
	TreeNode* t = decl();
	TreeNode* p = t;
	while ((token != ENDFILE) && (token != END))
	{
		TreeNode* q;
		match(SEMI);
		if ((token != INT) && (token != BOOL) && (token != STRING)) break;//标志声明语句结束
		q = decl();
		if (q != NULL) {
			if (t == NULL) t = p = q;
			else /* now p cannot be NULL either */
			{
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}

TreeNode* decl(void)
{
	TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	switch (token) {
	case INT: t = int_stmt(); break;
	case BOOL:t = bool_stmt(); break;
	case STRING: t = string_stmt(); break;
	default: syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken();
		break;
	} /* end case */
	return t;
}

TreeNode* stmt_sequence(void)
{
	TreeNode* t = statement();
	TreeNode* p = t;
	while ((token != ENDFILE) && (token != END) &&
		(token != ELSE) && (token != UNTIL) && (token != WHILE))
	{
		TreeNode* q;
		match(SEMI);
		q = statement();
		if (q != NULL) {
			if (t == NULL) t = p = q;
			else /* now p cannot be NULL either */
			{
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}

TreeNode* statement(void)
{
	TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	switch (token) {
	case IF: t = if_stmt(); break;
	case REPEAT: t = repeat_stmt(); break;
	case DO: t = while_stmt(); break;
	case ID: t = assign_stmt(); break;
	case READ: t = read_stmt(); break;
	case WRITE: t = write_stmt(); break;
	default: syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken();
		break;
	} /* end case */
	return t;
}


TreeNode* int_stmt(void)
{
	TreeNode* t = newDeclNode(IntK);//生成Int节点
	if ((t != NULL) && (token == ID))
		t->attr.name = copyString(tokenString);
	match(INT);//匹配INT符号
	TreeNode* p = t;
	
	if (p != NULL) p->child[0] = newExpNode(IdK);//生成Id节点
	p = p->child[0];//把节点挂在语法树上
	if ((p != NULL) && (token == ID))
		p->attr.name = copyString(tokenString);//写入ID的名字
	match(ID);//匹配ID字符

	while ((token == COMMA))//如果有逗号分隔符，循环生成Id节点
	{
		match(COMMA);//匹配逗号字符
		TreeNode* q = newExpNode(IdK);//生成新的Id节点
		if ((q != NULL) && (token == ID))
			q->attr.name = copyString(tokenString);//写入ID的名字
		match(ID);
		if (q != NULL) {
			if (t == NULL) t = p = q;
			else /* now p cannot be NULL either */
			{
				p->child[0] = q;//把节点挂在语法树上
				p = q;
			}
		}
	}
	return t;//返回根节点
}

TreeNode* bool_stmt(void)
{
	TreeNode* t = newDeclNode(BoolK);
	if ((t != NULL) && (token == ID))
		t->attr.name = copyString(tokenString);
	match(BOOL);
	TreeNode* p = t;

	if (p != NULL) p->child[0] = newExpNode(IdK);
	p = p->child[0];
	if ((p != NULL) && (token == ID))
		p->attr.name = copyString(tokenString);
	match(ID);

	while ((token == COMMA))
	{
		match(COMMA);
		TreeNode* q = newExpNode(IdK);
		if ((q != NULL) && (token == ID))
			q->attr.name = copyString(tokenString);
		match(ID);
		if (q != NULL) {
			if (t == NULL) t = p = q;
			else /* now p cannot be NULL either */
			{
				p->child[0] = q;
				p = q;
			}
		}
	}
	return t;
}

TreeNode* string_stmt(void)
{
	TreeNode* t = newDeclNode(StringK);
	if ((t != NULL) && (token == ID))
		t->attr.name = copyString(tokenString);
	match(STRING);
	TreeNode* p = t;

	if (p != NULL) p->child[0] = newExpNode(IdK);
	p = p->child[0];
	if ((p != NULL) && (token == ID))
		p->attr.name = copyString(tokenString);
	match(ID);

	while ((token == COMMA))
	{
		match(COMMA);
		TreeNode* q = newExpNode(IdK);
		if ((q != NULL) && (token == ID))
			q->attr.name = copyString(tokenString);
		match(ID);
		if (q != NULL) {
			if (t == NULL) t = p = q;
			else /* now p cannot be NULL either */
			{
				p->child[0] = q;
				p = q;
			}
		}
	}
	return t;
}

TreeNode* if_stmt(void)
{
	TreeNode* t = newStmtNode(IfK);
	match(IF);
	if (t != NULL) t->child[0] = exp();
	match(THEN);
	if (t != NULL) t->child[1] = stmt_sequence();
	if (token == ELSE) {
		match(ELSE);
		if (t != NULL) t->child[2] = stmt_sequence();
	}
	match(END);
	return t;
}

TreeNode* repeat_stmt(void)
{
	TreeNode* t = newStmtNode(RepeatK);
	match(REPEAT);
	if (t != NULL) t->child[0] = stmt_sequence();
	match(UNTIL);
	if (t != NULL) t->child[1] = exp();
	return t;
}

TreeNode* while_stmt(void)
{
	TreeNode* t = newStmtNode(WhileK);//生成while节点
	match(DO);//匹配DO
	if (t != NULL) t->child[0] = stmt_sequence();//循环语句
	match(WHILE);//匹配WHILE
	if (t != NULL) t->child[1] = exp();//循环结束条件
	return t;
}

TreeNode* assign_stmt(void)
{
	TreeNode* t = newStmtNode(AssignK);
	if ((t != NULL) && (token == ID))
		t->attr.name = copyString(tokenString);
	match(ID);
	match(ASSIGN);//匹配赋值符号
	if (t != NULL && (token == STR))//识别到字符串常量
	{
		t->child[0] = newExpNode(ConstK);
		t->child[0]->attr.name = copyString(tokenString);
		t->child[0]->type = String;
		match(STR);
	}
	else if (t != NULL) t->child[0] = exp();//识别到数值
	return t;
}

TreeNode* read_stmt(void)
{
	TreeNode* t = newStmtNode(ReadK);
	match(READ);
	if ((t != NULL) && (token == ID))
		t->attr.name = copyString(tokenString);
	match(ID);
	return t;
}

TreeNode* write_stmt(void)
{
	TreeNode* t = newStmtNode(WriteK);
	match(WRITE);
	if (t != NULL) t->child[0] = exp();
	return t;
}

TreeNode* exp(void)
{
	TreeNode* t = simple_exp();
	if ((token == LT) || (token == EQ) || (token == LTE) || (token == GT) || (token == GTE)) {
		TreeNode* p = newExpNode(OpK);
		if (p != NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
		}
		match(token);
		if (t != NULL)
			t->child[1] = simple_exp();
	}
	return t;
}

TreeNode* simple_exp(void)
{
	TreeNode* t = term();
	while ((token == PLUS) || (token == MINUS))
	{
		TreeNode* p = newExpNode(OpK);
		if (p != NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
			match(token);
			t->child[1] = term();
		}
	}
	return t;
}

TreeNode* term(void)
{
	TreeNode* t = factor();
	while ((token == TIMES) || (token == OVER))
	{
		TreeNode* p = newExpNode(OpK);
		if (p != NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
			match(token);
			p->child[1] = factor();
		}
	}
	return t;
}

TreeNode* factor(void)
{
	TreeNode* t = NULL;
	switch (token) {
	case NUM:
		t = newExpNode(ConstK);
		if ((t != NULL) && (token == NUM))
			t->type = Integer;
			t->attr.val = atoi(tokenString);
		match(NUM);
		break;
	case ID:
		t = newExpNode(IdK);
		if ((t != NULL) && (token == ID))
			t->attr.name = copyString(tokenString);
		match(ID);
		break;
	case LPAREN:
		match(LPAREN);
		t = exp();
		match(RPAREN);
		break;
	default:
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken();
		break;
	}
	return t;
}

/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function parse returns the newly
 * constructed syntax tree
 */
TreeNode* parse(void)
{
	TreeNode* t;
	token = getToken();
	t = program();
	if (token != ENDFILE)
		syntaxError("Code ends before file\n");
	return t;
}

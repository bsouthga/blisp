#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include "mpc.h"

// blisp AST node
typedef struct bval {
  int type;
  int count;

  char* err;
  char* sym;

  double num;

  // recursive list of child nodes
  struct bval** cell;
} bval;

// bval types
enum {
  BVAL_NUM,
  BVAL_ERR,
  BVAL_SEXPR,
  BVAL_QEXPR,
  BVAL_SYM
};

// forward declare functions
bval* bval_num(double num);
bval* bval_err(char* err);
bval* bval_sym(char* sym);
bval* bval_sexpr(void);
bval* bval_qexpr(void);
bval* bval_read(mpc_ast_t* tree);
bval* bval_read_num(mpc_ast_t* tree);
bval* bval_add(bval* parent, bval* child);
bval* bval_eval(bval* v);
bval* bval_take(bval* v, int i);
bval* bval_pop(bval* v, int i);
bval* bval_join(bval* x, bval* y);
bval* bval_eval_sexpr(bval* v);
bval* builtin(bval* a, char* fn);
bval* builtin_op(bval* v, char* op);
bval* builtin_head(bval* a);
bval* builtin_tail(bval* a);
bval* builtin_eval(bval* a);
bval* builtin_join(bval* a);
bval* builtin_list(bval* a);


void bval_del(bval* v);
void bval_print(bval* v);
void bval_println(bval* v);
void bval_expr_print(bval* v, char open, char close);

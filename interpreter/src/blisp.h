#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include "mpc.h"

struct bval;
struct benv;
typedef struct bval bval;
typedef struct benv benv;

// builtin function pointer
typedef bval*(*bbuiltin)(benv*, bval*);

// environment
struct benv {
  int count;
  char** syms;
  bval** vals;
};

// blisp AST node
struct bval {
  int type;
  int count;
  char* err;
  char* sym;
  double num;
  bbuiltin fun;
  struct bval** cell;
};

// bval types
enum {
  BVAL_NUM,
  BVAL_ERR,
  BVAL_SEXPR,
  BVAL_QEXPR,
  BVAL_FUN,
  BVAL_SYM
};

benv* benv_new(void);
bval* benv_get(benv* e, bval* k);
void benv_put(benv* e, bval* k, bval* v);
void benv_del(benv* e);
void benv_add_builtin(benv* e, char* name, bbuiltin fn);
void benv_add_builtins(benv* e);

// forward declare functions
bval* bval_num(double num);
bval* bval_err(char* err);
bval* bval_sym(char* sym);
bval* bval_sexpr(void);
bval* bval_qexpr(void);
bval* bval_read(mpc_ast_t* tree);
bval* bval_read_num(mpc_ast_t* tree);
bval* bval_add(bval* parent, bval* child);
bval* bval_eval(benv* e, bval* v);
bval* bval_take(bval* v, int i);
bval* bval_pop(bval* v, int i);
bval* bval_join(bval* x, bval* y);
bval* bval_eval_sexpr(benv* e, bval* v);
void bval_del(bval* v);
void bval_print(bval* v);
void bval_println(bval* v);
void bval_expr_print(bval* v, char open, char close);

bval* builtin_op(benv* e, bval* v, char* op);
bval* builtin_def(benv* e, bval* a);

bval* builtin_head(benv* e, bval* a);
bval* builtin_tail(benv* e, bval* a);
bval* builtin_eval(benv* e, bval* a);
bval* builtin_join(benv* e, bval* a);
bval* builtin_list(benv* e, bval* a);
bval* builtin_cons(benv* e, bval* a);
bval* builtin_init(benv* e, bval* a);
bval* builtin_len(benv* e, bval* a);

bval* builtin_add(benv* e, bval* a);
bval* builtin_sub(benv* e, bval* a);
bval* builtin_mul(benv* e, bval* a);
bval* builtin_div(benv* e, bval* a);
bval* builtin_mod(benv* e, bval* a);

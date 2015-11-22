#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include "mpc.h"


// veriadic macro
#define ASSERT(arg, cond, fmt, ...) \
  if (!(cond)) { \
    bval* err = bval_err(fmt, ##__VA_ARGS__); \
    bval_del(arg); \
    return err; \
  }


struct bval;
struct benv;
typedef struct bval bval;
typedef struct benv benv;

// builtin function pointer
typedef bval*(*bbuiltin)(benv*, bval*);

// environment
struct benv {
  benv* parent;
  int count;
  char** syms;
  bval** vals;
};

// blisp value
struct bval {
  int type;
  int count;

  char* err;
  char* sym;
  char* str;
  double num;

  bbuiltin builtin;
  benv* env;
  bval* formals;
  bval* body;

  struct bval** cell;
};

// bval types
enum {
  BVAL_NUM,
  BVAL_ERR,
  BVAL_SEXPR,
  BVAL_QEXPR,
  BVAL_FUN,
  BVAL_SYM,
  BVAL_STR
};

benv* benv_new(void);
bval* benv_get(benv* e, bval* k);
benv* benv_copy(benv* e);
void benv_put(benv* e, bval* k, bval* v);
void benv_del(benv* e);
void benv_add_builtin(benv* e, char* name, bbuiltin fn);
void benv_add_builtins(benv* e);
void benv_def(benv* e, bval* k, bval* v);

bval* bval_num(double num);
bval* bval_err(char* fmt, ...);
bval* bval_sym(char* sym);
bval* bval_str(char* str);
bval* bval_sexpr(void);
bval* bval_qexpr(void);
bval* bval_fun(bbuiltin fn, char* name);
bval* bval_lambda(bval* formals, bval* body);

bval* bval_read(mpc_ast_t* tree);
bval* bval_read_num(mpc_ast_t* tree);
bval* bval_read_str(mpc_ast_t* tree);

bval* bval_add(bval* parent, bval* child);
bval* bval_eval(benv* e, bval* v);
bval* bval_take(bval* v, int i);
bval* bval_pop(bval* v, int i);
bval* bval_join(bval* x, bval* y);
bval* bval_eval_sexpr(benv* e, bval* v);
bval* bval_call(benv* e, bval* f, bval* a);
bval* bval_copy(bval* v);

int bval_eq(bval* x, bval* y);

void bval_del(bval* v);
void bval_print(bval* v);
void bval_println(bval* v);
void bval_expr_print(bval* v, char open, char close);
void bval_str_print(bval* v);

char* btype_name(int type);

bval* builtin_op(benv* e, bval* v, char* op);
bval* builtin_ord(benv* e, bval* v, char* op);
bval* builtin_def(benv* e, bval* a);
bval* builtin_let(benv* e, bval* a);
bval* builtin_lambda(benv* e, bval* a);
bval* builtin_var(benv* e, bval* a, char* fn);
bval* builtin_cmp(benv* e, bval* a, char* op);
bval* builtin_type(benv* e, bval* a);

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
bval* builtin_not(benv* e, bval* a);

bval* builtin_if(benv* e, bval* a);
bval* builtin_lt(benv* e, bval* a);
bval* builtin_gt(benv* e, bval* a);
bval* builtin_le(benv* e, bval* a);
bval* builtin_ge(benv* e, bval* a);
bval* builtin_eq(benv* e, bval* a);
bval* builtin_ne(benv* e, bval* a);

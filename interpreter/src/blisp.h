// blisp AST node
typedef struct bval {
  int type;
  int count;

  char* err;
  char* sym;

  long num;

  // recursive list of child nodes
  struct bval** cell;
} bval;

// bval types
enum {
  BVAL_NUM,
  BVAL_ERR,
  BVAL_SEXPR,
  BVAL_SYM
};

// forward declare functions
bval* bval_num(long num);
bval* bval_err(char* err);
bval* bval_sym(char* sym);
bval* bval_sexpr(void);
bval* bval_read(mpc_ast_t* tree);
bval* bval_read_num(mpc_ast_t* tree);
bval* bval_add(bval* parent, bval* child);
bval* bval_eval(bval* v);
bval* bval_take(bval* v, int i);
bval* bval_pop(bval* v, int i);
bval* bval_eval_sexpr(bval* v);
bval* builin_op(bval* v, char* op);

void bval_del(bval* v);
void bval_print(bval* v);
void bval_println(bval* v);
void bval_expr_print(bval* v, char open, char close);

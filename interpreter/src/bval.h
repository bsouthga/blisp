/**
 * blisp AST node constructors
 */
bval* bval_num(double num) {
  bval* v = malloc(sizeof(bval));
  v->type = BVAL_NUM;
  v->num = num;
  return v;
}
bval* bval_err(char* err) {
  bval* v = malloc(sizeof(bval));
  v->type = BVAL_ERR;
  v->err = malloc(strlen(err) + 1);
  strcpy(v->err, err);
  return v;
}
bval* bval_sym(char* sym) {
  bval* v = malloc(sizeof(bval));
  v->type = BVAL_SYM;
  // allocate memory size = length of string + 1 for null terminator
  v->sym = malloc(strlen(sym) + 1);
  strcpy(v->sym, sym);
  return v;
}
bval* bval_sexpr(void) {
  bval* v = malloc(sizeof(bval));
  v->type = BVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}
bval* bval_qexpr(void) {
  bval* v = malloc(sizeof(bval));
  v->type = BVAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}


bval* bval_take(bval* v, int i) {
  bval* x = bval_pop(v, i);
  bval_del(v);
  return x;
}


bval* bval_pop(bval* v, int i) {
  bval* x = v->cell[i];

  memmove(
    &v->cell[i],      // pointer address to element i
    &v->cell[i+1],
    sizeof(bval*) * (v->count - i - 1)
  );

  v->count--;
  v->cell = realloc(v->cell, sizeof(bval*) * v->count);

  return x;
}


bval* bval_join(bval* x, bval* y) {
  // append children of y onto children of x
  while (y->count) x = bval_add(x, bval_pop(y, 0));
  bval_del(y);
  return x;
}


/**
 * blisp AST node destructor
 */
void bval_del(bval* v) {

  switch (v->type) {
    case BVAL_NUM: break; // no property pointers for BVAL_NUM
    case BVAL_ERR: free(v->err); break;
    case BVAL_SYM: free(v->sym); break;

    case BVAL_QEXPR:
    case BVAL_SEXPR:
      // deallocate child nodes
      for (int i = 0; i < v->count; i++) {
        bval_del(v->cell[i]);
      }
      free(v->cell);
      break;
  }

  // deallocate pointer to bval struct itself
  free(v);
}


/**
 * Number parser
 */
bval* bval_read_num(mpc_ast_t* tree) {
  errno = 0; // special variable indicating error flags
  double num = strtod(tree->contents, NULL);
  return errno != ERANGE
    ? bval_num(num)
    : bval_err("invald number");
}


/**
 * Add child node to parent
 */
bval* bval_add(bval* parent, bval* child) {
  parent->count++;
  parent->cell = realloc(parent->cell, sizeof(bval*) * parent->count);
  parent->cell[parent->count - 1] = child;
  return parent;
}


/**
 * Read ast into bvals
 */
bval* bval_read(mpc_ast_t* tree) {

  if (strstr(tree->tag, "number")) return bval_read_num(tree);
  if (strstr(tree->tag, "symbol")) return bval_sym(tree->contents);



  // if we're at the root of the AST (the prompt) -> return void expr;
  bval* x = NULL;
  if (strcmp(tree->tag, ">") == 0) x = bval_sexpr();
  if (strstr(tree->tag, "sexpr"))  x = bval_sexpr();
  if (strstr(tree->tag, "qexpr"))  x = bval_qexpr();

  // fill out children of x
  for (int i = 0; i < tree->children_num; i++) {
    mpc_ast_t* child = tree->children[i];

    if (strcmp(child->contents, "(") == 0) continue;
    if (strcmp(child->contents, ")") == 0) continue;
    if (strcmp(child->contents, "{") == 0) continue;
    if (strcmp(child->contents, "}") == 0) continue;
    if (strcmp(child->tag, "regex")  == 0) continue;

    x = bval_add(x, bval_read(child));
  }

  return x;
}

/**
 * Transform sexpr for evaluation
 */
bval* bval_eval(bval* v) {
  if (v->type == BVAL_SEXPR) return bval_eval_sexpr(v);
  return v;
}

bval* bval_eval_sexpr(bval* v) {

  // eval children first
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = bval_eval(v->cell[i]);
    if (v->cell[i]->type == BVAL_ERR) return bval_take(v, i);
  }

  if (v->count == 0) return v;
  if (v->count == 1) return bval_take(v, 0);

  bval* f = bval_pop(v, 0);
  if (f->type != BVAL_SYM) {
    printf("Type is not symbol!");
    bval_del(f);
    bval_del(v);
    return bval_err("S-expression does not start with symbol!");
  }

  // evaluate builtin function
  bval* result = builtin(v, f->sym);
  bval_del(f);
  return result;
}

/**
 * Printing
 */
void bval_print(bval* v) {
  switch (v->type) {
    case BVAL_NUM:   printf("%lf", v->num);        break;
    case BVAL_ERR:   printf("Error: %s", v->err);  break;
    case BVAL_SYM:   printf("%s", v->sym);         break;
    case BVAL_SEXPR: bval_expr_print(v, '(', ')'); break;
    case BVAL_QEXPR: bval_expr_print(v, '{', '}'); break;
  }
}
void bval_println(bval* v) {
  bval_print(v);
  putchar('\n');
}
void bval_expr_print(bval* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    bval_print(v->cell[i]);
    if (i != (v->count - 1)) putchar(' ');
  }
  putchar(close);
}

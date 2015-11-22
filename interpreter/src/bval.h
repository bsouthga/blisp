/**
 * blisp AST node constructors
 */
bval* bval_num(double num) {
  bval* v = malloc(sizeof(bval));
  v->type = BVAL_NUM;
  v->num = num;
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
bval* bval_fun(bbuiltin fn, char* name) {
  bval* v = malloc(sizeof(bval));
  v->type = BVAL_FUN;
  v->builtin = fn;
  v->sym = name;
  return v;
}
bval* bval_lambda(bval* formals, bval* body) {
  bval* v = malloc(sizeof(bval));
  v->type = BVAL_FUN;
  v->builtin = NULL;

  // new scope for function
  v->env = benv_new();

  v->formals = formals;
  v->body = body;
  return v;
}

// veriadic error message function
bval* bval_err(char* fmt, ...) {
  bval* v = malloc(sizeof(bval));
  v->type = BVAL_ERR;

  // create varargs list
  va_list va;
  va_start(va, fmt);

  v->err = malloc(512);

  // printf with fmt and arguments
  vsnprintf(v->err, 511, fmt, va);

  // shrink error memory to size used
  v->err = realloc(v->err, strlen(v->err) + 1);

  va_end(va);

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
 * Call a function in an environment, with arguments
 */
bval* bval_call(benv* e, bval* f, bval* a) {
  if (f->builtin) return f->builtin(e, a);

  int given = a->count;
  int total = f->formals->count;

  while (a->count) {

    if (f->formals->count == 0) {
      bval_del(a);
      return bval_err(
        "Function passed too many arguments. "
        "Expected %i, Got %i.",
        total, given
      );
    }

    bval* sym = bval_pop(f->formals, 0);
    bval* val = bval_pop(a, 0);

    // bind a copy of the val to the functions environment
    benv_put(f->env, sym, val);

    bval_del(sym);
    bval_del(val);
  }

  bval_del(a);

  // if all the functions parameters have been bound to arguments,
  // evaluate the function and return a result
  if (f->formals->count == 0) {

    f->env->parent = e;

    // evaluate the body of the function
    return builtin_eval(f->env,
      // copy the q-expression representing the function body into an s-expression,
      // using the environment of the function as context (with the newly found variables)
      bval_add(bval_sexpr(), bval_copy(f->body))
    );
  } else {
    // otherwise... curry the function
    return bval_copy(f);
  }
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

    case BVAL_FUN:
      if (!v->builtin) {
        benv_del(v->env);
        bval_del(v->formals);
        bval_del(v->body);
      }
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
bval* bval_eval(benv* e, bval* v) {
  if (v->type == BVAL_SYM) {
    bval* x = benv_get(e, v);
    bval_del(v);
    return x;
  }

  if (v->type == BVAL_SEXPR) return bval_eval_sexpr(e, v);
  return v;
}

bval* bval_eval_sexpr(benv* e, bval* v) {

  // eval children first
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = bval_eval(e, v->cell[i]);
    if (v->cell[i]->type == BVAL_ERR) return bval_take(v, i);
  }

  if (v->count == 0) return v;
  if (v->count == 1) return bval_take(v, 0);

  bval* f = bval_pop(v, 0);
  if (f->type != BVAL_FUN) {
    bval* err = bval_err(
      "S-expression starts with incorrect type!"
      "Given type %s, Expected type %s",
      btype_name(f->type), btype_name(BVAL_FUN)
    );
    bval_del(f);
    bval_del(v);
    return err;
  }

  bval* result = bval_call(e, f, v);
  bval_del(f);
  return result;
}


bval* bval_copy(bval* v) {
  bval* x = malloc(sizeof(bval));
  x->type = v->type;

  switch (v->type) {
    case BVAL_FUN:
      if (v->builtin) {
        x->sym = v->sym;
        x->builtin = v->builtin;
      } else {
        x->builtin = NULL;
        x->env = benv_copy(v->env);
        x->formals = bval_copy(v->formals);
        x->body = bval_copy(v->body);
      }
      break;

    case BVAL_NUM: x->num = v->num; break;

    case BVAL_ERR:
      x->err = malloc(strlen(v->err) + 1);
      strcpy(x->err, v->err);
      break;

    case BVAL_SYM:
      x->sym = malloc(strlen(v->sym) + 1);
      strcpy(x->sym, v->sym);
      break;

    case BVAL_SEXPR:
    case BVAL_QEXPR:
      x->count = v->count;
      x->cell = malloc(sizeof(bval*) * x->count);
      for (int i = 0; i < x->count; i++) {
        x->cell[i] = bval_copy(v->cell[i]);
      }
      break;
  }

  return x;
}


/**
 * Printing
 */
void bval_print(bval* v) {
  switch (v->type) {
    case BVAL_FUN:
      if (v->builtin) {
        printf("<builtin: %s >", v->sym);
      } else {
        printf("(\\ ");
        bval_print(v->formals);
        putchar(' ');
        bval_print(v->body);
        putchar(')');
      }
      break;
    case BVAL_NUM:   printf("%lf", v->num);             break;
    case BVAL_ERR:   printf("Error: %s", v->err);       break;
    case BVAL_SYM:   printf("%s", v->sym);              break;
    case BVAL_SEXPR: bval_expr_print(v, '(', ')');      break;
    case BVAL_QEXPR: bval_expr_print(v, '{', '}');      break;
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


char* btype_name(int type) {
  switch (type) {
    case BVAL_FUN:   return "Function";
    case BVAL_NUM:   return "Number";
    case BVAL_ERR:   return "Error";
    case BVAL_SYM:   return "Symbol";
    case BVAL_SEXPR: return "S-Expression";
    case BVAL_QEXPR: return "Q-Expression";
  }
  return "Invalid";
}

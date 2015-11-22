/**
 * Error reporting macros
 */
#define ASSERT_ARG_TYPE(a, index, arg_type, name) \
  ASSERT(a, a->cell[index]->type == arg_type, \
    "Function '%s' needs type %s as argument %i, given type %s!", \
    name, \
    btype_name(arg_type), \
    index, \
    btype_name(a->cell[index]->type));

#define ASSERT_ARG_LEN(a, len, name) \
  ASSERT(a, a->count == len, \
    "Function '%s' given %i arguments, expected %i", \
    name, a->count, len);

#define ASSERT_NOT_EMPTY(a, name) \
  ASSERT(a, a->cell[0]->count != 0, \
    "Function '%s' passed empty list!", \
    name);


// math builtins
bval* builtin_add(benv* e, bval* a) { return builtin_op(e, a, "+"); }
bval* builtin_sub(benv* e, bval* a) { return builtin_op(e, a, "-"); }
bval* builtin_mul(benv* e, bval* a) { return builtin_op(e, a, "*"); }
bval* builtin_div(benv* e, bval* a) { return builtin_op(e, a, "/"); }
bval* builtin_mod(benv* e, bval* a) { return builtin_op(e, a, "%"); }

// comparator builtins
bval* builtin_lt(benv* e, bval* a) { return builtin_ord(e, a, "<"); }
bval* builtin_gt(benv* e, bval* a) { return builtin_ord(e, a, ">"); }
bval* builtin_le(benv* e, bval* a) { return builtin_ord(e, a, "<="); }
bval* builtin_ge(benv* e, bval* a) { return builtin_ord(e, a, ">="); }
bval* builtin_eq(benv* e, bval* a) { return builtin_cmp(e, a, "="); }
bval* builtin_ne(benv* e, bval* a) { return builtin_cmp(e, a, "!="); }


bval* builtin_def(benv* e, bval* a) {
  return builtin_var(e, a, "def");
}

bval* builtin_let(benv* e, bval* a) {
  return builtin_var(e, a, "let");
}


bval* builtin_var(benv* e, bval* a, char* fn) {
  ASSERT_ARG_TYPE(a, 0, BVAL_QEXPR, "def");

  bval* syms = a->cell[0];

  for (int i = 0; i < syms->count; i++) {
    ASSERT(a, syms->cell[i]->type == BVAL_SYM,
      "Function '%s' cannot define non-symbol!"
      "Got %s, Expected %s.", fn,
      btype_name(syms->cell[i]->type),
      btype_name(BVAL_SYM));
  }

  ASSERT(a, syms->count == a->count - 1,
    "Function '%s' must have symbol for each value!"
    "Got %i symbols, expected %i symbols.",
    fn, syms->count, a->count - 1);

  for (int i = 0; i < syms->count; i++) {
    if (strcmp(fn, "def") == 0) {
      benv_def(e, syms->cell[i], a->cell[i + 1]);
    }
    if (strcmp(fn, "let") == 0) {
      benv_put(e, syms->cell[i], a->cell[i + 1]);
    }
  }

  bval_del(a);

  return bval_sexpr();
}


bval* builtin_lambda(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 2, "\\");
  ASSERT_ARG_TYPE(a, 0, BVAL_QEXPR, "\\");
  ASSERT_ARG_TYPE(a, 1, BVAL_QEXPR, "\\");

  bval* arg_list = a->cell[0];

  for (int i = 0; i < arg_list->count; i++) {
    bval* arg = arg_list->cell[i];
    ASSERT(a, (arg->type == BVAL_SYM),
      "Cannot define non-symbol. Got %s, Expected %s.",
      btype_name(arg->type), btype_name(BVAL_SYM));
  }

  bval* formals = bval_pop(a, 0);
  bval* body = bval_pop(a, 0);
  bval_del(a);

  return bval_lambda(formals, body);
}


bval* builtin_cons(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 2, "cons");
  ASSERT_ARG_TYPE(a, 1, BVAL_QEXPR, "cons");

  // create a new q expression with the first arg of cons
  // as its first element
  bval* v = bval_add(bval_qexpr(), bval_pop(a, 0));

  // join the new q expression with the qexpr passed to cons
  v = bval_join(v, bval_pop(a, 0));

  bval_del(a);
  return v;
}


bval* builtin_init(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 1, "init");
  ASSERT_ARG_TYPE(a, 0, BVAL_QEXPR, "init");
  ASSERT_NOT_EMPTY(a, "init");

  bval* v = bval_take(a, 0);
  bval_del(bval_pop(v, v->count - 1));
  return v;
}


bval* builtin_len(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 1, "len");
  ASSERT_ARG_TYPE(a, 0, BVAL_QEXPR, "len");

  bval* v = bval_num((double) a->cell[0]->count);
  bval_del(a);
  return v;
}


bval* builtin_head(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 1, "head");
  ASSERT_ARG_TYPE(a, 0, BVAL_QEXPR, "head");
  ASSERT_NOT_EMPTY(a, "head");

  bval* v = bval_take(a, 0);
  // delete remaining elements
  while (v->count > 1) bval_del(bval_pop(v, 1));
  return v;
}


bval* builtin_tail(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 1, "tail");
  ASSERT_ARG_TYPE(a, 0, BVAL_QEXPR, "tail");
  ASSERT_NOT_EMPTY(a, "tail");

  bval* v = bval_take(a, 0);
  bval_del(bval_pop(v, 0));
  return v;
}


bval* builtin_list(benv* e, bval* a) {
  a->type = BVAL_QEXPR;
  return a;
}


bval* builtin_eval(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 1, "eval");
  ASSERT_ARG_TYPE(a, 0, BVAL_QEXPR, "eval");

  bval* x = bval_take(a, 0);
  x->type = BVAL_SEXPR;
  return bval_eval(e, x);
}


bval* builtin_join(benv* e, bval* a) {
  for (int i = 0; i < a->count; i++) {
    ASSERT_ARG_TYPE(a, i, BVAL_QEXPR, "join");
  }

  bval* x = bval_pop(a, 0);

  while (a->count) x = bval_join(x, bval_pop(a, 0));

  bval_del(a);
  return x;
}


bval* builtin_op(benv* e, bval* v, char* op) {

  // currently can only accept number atoms
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type != BVAL_NUM) {
      bval_del(v);
      return bval_err("Cannot operate on non-number!");
    }
  }

  bval* head = bval_pop(v, 0);

  // unary negation operator
  if (strcmp(op, "-") == 0 && v->count == 1) {
    head->num = - head->num;
  }

  while(v->count) {
    bval* next = bval_pop(v, 0);

    if (strcmp(op, "+") == 0) head->num += next->num;
    if (strcmp(op, "-") == 0) head->num -= next->num;
    if (strcmp(op, "*") == 0) head->num *= next->num;

    if (strcmp(op, "%") == 0) {
      if (next->num == 0) {
        bval_del(head);
        bval_del(next);
        return bval_err("Modulus by zero!");
      }
      head->num = fmod(head->num, next->num);
    }

    if (strcmp(op, "/") == 0) {
      if (next->num == 0) {
        bval_del(head);
        bval_del(next);
        return bval_err("Division by zero!");
      }
      head->num /= next->num;
    }

    bval_del(next);
  }

  bval_del(v);

  return head;
}


bval* builtin_if(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 3, "if");
  ASSERT_ARG_TYPE(a, 0, BVAL_NUM, "if");
  ASSERT_ARG_TYPE(a, 1, BVAL_QEXPR, "if");
  ASSERT_ARG_TYPE(a, 2, BVAL_QEXPR, "if");

  // make args eval-able
  a->cell[1]->type = BVAL_SEXPR;
  a->cell[2]->type = BVAL_SEXPR;

  bval* r = a->cell[0]->num
    ? bval_eval(e, bval_pop(a, 1))
    : bval_eval(e, bval_pop(a, 2));

  bval_del(a);
  return r;
}


bval* builtin_ord(benv* e, bval* a, char* op) {
  ASSERT_ARG_LEN(a, 2, op);
  ASSERT_ARG_TYPE(a, 0, BVAL_NUM, op);
  ASSERT_ARG_TYPE(a, 1, BVAL_NUM, op);

  int r;
  double x = a->cell[0]->num;
  double y = a->cell[1]->num;

  if (strcmp(op, "<"))  r = (x < y);
  if (strcmp(op, ">"))  r = (x > y);
  if (strcmp(op, "<=")) r = (x <= y);
  if (strcmp(op, ">=")) r = (x >= y);

  bval_del(a);
  return bval_num(r);
}

bval* builtin_cmp(benv* e, bval* a, char* op) {
  ASSERT_ARG_LEN(a, 2, op);

  int r;
  bval* x = a->cell[0];
  bval* y = a->cell[1];

  if (strcmp(op, "="))   r =  bval_eq(x, y);
  if (strcmp(op, "!="))  r = !bval_eq(x, y);

  bval_del(a);
  return bval_num(r);
}

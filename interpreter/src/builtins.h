#define ASSERT(arg, cond, m) \
  if (!(cond)) { bval_del(arg); return bval_err(m); }


bval* builtin(bval* a, char* fn) {
  if (strcmp("list",  fn) == 0) return builtin_list(a);
  if (strcmp("head",  fn) == 0) return builtin_head(a);
  if (strcmp("tail",  fn) == 0) return builtin_tail(a);
  if (strcmp("join",  fn) == 0) return builtin_join(a);
  if (strcmp("eval",  fn) == 0) return builtin_eval(a);
  if (strcmp("cons",  fn) == 0) return builtin_cons(a);
  if (strcmp("len",   fn) == 0) return builtin_len(a);
  if (strcmp("init",  fn) == 0) return builtin_init(a);
  if (strstr("+/*-%", fn))      return builtin_op(a, fn);

  bval_del(a);
  return bval_err("Unknown function!");
}


bval* builtin_cons(bval* a) {
  ASSERT(a, a->count == 2, "Function 'cons' requires 2 arguments!");
  ASSERT(a, a->cell[1]->type == BVAL_QEXPR, "Function 'cons' passed incorrect type as second argument!");

  // create a new q expression with the first arg of cons
  // as its first element
  bval* v = bval_add(bval_qexpr(), bval_pop(a, 0));

  // join the new q expression with the qexpr passed to cons
  v = bval_join(v, bval_pop(a, 0));

  bval_del(a);
  return v;
}


bval* builtin_init(bval* a) {
  ASSERT(a, a->count == 1, "Function 'init' passed too may arguments");
  ASSERT(a, a->cell[0]->type == BVAL_QEXPR, "Function 'init' passed incorrect type!");
  ASSERT(a, a->cell[0]->count != 0, "Function 'init' passed empty list!");

  bval* v = bval_take(a, 0);
  bval_del(bval_pop(v, v->count - 1));
  return v;
}


bval* builtin_len(bval* a) {
  ASSERT(a, a->count == 1, "Function 'len' passed too may arguments");
  ASSERT(a, a->cell[0]->type == BVAL_QEXPR, "Function 'len' passed incorrect type!");

  bval* v = bval_num((double) a->cell[0]->count);
  bval_del(a);
  return v;
}


bval* builtin_head(bval* a) {
  ASSERT(a, a->count == 1, "Function 'head' passed too may arguments");
  ASSERT(a, a->cell[0]->type == BVAL_QEXPR, "Function 'head' passed incorrect type!");
  ASSERT(a, a->cell[0]->count != 0, "Function 'head' passed empty list!");

  bval* v = bval_take(a, 0);
  // delete remaining elements
  while (v->count > 1) bval_del(bval_pop(v, 1));
  return v;
}


bval* builtin_tail(bval* a) {
  ASSERT(a, a->count == 1, "Function 'tail' passed too may arguments");
  ASSERT(a, a->cell[0]->type == BVAL_QEXPR, "Function 'tail' passed incorrect type!");
  ASSERT(a, a->cell[0]->count != 0, "Function 'tail' passed empty list!");

  bval* v = bval_take(a, 0);
  bval_del(bval_pop(v, 0));
  return v;
}


bval* builtin_list(bval* a) {
  a->type = BVAL_QEXPR;
  return a;
}


bval* builtin_eval(bval* a) {
  ASSERT(a, a->count == 1, "Function 'eval' passed too may arguments");
  ASSERT(a, a->cell[0]->type == BVAL_QEXPR, "Function 'eval' passed incorrect type!");

  bval* x = bval_take(a, 0);
  x->type = BVAL_SEXPR;
  return bval_eval(x);
}


bval* builtin_join(bval* a) {
  for (int i = 0; i < a->count; i++) {
    ASSERT(a, a->cell[i]->type == BVAL_QEXPR, "Function 'eval' passed incorrect type!");
  }

  bval* x = bval_pop(a, 0);

  while (a->count) x = bval_join(x, bval_pop(a, 0));

  bval_del(a);
  return x;
}


bval* builtin_op(bval* v, char* op) {

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

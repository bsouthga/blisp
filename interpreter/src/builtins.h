#define ASSERT(arg, cond, m) \
  if (!(cond)) { bval_del(arg); return bval_err(m); }


bval* builtin(bval* a, char* fn) {
  if (strcmp("list", fn) == 0) return builtin_list(a);
  if (strcmp("head", fn) == 0) return builtin_head(a);
  if (strcmp("tail", fn) == 0) return builtin_tail(a);
  if (strcmp("join", fn) == 0) return builtin_join(a);
  if (strcmp("eval", fn) == 0) return builtin_eval(a);
  if (strstr("+/*-%", fn))     return builtin_op(a, fn);
  bval_del(a);
  return bval_err("Unknown function!");
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

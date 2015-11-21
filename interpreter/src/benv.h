benv* benv_new(void) {
  benv* e = malloc(sizeof(benv));
  e->count = 0;
  e->syms = NULL;
  e->vals = NULL;
  return e;
}


void benv_del(benv* e) {
  for (int i = 0; i < e->count; i++) {
    free(e->syms[i]);
    bval_del(e->vals[i]);
  }
  free(e->syms);
  free(e->vals);
  free(e);
}


// linear scan through symbols for matching value
bval* benv_get(benv* e, bval* k) {
  for (int i = 0; i < e->count; i++) {
    if (strcmp(e->syms[i], k->sym) == 0) {
      return bval_copy(e->vals[i]);
    }
  }
  return bval_err("Unbound symbol!");
}


void benv_put(benv* e, bval* k, bval* v) {

  for (int i = 0; i < e->count; i++) {
    if (strcmp(e->syms[i], k->sym) == 0) {
      bval_del(e->vals[i]);
      e->vals[i] = bval_copy(v);
      return;
    }
  }

  // no pre-existing variable found

  e->count++;
  e->vals = realloc(e->vals, sizeof(bval*) * e->count);
  e->syms = realloc(e->syms, sizeof(char*) * e->count);

  e->vals[e->count - 1] = bval_copy(v);
  e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
  strcpy(e->syms[e->count - 1], k->sym);
}

void benv_add_builtin(benv* e, char* name, bbuiltin fn) {
  bval* k = bval_sym(name);
  bval* v = bval_fun(fn);
  benv_put(e, k, v);
  bval_del(k);
  bval_del(v);
}


void benv_add_builtins(benv* e) {

  // list methods
  benv_add_builtin(e, "list", builtin_list);
  benv_add_builtin(e, "cons", builtin_cons);
  benv_add_builtin(e, "len", builtin_len);
  benv_add_builtin(e, "head", builtin_head);
  benv_add_builtin(e, "tail", builtin_tail);
  benv_add_builtin(e, "init", builtin_init);
  benv_add_builtin(e, "eval", builtin_eval);
  benv_add_builtin(e, "join", builtin_join);

  // math methods
  benv_add_builtin(e, "+", builtin_add);
  benv_add_builtin(e, "-", builtin_sub);
  benv_add_builtin(e, "*", builtin_mul);
  benv_add_builtin(e, "/", builtin_div);
  benv_add_builtin(e, "%", builtin_mod);
}

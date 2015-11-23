benv* benv_new(void) {
  benv* e = malloc(sizeof(benv));
  e->count = 0;
  e->parent = NULL;
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

  // look in parent scope
  if (e->parent) {
    return benv_get(e->parent, k);
  } else {
    return bval_err("Unbound symbol '%s'", k->sym);
  }
}


benv* benv_copy(benv* e) {
  benv* n = malloc(sizeof(benv));

  n->parent = e->parent;
  n->count = e->count;
  n->syms = malloc(sizeof(char*) * n->count);
  n->vals = malloc(sizeof(char*) * n->count);

  for (int i = 0; i < e->count; i++) {
    n->syms[i] = malloc(strlen(e->syms[i]) + 1);
    strcpy(n->syms[i], e->syms[i]);
    n->vals[i] = bval_copy(e->vals[i]);
  }

  return n;
}

void benv_def(benv* e, bval* k, bval* v) {
  while(e->parent) e = e->parent;
  benv_put(e, k, v);
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
  bval* v = bval_fun(fn, name);
  benv_put(e, k, v);
  bval_del(k);
  bval_del(v);
}


void benv_add_builtins(benv* e) {
  // list methods
  benv_add_builtin(e, "list", builtin_list);
  benv_add_builtin(e, "cons", builtin_cons);
  benv_add_builtin(e, "len",  builtin_len);
  benv_add_builtin(e, "head", builtin_head);
  benv_add_builtin(e, "tail", builtin_tail);
  benv_add_builtin(e, "init", builtin_init);
  benv_add_builtin(e, "eval", builtin_eval);
  benv_add_builtin(e, "join", builtin_join);

  benv_add_builtin(e, "def",   builtin_def);
  benv_add_builtin(e, "var",   builtin_var);
  benv_add_builtin(e, "\\",    builtin_lambda);
  benv_add_builtin(e, "type",  builtin_type);
  benv_add_builtin(e, "load",  builtin_load);
  benv_add_builtin(e, "read",  builtin_read);
  benv_add_builtin(e, "print", builtin_print);
  benv_add_builtin(e, "show",  builtin_show);
  benv_add_builtin(e, "error", builtin_error);
  benv_add_builtin(e, "exit",  builtin_exit);
  benv_add_builtin(e, "fread", builtin_fread);
  benv_add_builtin(e, "fwrite", builtin_fwrite);


  // math methods
  benv_add_builtin(e, "+", builtin_add);
  benv_add_builtin(e, "-", builtin_sub);
  benv_add_builtin(e, "*", builtin_mul);
  benv_add_builtin(e, "/", builtin_div);
  benv_add_builtin(e, "%", builtin_mod);
  benv_add_builtin(e, "not", builtin_not);

  benv_add_builtin(e, "if", builtin_if);
  benv_add_builtin(e, "<",  builtin_lt);
  benv_add_builtin(e, ">",  builtin_gt);
  benv_add_builtin(e, "<=", builtin_le);
  benv_add_builtin(e, ">=", builtin_ge);
  benv_add_builtin(e, "=",  builtin_eq);
  benv_add_builtin(e, "!=", builtin_ne);
}

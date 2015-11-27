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


bval* builtin_env(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 1, "env");
  ASSERT_ARG_TYPE(a, 0, BVAL_NUM, "env");
  bval* show_builtins = bval_pop(a, 0);
  benv_print(e, show_builtins->num);
  bval_del(show_builtins);
  bval_del(a);
  return bval_ok();
}

bval* builtin_def(benv* e, bval* a) {
  return builtin_variable(e, a, "def");
}

bval* builtin_var(benv* e, bval* a) {
  return builtin_variable(e, a, "var");
}


bval* builtin_variable(benv* e, bval* a, char* fn) {
  ASSERT_ARG_TYPE(a, 0, BVAL_QEXPR, fn);

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
    if (strcmp(fn, "var") == 0) {
      benv_put(e, syms->cell[i], a->cell[i + 1]);
    }
  }

  bval_del(a);

  return bval_ok();
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


bval* builtin_print(benv* e, bval* a) {
  for (int i = 0; i < a->count; i++) {
    bval_print(a->cell[i]);
    putchar(' ');
  }
  putchar('\n');
  bval_del(a);
  return bval_ok();
}


bval* builtin_show(benv* e, bval* a) {
  for (int i = 0; i < a->count; i++) {
    ASSERT_ARG_TYPE(a, i, BVAL_STR, "show");
    printf("%s", a->cell[i]->str);
    putchar(' ');
  }
  putchar('\n');
  bval_del(a);
  return bval_ok();
}


bval* builtin_error(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 1, "error");
  ASSERT_ARG_TYPE(a, 0, BVAL_STR, "error");

  bval* err = bval_err(a->cell[0]->str);
  bval_del(a);
  return err;
}


bval* builtin_exit(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 1, "exit");
  ASSERT_ARG_TYPE(a, 0, BVAL_NUM, "exit");
  printf("Exiting!");
  exit(!!a->cell[0]->num);
  return bval_ok();
}


bval* builtin_fwrite(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 2, "fwrite");
  ASSERT_ARG_TYPE(a, 0, BVAL_STR, "fwrite");
  ASSERT_ARG_TYPE(a, 1, BVAL_STR, "fwrite");

  bval* file_param = bval_pop(a, 0);
  bval* out_string = bval_pop(a, 0);
  bval* x;

  FILE* input_file = fopen(file_param->str, "w");

  if (input_file) {
    fprintf(input_file, "%s", out_string->str);
    fclose(input_file);
    x = bval_num(1);
  } else {
    x = bval_err("File '%s' failed to open.", file_param->str);
  }

  bval_del(file_param);
  return x;
}


bval* builtin_fread(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 1, "fread");
  ASSERT_ARG_TYPE(a, 0, BVAL_STR, "fread");

  bval* file_param = bval_pop(a, 0);
  bval* x;

  char* file_contents;
  size_t input_file_size;

  FILE* input_file = fopen(file_param->str, "rb");
  if (input_file) {
    fseek(input_file, 0, SEEK_END);
    input_file_size = ftell(input_file);
    rewind(input_file);
    file_contents = malloc((input_file_size + 1) * (sizeof(char)));
    fread(file_contents, sizeof(char), input_file_size, input_file);
    fclose(input_file);
    file_contents[input_file_size] = '\0';
    x = bval_str(file_contents);
  } else {
    x = bval_err("File '%s' not found.", file_param->str);
  }

  bval_del(file_param);
  return x;
}


bval* builtin_load(benv* e, bval* a) {
  return bultin_load_file(e, a, "load");
}

bval* builtin_read(benv* e, bval* a) {
  return bultin_load_file(e, a, "read");
}

bval* bultin_load_file(benv* e, bval* a, char* op) {
  ASSERT_ARG_LEN(a, 1, op);
  ASSERT_ARG_TYPE(a, 0, BVAL_STR, op);

  mpc_result_t r;

  if (mpc_parse_contents(a->cell[0]->str, Blisp, &r)) {
    bval* expr = bval_read(r.output);
    mpc_ast_delete(r.output);

    if (strcmp(op, "read") == 0) {
      expr->type = BVAL_QEXPR;
      bval_del(a);
      return expr;
    };

    // pop expressions off stack and eval
    while(expr->count) {
      bval* x = bval_eval(e, bval_pop(expr, 0));

      switch (x->type) {
        case BVAL_ERR:
        case BVAL_STR:
        case BVAL_NUM:
          bval_println(x);
      }

      bval_del(x);
    }

    bval_del(expr);
    bval_del(a);

    return bval_ok();
  } else {
    char* error_msg = mpc_err_string(r.error);
    mpc_err_delete(r.error);
    bval* err = bval_err("Could not %s library due to error: %s", op, error_msg);
    free(error_msg);
    bval_del(a);
    return err;
  }
}


bval* builtin_type(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 1, "type");
  bval* x = bval_pop(a, 0);
  bval* r = bval_str(btype_name(x->type));
  bval_del(x);
  bval_del(a);
  return r;
}


bval* builtin_cons(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 2, "cons");

  bval* v;

  // switch on second arg to proceed
  switch (a->cell[1]->type) {

    case BVAL_STR:
      // for strings, join is the same as cons
      ASSERT_ARG_TYPE(a, 0, BVAL_STR, "cons");
      ASSERT(a, strlen(a->cell[0]->str) == 1,
        "Function cons takes a single character as the first argument.");
      return builtin_join(e, a);

    case BVAL_QEXPR:
      // create a new q expression with the first arg of cons
      // as its first element
      v = bval_add(bval_qexpr(), bval_pop(a, 0));
      // join the new q expression with the qexpr passed to cons
      v = bval_join(v, bval_pop(a, 0));
      break;

    default:
      v = bval_err(
        "Invalid type passed as second argument to cons. Got %s, Expected %s or %s.",
        btype_name(a->cell[0]->type), btype_name(BVAL_STR), btype_name(BVAL_QEXPR)
      );
      break;
  }

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

  bval* v;

  switch (a->cell[0]->type) {
    case BVAL_QEXPR:
      v = bval_num((double) a->cell[0]->count);
      break;

    case BVAL_STR:
      v = bval_num((double) strlen(a->cell[0]->str));
      break;

    default:
      v = bval_err(
        "Invalid type passed to len. Got %s, Expected %s or %s.",
        btype_name(a->cell[0]->type), btype_name(BVAL_STR), btype_name(BVAL_QEXPR)
      );
      break;

  }

  bval_del(a);
  return v;
}


bval* builtin_head(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 1, "head");
  ASSERT_NOT_EMPTY(a, "head");

  bval* v;

  switch (a->cell[0]->type) {

    case BVAL_QEXPR:
      v = bval_take(a, 0);
      // delete remaining elements
      while (v->count > 1) bval_del(bval_pop(v, 1));
      break;

    case BVAL_STR:
      a->cell[0]->str[1] = '\0';
      v = bval_str(a->cell[0]->str);
      bval_del(a);
      break;

    default:
      v = bval_err(
        "Invalid type passed to head. Got %s, Expected %s or %s.",
        btype_name(a->cell[0]->type), btype_name(BVAL_STR), btype_name(BVAL_QEXPR)
      );
      bval_del(a);
      break;

  }

  return v;
}


bval* builtin_tail(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 1, "tail");
  ASSERT_NOT_EMPTY(a, "tail");

  bval* v;

  switch (a->cell[0]->type) {

    case BVAL_QEXPR:
      v = bval_take(a, 0);
      bval_del(bval_pop(v, 0));
      break;

    case BVAL_STR:
      // increment string pointer
      v = bval_str(a->cell[0]->str + 1);
      bval_del(a);
      break;

    default:
      v = bval_err(
        "Invalid type passed to tail. Got %s, Expected %s or %s.",
        btype_name(a->cell[0]->type), btype_name(BVAL_STR), btype_name(BVAL_QEXPR)
      );
      bval_del(a);
      break;

  }

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

  // get first val
  bval* x = bval_pop(a, 0);

  int total_size;

  switch (x->type) {

    case BVAL_QEXPR:
      for (int i = 0; i < a->count; i++) {
        ASSERT_ARG_TYPE(a, i, BVAL_QEXPR, "join");
      }
      while (a->count) x = bval_join(x, bval_pop(a, 0));
      break;

    case BVAL_STR:
      total_size = strlen(x->str);

      for (int i = 0; i < a->count; i++) {
        ASSERT_ARG_TYPE(a, i, BVAL_STR, "join");
        total_size += strlen(a->cell[i]->str);
      }

      char* new_str = malloc(total_size + 1);

      strcpy(new_str, x->str);

      for (int i = 0; i < a->count; i++) {
        strcat(new_str, a->cell[i]->str);
      }

      free(x->str);
      x->str = malloc(strlen(new_str) + 1);
      strcpy(x->str, new_str);
      break;

    default:
      x = bval_err(
        "Invalid type passed to join. Got %s, Expected %s or %s.",
        btype_name(x->type), btype_name(BVAL_STR), btype_name(BVAL_QEXPR)
      );
  }

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
  if (strcmp(op, "-") == 0 && v->count == 0) {
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


bval* builtin_not(benv* e, bval* a) {
  ASSERT_ARG_LEN(a, 1, "not");
  ASSERT_ARG_TYPE(a, 0, BVAL_NUM, "not");
  bval* x = bval_pop(a, 0);

  x->num = x->num
    ? ((double) 0)
    : ((double) 1);

  bval_del(a);
  return x;
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

  if (strcmp(op, "=") == 0)   r =  bval_eq(x, y);
  if (strcmp(op, "!=") == 0)  r = !bval_eq(x, y);

  bval_del(a);
  return bval_num(r);
}

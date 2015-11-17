#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include "mpc.h"
#include "blisp.h"

/**
 * blisp AST node constructors
 */
bval* bval_num(long num) {
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

/**
 * blisp AST node destructor
 */
void bval_del(bval* v) {

  switch (v->type) {
    case BVAL_NUM: break; // no property pointers for BVAL_NUM
    case BVAL_ERR: free(v->err); break;
    case BVAL_SYM: free(v->sym); break;
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
  long num = strtol(tree->contents, NULL, 10);
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

  bval* result = builin_op(v, f->sym);
  bval_del(f);
  return result;
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


bval* builin_op(bval* v, char* op) {

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
    if (strcmp(op, "/") == 0) {
      if (next->num == 0) {
        bval_del(head);
        bval_del(next);
        head = bval_err("Division by zero!");
        break;
      }
      head->num /= next->num;
    }

    bval_del(next);
  }

  bval_del(v);

  return head;
}


/**
 * Printing
 */
void bval_print(bval* v) {
  switch (v->type) {
    case BVAL_NUM:   printf("%li", v->num);        break;
    case BVAL_ERR:   printf("Error: %s", v->err);  break;
    case BVAL_SYM:   printf("%s", v->sym);         break;
    case BVAL_SEXPR: bval_expr_print(v, '(', ')'); break;
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


int main(int argc, char** argv) {

  // create Parsers
  mpc_parser_t* Number    = mpc_new("number");
  mpc_parser_t* Symbol    = mpc_new("symbol");
  mpc_parser_t* Sexpr     = mpc_new("sexpr");
  mpc_parser_t* Expr      = mpc_new("expr");
  mpc_parser_t* Blisp     = mpc_new("blisp");

  mpca_lang(MPCA_LANG_DEFAULT,
    "\
        number   : /-?[0-9]+/                           ;\
        symbol   : '+' | '-' | '*' | '/'                ;\
        sexpr    : '(' <expr>* ')'                      ;\
        expr     : <number> | <symbol> | <sexpr>        ;\
        blisp    : /^/ <expr>* /$/                      ;\
    ",
    Number, Symbol, Sexpr, Expr, Blisp);

  puts("blisp version 0.0.1");
  puts("press ^C to Exit\n");

  while(1) {
    mpc_result_t r;
    char* input;

    // output prompt to stdout and get input
    input = readline("blisp> ");
    add_history(input);

    if (mpc_parse("<stdin>", input, Blisp, &r)) {
      // print the AST if valid
      bval* v = bval_eval(bval_read(r.output));
      bval_println(v);
      bval_del(v);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    // free up input
    free(input);
  }

  // delete parsers
  mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Blisp);

  return 0;
}

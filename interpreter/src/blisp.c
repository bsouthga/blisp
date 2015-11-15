#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include "mpc.h"

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

  bval* x = NULL;

  // if we're at the root of the AST (the prompt) -> return void expr;
  if (strstr(tree->tag, "sexpr") || strcmp(tree->tag, ">") == 0) {
    x = bval_sexpr();
  }

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
 * Print expression
 */
void bval_print(bval* v); // forward declare for mutual recursion

void bval_expr_print(bval* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    bval_print(v->cell[i]);
    if (i != (v->count - 1)) putchar(' ');
  }
  putchar(close);
}

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


// bval eval_symbol(bval x, char* op, bval y) {
//   if (x.type == BVAL_ERR) return x;
//   if (y.type == BVAL_ERR) return y;
//
//   if (strcmp(op, "+") == 0) return bval_num(x.num + y.num);
//   if (strcmp(op, "-") == 0) return bval_num(x.num - y.num);
//   if (strcmp(op, "*") == 0) return bval_num(x.num * y.num);
//   if (strcmp(op, "/") == 0) {
//     return y.num == 0
//       ? bval_err(ERR_DIV_ZERO)
//       : bval_num(x.num / y.num);
//   }
//
//   return bval_err(ERR_BAD_OP);
// }


// bval eval(mpc_ast_t* tree) {
//
//   // if the tag is a number, parse and return
//   if (strstr(tree->tag, "number")) {
//     errno = 0;
//     long x = strtol(tree->contents, NULL, 10);
//     return errno != ERANGE
//       ? bval_num(x)
//       : bval_err(ERR_BAD_NUM);
//   }
//
//   // symbol is always second child (after "(")
//   char* op = tree->children[1]->contents;
//
//   // evaluate third child
//   bval x = eval(tree->children[2]);
//
//   // evaluate remaining children
//   int i = 3;
//   while (strstr(tree->children[i]->tag, "expr")) {
//     x = eval_symbol(x, op, eval(tree->children[i]));
//     i++;
//   }
//
//   if (strcmp(op, "-") == 0 && i == 3 && x.type == BVAL_NUM) {
//     x.num = 0 - x.num;
//   }
//
//   return x;
// }





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
      bval* v = bval_read(r.output);
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

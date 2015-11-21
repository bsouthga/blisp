#include "blisp.h"
#include "bval.h"
#include "builtins.h"

int main(int argc, char** argv) {

  // create Parsers
  mpc_parser_t* Number    = mpc_new("number");
  mpc_parser_t* Symbol    = mpc_new("symbol");
  mpc_parser_t* Sexpr     = mpc_new("sexpr");
  mpc_parser_t* Qexpr     = mpc_new("qexpr");
  mpc_parser_t* Expr      = mpc_new("expr");
  mpc_parser_t* Blisp     = mpc_new("blisp");

  mpca_lang(MPCA_LANG_DEFAULT,
    "\
        number   : /-?[0-9]+(\\.[0-9]+)?/                   ;\
        symbol   : \"list\" | \
                   \"head\" | \
                   \"tail\" | \
                   \"join\" | \
                   \"eval\" | \
                   '+' | '-' | '*' | '/' | '%'              ;\
        sexpr    : '(' <expr>* ')'                          ;\
        qexpr    : '{' <expr>* '}'                          ;\
        expr     : <number> | <symbol> | <sexpr> | <qexpr>  ;\
        blisp    : /^/ <expr>* /$/                          ;\
    ",
    Number, Symbol, Sexpr,  Qexpr, Expr, Blisp);

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
  mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Blisp);
  return 0;
}

#include "blisp.h"
#include "bval.c"
#include "benv.c"
#include "builtins.c"


// embedded parser
void eval_blisp(benv* e, char* code) {
  mpc_result_t r;

  if (mpc_parse("<stdin>", code, Blisp, &r)) {
    // print the AST if valid
    bval* v = bval_eval(e, bval_read(r.output));

    bval_println(v);
    bval_del(v);

    mpc_ast_delete(r.output);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }
}


int main(int argc, char** argv) {

  // create Parsers
  Comment   = mpc_new("comment");
  Number    = mpc_new("number");
  Symbol    = mpc_new("symbol");
  String    = mpc_new("string");
  Sexpr     = mpc_new("sexpr");
  Qexpr     = mpc_new("qexpr");
  Expr      = mpc_new("expr");
  Blisp     = mpc_new("blisp");

  mpca_lang(MPCA_LANG_DEFAULT,
    "\
        comment  : /;[^\\r\\n]*/                            ;\
        number   : /-?[0-9]+(\\.[0-9]+)?/                   ;\
        string   : /\"(\\\\.|[^\"])*\"/                     ;\
        symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&%:]+/       ;\
        sexpr    : '(' <expr>* ')'                          ;\
        qexpr    : '{' <expr>* '}'                          ;\
        expr     : <comment> | \
                   <number>  | \
                   <symbol>  | \
                   <string>  | \
                   <sexpr>   | \
                   <qexpr>                                  ;\
        blisp    : /^/ <expr>* /$/                          ;\
    ",
    Comment, Number, Symbol, String, Sexpr,  Qexpr, Expr, Blisp);

  benv* e = benv_new();
  benv_add_builtins(e);

  // eval passed files
  if (argc >= 2) {
    for (int i = 1; i < argc; i++) {

      bval* args = bval_add(
        bval_sexpr(),
        bval_str(argv[i])
      );

      bval* x = builtin_load(e, args);

      if (x->type == BVAL_ERR) bval_println(x);
      bval_del(x);
    }
  } else {
    puts("blisp version 0.0.1");
    puts("press ^C to Exit");
    puts("|_ loading prelude...");
    eval_blisp(e, "(load \"./core/prelude.blisp\")");

    while(1) {
      char* input;
      // output prompt to stdout and get input
      input = readline("blisp> ");
      add_history(input);
      // evaluate input expression
      eval_blisp(e, input);
      // free up input
      free(input);
    }
  }

  benv_del(e);

  // delete parsers
  mpc_cleanup(8, Comment, Number, Symbol, String, Sexpr, Qexpr, Expr, Blisp);
  return 0;
}

#include <stdio.h>
#include <unistd.h> // for getopt
#include <memory>
#include "lexer.h"
#include "parser2.h"
#include "ast.h"
#include "exceptions.h"
#include "treeprint.h"
#include "interp.h"
#include <iostream>
enum {
  PRINT_TOKENS,
  PRINT_AST,
  EXECUTE,
};

// The execute function orchestrates the overall program logic,
// but could throw an exception if an error occurs
int execute(int argc, char **argv) {
  // handle command line options
  int mode = EXECUTE, opt;
  while ((opt = getopt(argc, argv, "lp")) != -1) {
    switch (opt) {
    case 'l':
      mode = PRINT_TOKENS;
      break;
    case 'p':
      mode = PRINT_AST;
      break;
    default:
      RuntimeError::raise("Unknown option: %c", opt);
    }
  }

  // determine source of input

  FILE *in;
  const char *filename;

  if (optind < argc) {
    // read input from file
    filename = argv[optind];
    in = fopen(filename, "r");
    if (!in) {
      RuntimeError::raise("Could not open input file '%s'", filename);
    }
  } else {
    filename = "<stdin>";
    in = stdin;
  }

  // create the Lexer
  std::unique_ptr<Lexer> lexer(new Lexer(in, filename));

  if (mode == PRINT_TOKENS) {
    // just print the tokens
    while (lexer->peek() != nullptr) {
      Node *tok = lexer->next();
      int kind = tok->get_tag();
      std::string lexeme = tok->get_str();
      printf("%d:%s\n", kind, lexeme.c_str());
      delete tok;
    }
  } else if (mode == PRINT_AST || mode == EXECUTE) {
    // Create parser and parse the input
    std::unique_ptr<Parser2> parser2(new Parser2(lexer.release()));
    std::unique_ptr<Node> ast(parser2->parse());

    if (mode == PRINT_AST) {
      // Print a text representation of the AST
      ASTTreePrint tp;
      tp.print(ast.get());
    } else {
      // Execute the program: note that the Interpreter assumes responsibility
      // for deleting the AST
      Interpreter interp(ast.release());
      interp.analyze();
      Value result = interp.execute();
      printf("Result: %s\n", result.as_str().c_str());
    }
  }

  return 0;
}

int main(int argc, char **argv) {
  try {
    return execute(argc, argv);
  } catch (BaseException &ex) {
    if (ex.has_location()) {
      // the exception has Location information
      const Location &loc = ex.get_loc();
      fprintf(stderr, "%s:%d:%d: Error: %s\n", loc.get_srcfile().c_str(), loc.get_line(), loc.get_col(), ex.what());
    } else {
      // no Location
      fprintf(stderr, "Error: %s\n", ex.what());
    }
    return 1;
  }
}

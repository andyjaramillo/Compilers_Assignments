// Copyright (c) 2021-2022, David H. Hovemeyer <david.hovemeyer@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#include <cstdlib>
#include "context.h"
#include "ast.h"
#include "grammar_symbols.h"
#include "node.h"
#include "print_highlevel_code.h"
#include "print_lowlevel_code.h"
#include "exceptions.h"
#include "cfg.h"
#include "print_cfg.h"

void usage() {
  fprintf(stderr, "Usage: nearly_cc [options...] <filename>\n"
                  "Options:\n"
                  "  -l   print tokens\n"
                  "  -p   print parse tree\n"
                  "  -C   print CFG of high-level code\n"
                  "  -c   print CFG of low-level code\n"
                  "  -L   print CFG of high-level code with liveness info\n"
                  "  -M   print CFG of low-level code with liveness info\n"
                  "  -a   perform semantic analysis, print symbol table\n"
                  "  -h   print results of high-level code generation\n"
                  "  -o   enable code optimization\n");
  exit(1);
}

enum class Mode {
  PRINT_TOKENS,
  PRINT_PARSE_TREE,
  SEMANTIC_ANALYSIS,
  HIGHLEVEL_CODEGEN,
  PRINT_HIGHLEVEL_CFG,
  PRINT_LOWLEVEL_CFG,
  PRINT_HIGHLEVEL_CFG_LIVENESS,
  PRINT_LOWLEVEL_CFG_LIVENESS,
  COMPILE,
};

void process_source_file(const std::string &filename, Mode mode, bool optimize);

int main(int argc, char **argv) {
  if (argc < 2) {
    usage();
  }

  Mode mode = Mode::COMPILE;
  bool optimize = false;

  int index = 1;
  while (index < argc) {
    std::string arg(argv[index]);
    if (arg == "-l") {
      mode = Mode::PRINT_TOKENS;
    } else if (arg == "-p") {
      mode = Mode::PRINT_PARSE_TREE;
    } else if (arg == "-C") {
      mode = Mode::PRINT_HIGHLEVEL_CFG;
    } else if (arg == "-c") {
      mode = Mode::PRINT_LOWLEVEL_CFG;
    } else if (arg == "-L") {
      mode = Mode::PRINT_HIGHLEVEL_CFG_LIVENESS;
    } else if (arg == "-M") {
      mode = Mode::PRINT_LOWLEVEL_CFG_LIVENESS;
    } else if (arg == "-a") {
      mode = Mode::SEMANTIC_ANALYSIS;
    } else if (arg == "-h") {
      mode = Mode::HIGHLEVEL_CODEGEN;
    } else if (arg == "-o") {
      // enable code optimization
      optimize = true;
    } else {
      break;
    }
    index++;
  }

  if (index >= argc) {
    usage();
  }

  const char *filename = argv[index];
  try {
    process_source_file(filename, mode, optimize);
  } catch (BaseException &ex) {
    const Location &loc = ex.get_loc();
    if (loc.is_valid()) {
      fprintf(stderr, "%s:%d:%d:Error: %s\n", loc.get_srcfile().c_str(), loc.get_line(), loc.get_col(), ex.what());
    } else {
      fprintf(stderr, "Error: %s\n", ex.what());
    }
    exit(1);
  }

  return 0;
}

void process_source_file(const std::string &filename, Mode mode, bool optimize) {
  Context ctx;

  if (mode == Mode::PRINT_TOKENS) {
    std::vector<Node *> tokens;
    ctx.scan_tokens(filename, tokens);
    for (auto i = tokens.begin(); i != tokens.end(); ++i) {
      Node *tok = *i;
      printf("%d:%s[%s]\n", tok->get_tag(), get_grammar_symbol_name(tok->get_tag()), tok->get_str().c_str());
      delete tok;
    }
  } else {
    // Parse the input
    ctx.parse(filename);

    if (mode == Mode::PRINT_PARSE_TREE) {
      // Note that we use an ASTTreePrint object to print the parse
      // tree. That way, the parser can build either a parse tree or
      // an AST, and tree printing should work correctly.
      Node *ast = ctx.get_ast();
      ASTTreePrint ptp;
      ptp.print(ast);
    } else if (mode >= Mode::SEMANTIC_ANALYSIS) {
      // Perform semantic analysis, print symbol table
      ctx.analyze();

      if (mode >= Mode::HIGHLEVEL_CODEGEN) {
        std::unique_ptr<ModuleCollector> module_collector;

        if (mode == Mode::HIGHLEVEL_CODEGEN) {
          module_collector.reset(new PrintHighLevelCode());
        } else  if (mode == Mode::PRINT_HIGHLEVEL_CFG) {
          // print a high-level CFG for each function
          module_collector.reset(new PrintHighLevelCFG());
        } else if (mode == Mode::PRINT_LOWLEVEL_CFG) {
          // print a low-level CFG for each function
          module_collector.reset(new PrintLowLevelCFG());
        } else if (mode == Mode::PRINT_HIGHLEVEL_CFG_LIVENESS) {
          // print high-level CFG with liveness info for each function
          module_collector.reset(new PrintHighLevelCFGWithLiveness());
        } else if (mode == Mode::PRINT_LOWLEVEL_CFG_LIVENESS) {
          // print low-level CFG with liveness info for each function
          module_collector.reset(new PrintLowLevelCFGWithLiveness());
        } else {
          assert(mode == Mode::COMPILE);
          module_collector.reset(new PrintLowLevelCode());
        }

        if (mode == Mode::COMPILE || mode == Mode::PRINT_LOWLEVEL_CFG || mode == Mode::PRINT_LOWLEVEL_CFG_LIVENESS)
          ctx.lowlevel_codegen(module_collector.get(), optimize);
        else
          ctx.highlevel_codegen(module_collector.get(), optimize);
      }
    }
  }
}

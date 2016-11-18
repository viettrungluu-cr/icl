#include <stdio.h>
#include <stdlib.h>

#include <memory>
#include <vector>

#include "icl/err.h"
#include "icl/input_file.h"
#include "icl/parse_tree.h"
#include "icl/parser.h"
#include "icl/scope.h"
#include "icl/settings.h"
#include "icl/token.h"
#include "icl/tokenizer.h"

static void PanicOnError(const Err& err, const char* msg) {
  if (err.has_error()) {
    // TODO(vtl): Ugh.
    err.PrintToStdout();
    fprintf(stderr, "PANIC: %s\n", msg);
    abort();
  }
}

int main() {
  InputFile input("noname");
  input.SetContents("a = \"hello world\"\n"
                    "print(a)");

  Err err;
  std::vector<Token> tokens = Tokenizer::Tokenize(&input, &err);
  PanicOnError(err, "tokenization failed");

  std::unique_ptr<ParseNode> ast = Parser::Parse(tokens, &err);
  PanicOnError(err, "parse failed");

  Settings settings;
  Scope scope(&settings);
  ast->Execute(&scope, &err);
  PanicOnError(err, "execute failed");

  return 0;
}

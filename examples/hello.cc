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

static void PanicOnError(const icl::Err& err, const char* msg) {
  if (err.has_error()) {
    // TODO(vtl): Ugh.
    err.PrintToStdout();
    fprintf(stderr, "PANIC: %s\n", msg);
    abort();
  }
}

int main() {
  icl::InputFile input("noname");
  input.SetContents("a = \"hello world\"\n"
                    "print(a)");

  icl::Err err;
  std::vector<icl::Token> tokens = icl::Tokenizer::Tokenize(&input, &err);
  PanicOnError(err, "tokenization failed");

  std::unique_ptr<icl::ParseNode> ast = icl::Parser::Parse(tokens, &err);
  PanicOnError(err, "parse failed");

  icl::Settings settings;
  icl::Scope scope(&settings);
  ast->Execute(&scope, &err);
  PanicOnError(err, "execute failed");

  return 0;
}

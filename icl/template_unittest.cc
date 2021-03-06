// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/template.h"

#include <gtest/gtest.h>

#include <utility>

#include "icl/string_number_conversions.h"
#include "icl/test_with_scope.h"

namespace icl {
namespace {

TEST(Template, Basic) {
  TestWithScope setup;
  TestParseInput input(
      "template(\"foo\") {\n"
      "  print(item_name)\n"
      "  print(invoker.bar)\n"
      "}\n"
      "foo(\"lala\") {\n"
      "  bar = 42\n"
      "}"
  );
  ASSERT_FALSE(input.has_error());

  Err err;
  input.parsed()->Execute(setup.scope(), &err);
  ASSERT_FALSE(err.has_error()) << err.message();

  EXPECT_EQ("lala\n42\n", setup.print_output());
}

TEST(Template, UnusedItemNameShouldThrowError) {
  TestWithScope setup;
  TestParseInput input(
      "template(\"foo\") {\n"
      "  print(invoker.bar)\n"
      "}\n"
      "foo(\"lala\") {\n"
      "  bar = 42\n"
      "}");
  ASSERT_FALSE(input.has_error());

  Err err;
  input.parsed()->Execute(setup.scope(), &err);
  EXPECT_TRUE(err.has_error());
}

TEST(Template, UnusedInvokerShouldThrowError) {
  TestWithScope setup;
  TestParseInput input(
      "template(\"foo\") {\n"
      "  print(item_name)\n"
      "}\n"
      "foo(\"lala\") {\n"
      "  bar = 42\n"
      "}");
  ASSERT_FALSE(input.has_error());

  Err err;
  input.parsed()->Execute(setup.scope(), &err);
  EXPECT_TRUE(err.has_error());
}

TEST(Template, UnusedVarInInvokerShouldThrowError) {
  TestWithScope setup;
  TestParseInput input(
      "template(\"foo\") {\n"
      "  print(item_name)\n"
      "  print(invoker.bar)\n"
      "}\n"
      "foo(\"lala\") {\n"
      "  bar = 42\n"
      "  baz = [ \"foo\" ]\n"
      "}");
  ASSERT_FALSE(input.has_error());

  Err err;
  input.parsed()->Execute(setup.scope(), &err);
  EXPECT_TRUE(err.has_error());
}

// Previous versions of the template implementation would copy templates by
// value when it makes a closure. Doing a sequence of them means that every new
// one copies all previous ones, which gives a significant blow-up in memory.
// If this test doesn't crash with out-of-memory, it passed.
TEST(Template, MemoryBlowUp) {
  TestWithScope setup;
  std::string code;
  for (int i = 0; i < 100; i++)
    code += "template(\"test" + NumberToString(i) + "\") {}\n";

  TestParseInput input(std::move(code));

  Err err;
  input.parsed()->Execute(setup.scope(), &err);
  ASSERT_FALSE(input.has_error());
}

}  // namespace
}  // namespace icl

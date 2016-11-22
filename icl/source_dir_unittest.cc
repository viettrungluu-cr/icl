// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/source_dir.h"

#include <gtest/gtest.h>

#include "icl/err.h"
#include "icl/source_file.h"
#include "icl/value.h"

namespace icl {
namespace {

//FIXME
/*
TEST(SourceDir, ResolveRelativeFile) {
  Err err;
  SourceDir base("//base/");
  StringPiece source_root("/source/root");

  // Empty input is an error.
  EXPECT_TRUE(base.ResolveRelativeFile(
      Value(nullptr, std::string()), &err, source_root) == SourceFile());
  EXPECT_TRUE(err.has_error());

  // These things are directories, so should be an error.
  err = Err();
  EXPECT_TRUE(base.ResolveRelativeFile(
      Value(nullptr, "//foo/bar/"), &err, source_root) == SourceFile());
  EXPECT_TRUE(err.has_error());

  err = Err();
  EXPECT_TRUE(base.ResolveRelativeFile(
      Value(nullptr, "bar/"), &err, source_root) == SourceFile());
  EXPECT_TRUE(err.has_error());

  // Absolute paths should be passed unchanged.
  err = Err();
  EXPECT_TRUE(base.ResolveRelativeFile(
      Value(nullptr, "//foo"), &err, source_root) == SourceFile("//foo"));
  EXPECT_FALSE(err.has_error());

  EXPECT_TRUE(base.ResolveRelativeFile(
      Value(nullptr, "/foo"), &err, source_root) == SourceFile("/foo"));
  EXPECT_FALSE(err.has_error());

  // Basic relative stuff.
  EXPECT_TRUE(base.ResolveRelativeFile(
      Value(nullptr, "foo"), &err, source_root) == SourceFile("//base/foo"));
  EXPECT_FALSE(err.has_error());
  EXPECT_TRUE(base.ResolveRelativeFile(
      Value(nullptr, "./foo"), &err, source_root) == SourceFile("//base/foo"));
  EXPECT_FALSE(err.has_error());
  EXPECT_TRUE(base.ResolveRelativeFile(
      Value(nullptr, "../foo"), &err, source_root) == SourceFile("//foo"));
  EXPECT_FALSE(err.has_error());

  // If the given relative path points outside the source root, we
  // expect an absolute path.
  EXPECT_TRUE(base.ResolveRelativeFile(
          Value(nullptr, "../../foo"), &err, source_root) ==
      SourceFile("/source/foo"));
  EXPECT_FALSE(err.has_error());

  EXPECT_TRUE(base.ResolveRelativeFile(
          Value(nullptr, "//../foo"), &err, source_root) ==
      SourceFile("/source/foo"));
  EXPECT_FALSE(err.has_error());

  EXPECT_TRUE(base.ResolveRelativeFile(
          Value(nullptr, "//../root/foo"), &err, source_root) ==
      SourceFile("/source/root/foo"));
  EXPECT_FALSE(err.has_error());

  EXPECT_TRUE(base.ResolveRelativeFile(
          Value(nullptr, "//../../../foo/bar"), &err, source_root) ==
      SourceFile("/foo/bar"));
  EXPECT_FALSE(err.has_error());
}

TEST(SourceDir, ResolveRelativeDir) {
  Err err;
  SourceDir base("//base/");
  StringPiece source_root("/source/root");

  // Empty input is an error.
  EXPECT_TRUE(base.ResolveRelativeDir(
      Value(nullptr, std::string()), &err, source_root) == SourceDir());
  EXPECT_TRUE(err.has_error());

  // Absolute paths should be passed unchanged.
  err = Err();
  EXPECT_TRUE(base.ResolveRelativeDir(
      Value(nullptr, "//foo"), &err, source_root) == SourceDir("//foo/"));
  EXPECT_FALSE(err.has_error());
  EXPECT_TRUE(base.ResolveRelativeDir(
      Value(nullptr, "/foo"), &err, source_root) == SourceDir("/foo/"));
  EXPECT_FALSE(err.has_error());

  // Basic relative stuff.
  EXPECT_TRUE(base.ResolveRelativeDir(
      Value(nullptr, "foo"), &err, source_root) == SourceDir("//base/foo/"));
  EXPECT_FALSE(err.has_error());
  EXPECT_TRUE(base.ResolveRelativeDir(
      Value(nullptr, "./foo"), &err, source_root) == SourceDir("//base/foo/"));
  EXPECT_FALSE(err.has_error());
  EXPECT_TRUE(base.ResolveRelativeDir(
      Value(nullptr, "../foo"), &err, source_root) == SourceDir("//foo/"));
  EXPECT_FALSE(err.has_error());

  // If the given relative path points outside the source root, we
  // expect an absolute path.
  EXPECT_TRUE(base.ResolveRelativeDir(
          Value(nullptr, "../../foo"), &err, source_root) ==
      SourceDir("/source/foo/"));
  EXPECT_FALSE(err.has_error());
  EXPECT_TRUE(base.ResolveRelativeDir(
          Value(nullptr, "//../foo"), &err, source_root) ==
      SourceDir("/source/foo/"));
  EXPECT_FALSE(err.has_error());
  EXPECT_TRUE(base.ResolveRelativeDir(
          Value(nullptr, "//.."), &err, source_root) ==
      SourceDir("/source/"));
  EXPECT_FALSE(err.has_error());
}
*/

}  // namespace
}  // namespace icl

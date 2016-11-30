// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/filesystem_utils.h"

#include <gtest/gtest.h>

#include <string>

//FIXME
/*
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/platform_thread.h"
#include "tools/gn/target.h"
*/

namespace icl {
namespace {

//FIXME
/*
TEST(FilesystemUtils, FileExtensionOffset) {
  EXPECT_EQ(std::string::npos, FindExtensionOffset(""));
  EXPECT_EQ(std::string::npos, FindExtensionOffset("foo/bar/baz"));
  EXPECT_EQ(4u, FindExtensionOffset("foo."));
  EXPECT_EQ(4u, FindExtensionOffset("f.o.bar"));
  EXPECT_EQ(std::string::npos, FindExtensionOffset("foo.bar/"));
  EXPECT_EQ(std::string::npos, FindExtensionOffset("foo.bar/baz"));
}

TEST(FilesystemUtils, FindExtension) {
  std::string input;
  EXPECT_EQ("", FindExtension(&input).as_string());
  input = "foo/bar/baz";
  EXPECT_EQ("", FindExtension(&input).as_string());
  input = "foo.";
  EXPECT_EQ("", FindExtension(&input).as_string());
  input = "f.o.bar";
  EXPECT_EQ("bar", FindExtension(&input).as_string());
  input = "foo.bar/";
  EXPECT_EQ("", FindExtension(&input).as_string());
  input = "foo.bar/baz";
  EXPECT_EQ("", FindExtension(&input).as_string());
}

TEST(FilesystemUtils, FindFilenameOffset) {
  EXPECT_EQ(0u, FindFilenameOffset(""));
  EXPECT_EQ(0u, FindFilenameOffset("foo"));
  EXPECT_EQ(4u, FindFilenameOffset("foo/"));
  EXPECT_EQ(4u, FindFilenameOffset("foo/bar"));
}

TEST(FilesystemUtils, RemoveFilename) {
  std::string s;

  RemoveFilename(&s);
  EXPECT_STREQ("", s.c_str());

  s = "foo";
  RemoveFilename(&s);
  EXPECT_STREQ("", s.c_str());

  s = "/";
  RemoveFilename(&s);
  EXPECT_STREQ("/", s.c_str());

  s = "foo/bar";
  RemoveFilename(&s);
  EXPECT_STREQ("foo/", s.c_str());

  s = "foo/bar/baz.cc";
  RemoveFilename(&s);
  EXPECT_STREQ("foo/bar/", s.c_str());
}

TEST(FilesystemUtils, FindDir) {
  std::string input;
  EXPECT_EQ("", FindDir(&input));
  input = "/";
  EXPECT_EQ("/", FindDir(&input));
  input = "foo/";
  EXPECT_EQ("foo/", FindDir(&input));
  input = "foo/bar/baz";
  EXPECT_EQ("foo/bar/", FindDir(&input));
}

TEST(FilesystemUtils, FindLastDirComponent) {
  SourceDir empty;
  EXPECT_EQ("", FindLastDirComponent(empty));

  SourceDir root("/");
  EXPECT_EQ("", FindLastDirComponent(root));

  SourceDir srcroot("//");
  EXPECT_EQ("", FindLastDirComponent(srcroot));

  SourceDir regular1("//foo/");
  EXPECT_EQ("foo", FindLastDirComponent(regular1));

  SourceDir regular2("//foo/bar/");
  EXPECT_EQ("bar", FindLastDirComponent(regular2));
}

TEST(FilesystemUtils, EnsureStringIsInOutputDir) {
  SourceDir output_dir("//out/Debug/");

  // Some outside.
  Err err;
  EXPECT_FALSE(EnsureStringIsInOutputDir(output_dir, "//foo", nullptr, &err));
  EXPECT_TRUE(err.has_error());
  err = Err();
  EXPECT_FALSE(
      EnsureStringIsInOutputDir(output_dir, "//out/Debugit", nullptr, &err));
  EXPECT_TRUE(err.has_error());

  // Some inside.
  err = Err();
  EXPECT_TRUE(
      EnsureStringIsInOutputDir(output_dir, "//out/Debug/", nullptr, &err));
  EXPECT_FALSE(err.has_error());
  EXPECT_TRUE(
      EnsureStringIsInOutputDir(output_dir, "//out/Debug/foo", nullptr, &err));
  EXPECT_FALSE(err.has_error());

  // Pattern but no template expansions are allowed.
  EXPECT_FALSE(EnsureStringIsInOutputDir(output_dir, "{{source_gen_dir}}",
                                         nullptr, &err));
  EXPECT_TRUE(err.has_error());
}
*/

TEST(FilesystemUtils, IsPathAbsolute) {
  EXPECT_TRUE(IsPathAbsolute("/foo/bar"));
  EXPECT_TRUE(IsPathAbsolute("/"));
  EXPECT_FALSE(IsPathAbsolute(""));
  EXPECT_FALSE(IsPathAbsolute("//"));
  EXPECT_FALSE(IsPathAbsolute("//foo/bar"));
}

TEST(FilesystemUtils, MakeAbsolutePathRelativeIfPossible) {
  std::string dest;

  EXPECT_TRUE(MakeAbsolutePathRelativeIfPossible("/base", "/base/foo/", &dest));
  EXPECT_EQ("//foo/", dest);
  EXPECT_TRUE(MakeAbsolutePathRelativeIfPossible("/base", "/base/foo", &dest));
  EXPECT_EQ("//foo", dest);
  EXPECT_TRUE(MakeAbsolutePathRelativeIfPossible("/base/", "/base/foo/",
                                                 &dest));
  EXPECT_EQ("//foo/", dest);

  EXPECT_FALSE(MakeAbsolutePathRelativeIfPossible("/base", "/ba", &dest));
  EXPECT_FALSE(MakeAbsolutePathRelativeIfPossible("/base", "/notbase/foo",
                                                  &dest));
}

TEST(FilesystemUtils, NormalizePath) {
  std::string input;

  NormalizePath(&input);
  EXPECT_EQ("", input);

  input = "foo/bar.txt";
  NormalizePath(&input);
  EXPECT_EQ("foo/bar.txt", input);

  input = ".";
  NormalizePath(&input);
  EXPECT_EQ("", input);

  input = "..";
  NormalizePath(&input);
  EXPECT_EQ("..", input);

  input = "foo//bar";
  NormalizePath(&input);
  EXPECT_EQ("foo/bar", input);

  input = "//foo";
  NormalizePath(&input);
  EXPECT_EQ("//foo", input);

  input = "foo/..//bar";
  NormalizePath(&input);
  EXPECT_EQ("bar", input);

  input = "foo/../../bar";
  NormalizePath(&input);
  EXPECT_EQ("../bar", input);

  input = "/../foo";  // Don't go above the root dir.
  NormalizePath(&input);
  EXPECT_EQ("/foo", input);

  input = "//../foo";  // Don't go above the root dir.
  NormalizePath(&input);
  EXPECT_EQ("//foo", input);

  input = "../foo";
  NormalizePath(&input);
  EXPECT_EQ("../foo", input);

  input = "..";
  NormalizePath(&input);
  EXPECT_EQ("..", input);

  input = "./././.";
  NormalizePath(&input);
  EXPECT_EQ("", input);

  input = "../../..";
  NormalizePath(&input);
  EXPECT_EQ("../../..", input);

  input = "../";
  NormalizePath(&input);
  EXPECT_EQ("../", input);

  // Trailing slashes should get preserved.
  input = "//foo/bar/";
  NormalizePath(&input);
  EXPECT_EQ("//foo/bar/", input);

  // Go above and outside of the source root.
  input = "//../foo";
  NormalizePath(&input, "/source/root");
  EXPECT_EQ("/source/foo", input);

  input = "//../";
  NormalizePath(&input, "/source/root");
  EXPECT_EQ("/source/", input);

  input = "//../foo.txt";
  NormalizePath(&input, "/source/root");
  EXPECT_EQ("/source/foo.txt", input);

  input = "//../foo/bar/";
  NormalizePath(&input, "/source/root");
  EXPECT_EQ("/source/foo/bar/", input);

  // Go above and back into the source root. This should return a system-
  // absolute path. We could arguably return this as a source-absolute path,
  // but that would require additional handling to account for a rare edge
  // case.
  input = "//../root/foo";
  NormalizePath(&input, "/source/root");
  EXPECT_EQ("/source/root/foo", input);

  input = "//../root/foo/bar/";
  NormalizePath(&input, "/source/root");
  EXPECT_EQ("/source/root/foo/bar/", input);

  // Stay inside the source root
  input = "//foo/bar";
  NormalizePath(&input, "/source/root");
  EXPECT_EQ("//foo/bar", input);

  input = "//foo/bar/";
  NormalizePath(&input, "/source/root");
  EXPECT_EQ("//foo/bar/", input);

  // The path should not go above the system root.
  input = "//../../../../../foo/bar";
  NormalizePath(&input, "/source/root");
  EXPECT_EQ("/foo/bar", input);

  // Test when the source root is the system root.
  input = "//../foo/bar/";
  NormalizePath(&input, "/");
  EXPECT_EQ("/foo/bar/", input);

  input = "//../";
  NormalizePath(&input, "/");
  EXPECT_EQ("/", input);

  input = "//../foo.txt";
  NormalizePath(&input, "/");
  EXPECT_EQ("/foo.txt", input);
}

//FIXME
/*
TEST(FilesystemUtils, RebasePath) {
  StringPiece source_root("/source/root");

  // Degenerate case.
  EXPECT_EQ(".", RebasePath("//", SourceDir("//"), source_root));
  EXPECT_EQ(".", RebasePath("//foo/bar/", SourceDir("//foo/bar/"),
                            source_root));

  // Going up the tree.
  EXPECT_EQ("../foo", RebasePath("//foo", SourceDir("//bar/"), source_root));
  EXPECT_EQ("../foo/", RebasePath("//foo/", SourceDir("//bar/"), source_root));
  EXPECT_EQ("../../foo", RebasePath("//foo", SourceDir("//bar/moo"),
                                    source_root));
  EXPECT_EQ("../../foo/", RebasePath("//foo/", SourceDir("//bar/moo"),
                                     source_root));

  // Going down the tree.
  EXPECT_EQ("foo/bar", RebasePath("//foo/bar", SourceDir("//"), source_root));
  EXPECT_EQ("foo/bar/", RebasePath("//foo/bar/", SourceDir("//"),
                                   source_root));

  // Going up and down the tree.
  EXPECT_EQ("../../foo/bar", RebasePath("//foo/bar", SourceDir("//a/b/"),
                                        source_root));
  EXPECT_EQ("../../foo/bar/", RebasePath("//foo/bar/", SourceDir("//a/b/"),
                                         source_root));

  // Sharing prefix.
  EXPECT_EQ("foo", RebasePath("//a/foo", SourceDir("//a/"), source_root));
  EXPECT_EQ("foo/", RebasePath("//a/foo/", SourceDir("//a/"), source_root));
  EXPECT_EQ("foo", RebasePath("//a/b/foo", SourceDir("//a/b/"), source_root));
  EXPECT_EQ("foo/", RebasePath("//a/b/foo/", SourceDir("//a/b/"),
                               source_root));
  EXPECT_EQ("foo/bar", RebasePath("//a/b/foo/bar", SourceDir("//a/b/"),
                                  source_root));
  EXPECT_EQ("foo/bar/", RebasePath("//a/b/foo/bar/", SourceDir("//a/b/"),
                                   source_root));

  // One could argue about this case. Since the input doesn't have a slash it
  // would normally not be treated like a directory and we'd go up, which is
  // simpler. However, since it matches the output directory's name, we could
  // potentially infer that it's the same and return "." for this.
  EXPECT_EQ("../bar", RebasePath("//foo/bar", SourceDir("//foo/bar/"),
                                 source_root));

  // Check when only |input| is system-absolute
  EXPECT_EQ("foo", RebasePath("/source/root/foo", SourceDir("//"),
                              StringPiece("/source/root")));
  EXPECT_EQ("foo/", RebasePath("/source/root/foo/", SourceDir("//"),
                               StringPiece("/source/root")));
  EXPECT_EQ("../../builddir/Out/Debug",
            RebasePath("/builddir/Out/Debug", SourceDir("//"),
                       StringPiece("/source/root")));
  EXPECT_EQ("../../../builddir/Out/Debug",
            RebasePath("/builddir/Out/Debug", SourceDir("//"),
                       StringPiece("/source/root/foo")));
  EXPECT_EQ("../../../builddir/Out/Debug/",
            RebasePath("/builddir/Out/Debug/", SourceDir("//"),
                       StringPiece("/source/root/foo")));
  EXPECT_EQ("../../path/to/foo",
            RebasePath("/path/to/foo", SourceDir("//"),
                       StringPiece("/source/root")));
  EXPECT_EQ("../../../path/to/foo",
            RebasePath("/path/to/foo", SourceDir("//a"),
                       StringPiece("/source/root")));
  EXPECT_EQ("../../../../path/to/foo",
            RebasePath("/path/to/foo", SourceDir("//a/b"),
                       StringPiece("/source/root")));

  // Check when only |dest_dir| is system-absolute.
  EXPECT_EQ(".",
            RebasePath("//", SourceDir("/source/root"),
                       StringPiece("/source/root")));
  EXPECT_EQ("foo",
            RebasePath("//foo", SourceDir("/source/root"),
                       StringPiece("/source/root")));
  EXPECT_EQ("../foo",
            RebasePath("//foo", SourceDir("/source/root/bar"),
                       StringPiece("/source/root")));
  EXPECT_EQ("../../../source/root/foo",
            RebasePath("//foo", SourceDir("/other/source/root"),
                       StringPiece("/source/root")));
  EXPECT_EQ("../../../../source/root/foo",
            RebasePath("//foo", SourceDir("/other/source/root/bar"),
                       StringPiece("/source/root")));

  // Check when |input| and |dest_dir| are both system-absolute. Also,
  // in this case |source_root| is never used so set it to a dummy
  // value.
  EXPECT_EQ("foo",
            RebasePath("/source/root/foo", SourceDir("/source/root"),
                       StringPiece("/x/y/z")));
  EXPECT_EQ("foo/",
            RebasePath("/source/root/foo/", SourceDir("/source/root"),
                       StringPiece("/x/y/z")));
  EXPECT_EQ("../../builddir/Out/Debug",
            RebasePath("/builddir/Out/Debug",SourceDir("/source/root"),
                       StringPiece("/x/y/z")));
  EXPECT_EQ("../../../builddir/Out/Debug",
            RebasePath("/builddir/Out/Debug", SourceDir("/source/root/foo"),
                       StringPiece("/source/root/foo")));
  EXPECT_EQ("../../../builddir/Out/Debug/",
            RebasePath("/builddir/Out/Debug/", SourceDir("/source/root/foo"),
                       StringPiece("/source/root/foo")));
  EXPECT_EQ("../../path/to/foo",
            RebasePath("/path/to/foo", SourceDir("/source/root"),
                       StringPiece("/x/y/z")));
  EXPECT_EQ("../../../path/to/foo",
            RebasePath("/path/to/foo", SourceDir("/source/root/a"),
                       StringPiece("/x/y/z")));
  EXPECT_EQ("../../../../path/to/foo",
            RebasePath("/path/to/foo", SourceDir("/source/root/a/b"),
                       StringPiece("/x/y/z")));
}

TEST(FilesystemUtils, DirectoryWithNoLastSlash) {
  EXPECT_EQ("", DirectoryWithNoLastSlash(SourceDir()));
  EXPECT_EQ("/.", DirectoryWithNoLastSlash(SourceDir("/")));
  EXPECT_EQ("//.", DirectoryWithNoLastSlash(SourceDir("//")));
  EXPECT_EQ("//foo", DirectoryWithNoLastSlash(SourceDir("//foo/")));
  EXPECT_EQ("/bar", DirectoryWithNoLastSlash(SourceDir("/bar/")));
}

TEST(FilesystemUtils, SourceDirForPath) {
  base::FilePath root("/source/foo/");
  EXPECT_EQ("/foo/bar/", SourceDirForPath(root,
            base::FilePath("/foo/bar/")).value());
  EXPECT_EQ("/", SourceDirForPath(root,
            base::FilePath("/")).value());
  EXPECT_EQ("//", SourceDirForPath(root,
            base::FilePath("/source/foo")).value());
  EXPECT_EQ("//bar/", SourceDirForPath(root,
            base::FilePath("/source/foo/bar/")).value());
  EXPECT_EQ("//bar/baz/", SourceDirForPath(root,
            base::FilePath("/source/foo/bar/baz/")).value());

  // Should be case-sensitive.
  EXPECT_EQ("/SOURCE/foo/bar/", SourceDirForPath(root,
            base::FilePath("/SOURCE/foo/bar/")).value());

  // Empty source dir.
  base::FilePath empty;
  EXPECT_EQ("/source/foo/",
            SourceDirForPath(empty, base::FilePath("/source/foo")).value());
}
*/

}  // namespace
}  // namespace icl

// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_FILESYSTEM_UTILS_H_
#define ICL_FILESYSTEM_UTILS_H_

#include <stddef.h>

#include <string>

//#include "base/files/file_path.h"
#include "icl/string_piece.h"
//#include "tools/gn/settings.h"
//#include "tools/gn/target.h"

namespace icl {

class Err;
class Location;
class ParseNode;
class SourceDir;
class Value;

// Extensions -----------------------------------------------------------------

// Returns the index of the extension (character after the last dot not after a
// slash). Returns std::string::npos if not found. Returns path.size() if the
// file ends with a dot.
size_t FindExtensionOffset(const std::string& path);

// Returns a string piece pointing into the input string identifying the
// extension. Note that the input pointer must outlive the output.
StringPiece FindExtension(const std::string* path);

// Filename parts -------------------------------------------------------------

// Returns the offset of the character following the last slash, or
// 0 if no slash was found. Returns path.size() if the path ends with a slash.
// Note that the input pointer must outlive the output.
size_t FindFilenameOffset(const std::string& path);

// Returns a string piece pointing into the input string identifying the
// file name (following the last slash, including the extension). Note that the
// input pointer must outlive the output.
StringPiece FindFilename(const std::string* path);

// Like FindFilename but does not include the extension.
StringPiece FindFilenameNoExtension(const std::string* path);

// Removes everything after the last slash. The last slash, if any, will be
// preserved.
void RemoveFilename(std::string* path);

// Path parts -----------------------------------------------------------------

// Returns a string piece pointing into the input string identifying the
// directory name of the given path, including the last slash. Note that the
// input pointer must outlive the output.
StringPiece FindDir(const std::string* path);

// Returns the substring identifying the last component of the dir, or the
// empty substring if none. For example "//foo/bar/" -> "bar".
StringPiece FindLastDirComponent(const SourceDir& dir);

// ----------------------------------------------------------------------------

// Returns true if the input string is absolute. Double-slashes at the
// beginning are treated as source-relative paths. On Windows, this handles
// paths of both the native format: "C:/foo" and ours "/C:/foo"
bool IsPathAbsolute(const StringPiece& path);

// Returns true if the input string is source-absolute. Source-absolute
// paths begin with two forward slashes and resolve as if they are
// relative to the source root.
bool IsPathSourceAbsolute(const StringPiece& path);

// Given an absolute path, checks to see if is it is inside the source root.
// If it is, fills a source-absolute path into the given output and returns
// true. If it isn't, clears the dest and returns false.
//
// The source_root should be a base::FilePath converted to UTF-8. On Windows,
// it should begin with a "C:/" rather than being our SourceFile's style
// ("/C:/"). The source root can end with a slash or not.
//
// Note that this does not attempt to normalize slashes in the output.
bool MakeAbsolutePathRelativeIfPossible(const StringPiece& source_root,
                                        const StringPiece& path,
                                        std::string* dest);

// Collapses "." and sequential "/"s and evaluates "..". |path| may be
// system-absolute, source-absolute, or relative. If |path| is source-absolute
// and |source_root| is non-empty, |path| may be system absolute after this
// function returns, if |path| references the filesystem outside of
// |source_root| (ex. path = "//.."). In this case on Windows, |path| will have
// a leading slash. Otherwise, |path| will retain its relativity. |source_root|
// must not end with a slash.
void NormalizePath(std::string* path,
                   const StringPiece& source_root = StringPiece());

/*
// Takes a path, |input|, and makes it relative to the given directory
// |dest_dir|. Both inputs may be source-relative (e.g. begins with
// with "//") or may be absolute.
//
// If supplied, the |source_root| parameter is the absolute path to
// the source root and not end in a slash. Unless you know that the
// inputs are always source relative, this should be supplied.
std::string RebasePath(
    const std::string& input,
    const SourceDir& dest_dir,
    const StringPiece& source_root = StringPiece());

// Returns the given directory with no terminating slash at the end, such that
// appending a slash and more stuff will produce a valid path.
//
// If the directory refers to either the source or system root, we'll append
// a "." so this remains valid.
std::string DirectoryWithNoLastSlash(const SourceDir& dir);

// Returns the "best" SourceDir representing the given path. If it's inside the
// given source_root, a source-relative directory will be returned (e.g.
// "//foo/bar.cc". If it's outside of the source root or the source root is
// empty, a system-absolute directory will be returned.
SourceDir SourceDirForPath(const base::FilePath& source_root,
                           const base::FilePath& path);

// Like SourceDirForPath but returns the SourceDir representing the current
// directory.
SourceDir SourceDirForCurrentDirectory(const base::FilePath& source_root);
*/


}  // namespace icl

#endif  // ICL_FILESYSTEM_UTILS_H_

// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/filesystem_utils.h"

#include <assert.h>

//FIXME
//#include <algorithm>

//FIXME
//#include "base/files/file_util.h"
//#include "base/logging.h"
//#include "base/strings/string_util.h"
//#include "base/strings/utf_string_conversions.h"
//#include "tools/gn/location.h"
//#include "tools/gn/settings.h"
//#include "tools/gn/source_dir.h"

namespace icl {

namespace {

enum DotDisposition {
  // The given dot is just part of a filename and is not special.
  NOT_A_DIRECTORY,

  // The given dot is the current directory.
  DIRECTORY_CUR,

  // The given dot is the first of a double dot that should take us up one.
  DIRECTORY_UP
};

// When we find a dot, this function is called with the character following
// that dot to see what it is. The return value indicates what type this dot is
// (see above). This code handles the case where the dot is at the end of the
// input.
//
// |*consumed_len| will contain the number of characters in the input that
// express what we found.
DotDisposition ClassifyAfterDot(const std::string& path,
                                size_t after_dot,
                                size_t* consumed_len) {
  if (after_dot == path.size()) {
    // Single dot at the end.
    *consumed_len = 1;
    return DIRECTORY_CUR;
  }
  if (path[after_dot] == '/') {
    // Single dot followed by a slash.
    *consumed_len = 2;  // Consume the slash
    return DIRECTORY_CUR;
  }

  if (path[after_dot] == '.') {
    // Two dots.
    if (after_dot + 1 == path.size()) {
      // Double dot at the end.
      *consumed_len = 2;
      return DIRECTORY_UP;
    }
    if (path[after_dot + 1] == '/') {
      // Double dot folowed by a slash.
      *consumed_len = 3;
      return DIRECTORY_UP;
    }
  }

  // The dots are followed by something else, not a directory.
  *consumed_len = 1;
  return NOT_A_DIRECTORY;
}

//FIXME
/*
// A wrapper around FilePath.GetComponents that works the way we need. This is
// not super efficient since it does some O(n) transformations on the path. If
// this is called a lot, we might want to optimize.
std::vector<base::FilePath::StringType> GetPathComponents(
    const base::FilePath& path) {
  std::vector<base::FilePath::StringType> result;
  path.GetComponents(&result);

  if (result.empty())
    return result;

  // GetComponents will preserve the "/" at the beginning, which confuses us.
  // We don't expect to have relative paths in this function.
  // Don't use IsSeparator since we always want to allow backslashes.
  if (result[0] == FILE_PATH_LITERAL("/") ||
      result[0] == FILE_PATH_LITERAL("\\"))
    result.erase(result.begin());

  return result;
}

// Provides the equivalent of == for filesystem strings, trying to do
// approximately the right thing with case.
bool FilesystemStringsEqual(const base::FilePath::StringType& a,
                            const base::FilePath::StringType& b) {
  // Assume case-sensitive filesystems on non-Windows.
  return a == b;
}

// Helper function for computing subdirectories in the build directory
// corresponding to absolute paths. This will try to resolve the absolute
// path as a source-relative path first, and otherwise it creates a
// special subdirectory for absolute paths to keep them from colliding with
// other generated sources and outputs.
void AppendFixedAbsolutePathSuffix(const BuildSettings* build_settings,
                                   const SourceDir& source_dir,
                                   OutputFile* result) {
  const std::string& build_dir = build_settings->build_dir().value();

  if (base::StartsWith(source_dir.value(), build_dir,
                       base::CompareCase::SENSITIVE)) {
    size_t build_dir_size = build_dir.size();
    result->value().append(&source_dir.value()[build_dir_size],
                           source_dir.value().size() - build_dir_size);
  } else {
    result->value().append("ABS_PATH");
    const std::string& src_dir_value = source_dir.value();
    result->value().append(src_dir_value);
  }
}
*/

}  // namespace

//FIXME
/*
std::string FilePathToUTF8(const base::FilePath::StringType& str) {
  return str;
}

base::FilePath UTF8ToFilePath(const StringPiece& sp) {
  return base::FilePath(sp.as_string());
}
*/

//FIXME
/*
size_t FindExtensionOffset(const std::string& path) {
  for (int i = static_cast<int>(path.size()); i >= 0; i--) {
    if (path[i] == '/')
      break;
    if (path[i] == '.')
      return i + 1;
  }
  return std::string::npos;
}

StringPiece FindExtension(const std::string* path) {
  size_t extension_offset = FindExtensionOffset(*path);
  if (extension_offset == std::string::npos)
    return StringPiece();
  return StringPiece(&path->data()[extension_offset],
                     path->size() - extension_offset);
}

size_t FindFilenameOffset(const std::string& path) {
  for (int i = static_cast<int>(path.size()) - 1; i >= 0; i--) {
    if (path[i] == '/')
      return i + 1;
  }
  return 0;  // No filename found means everything was the filename.
}

StringPiece FindFilename(const std::string* path) {
  size_t filename_offset = FindFilenameOffset(*path);
  if (filename_offset == 0)
    return StringPiece(*path);  // Everything is the file name.
  return StringPiece(&(*path).data()[filename_offset],
                     path->size() - filename_offset);
}

StringPiece FindFilenameNoExtension(const std::string* path) {
  if (path->empty())
    return StringPiece();
  size_t filename_offset = FindFilenameOffset(*path);
  size_t extension_offset = FindExtensionOffset(*path);

  size_t name_len;
  if (extension_offset == std::string::npos)
    name_len = path->size() - filename_offset;
  else
    name_len = extension_offset - filename_offset - 1;

  return StringPiece(&(*path).data()[filename_offset], name_len);
}

void RemoveFilename(std::string* path) {
  path->resize(FindFilenameOffset(*path));
}

StringPiece FindDir(const std::string* path) {
  size_t filename_offset = FindFilenameOffset(*path);
  if (filename_offset == 0u)
    return StringPiece();
  return StringPiece(path->data(), filename_offset);
}

StringPiece FindLastDirComponent(const SourceDir& dir) {
  const std::string& dir_string = dir.value();

  if (dir_string.empty())
    return StringPiece();
  int cur = static_cast<int>(dir_string.size()) - 1;
  DCHECK(dir_string[cur] == '/');
  int end = cur;
  cur--;  // Skip before the last slash.

  for (; cur >= 0; cur--) {
    if (dir_string[cur] == '/')
      return StringPiece(&dir_string[cur + 1], end - cur - 1);
  }
  return StringPiece(&dir_string[0], end);
}

bool IsStringInOutputDir(const SourceDir& output_dir, const std::string& str) {
  // This check will be wrong for all proper prefixes "e.g. "/output" will
  // match "/out" but we don't really care since this is just a sanity check.
  const std::string& dir_str = output_dir.value();
  return str.compare(0, dir_str.length(), dir_str) == 0;
}

bool EnsureStringIsInOutputDir(const SourceDir& output_dir,
                               const std::string& str,
                               const ParseNode* origin,
                               Err* err) {
  if (IsStringInOutputDir(output_dir, str))
    return true;  // Output directory is hardcoded.

  *err = Err(origin, "File is not inside output directory.",
      "The given file should be in the output directory. Normally you would "
      "specify\n\"$target_out_dir/foo\" or "
      "\"$target_gen_dir/foo\". I interpreted this as\n\""
      + str + "\".");
  return false;
}
*/

bool IsPathAbsolute(const StringPiece& path) {
  if (path.empty())
    return false;

  if (path[0] != '/')
    return false;  // Doesn't begin with a slash, is relative.

  // Double forward slash at the beginning means source-relative (we don't
  // allow backslashes for denoting this).
  if (path.size() > 1 && path[1] == '/')
    return false;

  return true;
}

//FIXME
/*
bool IsPathSourceAbsolute(const StringPiece& path) {
  return (path.size() >= 2 && path[0] == '/' && path[1] == '/');
}
*/

bool MakeAbsolutePathRelativeIfPossible(const StringPiece& source_root,
                                        const StringPiece& path,
                                        std::string* dest) {
  assert(IsPathAbsolute(source_root));
  assert(IsPathAbsolute(path));

  dest->clear();

  if (source_root.size() > path.size())
    return false;  // The source root is longer: the path can never be inside.

  // On non-Windows this is easy. Since we know both are absolute, just do a
  // prefix check.
  if (path.substr(0, source_root.size()) == source_root) {
    // The base may or may not have a trailing slash, so skip all slashes from
    // the path after our prefix match.
    size_t first_after_slash = source_root.size();
    while (first_after_slash < path.size() && path[first_after_slash] == '/')
      first_after_slash++;

    dest->assign("//");  // Result is source root relative.
    dest->append(&path.data()[first_after_slash],
                 path.size() - first_after_slash);
    return true;
  }
  return false;
}

void NormalizePath(std::string* path, const StringPiece& source_root) {
  char* pathbuf = path->empty() ? nullptr : &(*path)[0];

  // top_index is the first character we can modify in the path. Anything
  // before this indicates where the path is relative to.
  size_t top_index = 0;
  bool is_relative = true;
  if (!path->empty() && pathbuf[0] == '/') {
    is_relative = false;

    if (path->size() > 1 && pathbuf[1] == '/') {
      // Two leading slashes, this is a path into the source dir.
      top_index = 2;
    } else {
      // One leading slash, this is a system-absolute path.
      top_index = 1;
    }
  }

  size_t dest_i = top_index;
  for (size_t src_i = top_index; src_i < path->size(); /* nothing */) {
    if (pathbuf[src_i] == '.') {
      if (src_i == 0 || pathbuf[src_i - 1] == '/') {
        // Slash followed by a dot, see if it's something special.
        size_t consumed_len;
        switch (ClassifyAfterDot(*path, src_i + 1, &consumed_len)) {
          case NOT_A_DIRECTORY:
            // Copy the dot to the output, it means nothing special.
            pathbuf[dest_i++] = pathbuf[src_i++];
            break;
          case DIRECTORY_CUR:
            // Current directory, just skip the input.
            src_i += consumed_len;
            break;
          case DIRECTORY_UP:
            // Back up over previous directory component. If we're already
            // at the top, preserve the "..".
            if (dest_i > top_index) {
              // The previous char was a slash, remove it.
              dest_i--;
            }

            if (dest_i == top_index) {
              if (is_relative) {
                // We're already at the beginning of a relative input, copy the
                // ".." and continue. We need the trailing slash if there was
                // one before (otherwise we're at the end of the input).
                pathbuf[dest_i++] = '.';
                pathbuf[dest_i++] = '.';
                if (consumed_len == 3)
                  pathbuf[dest_i++] = '/';

                // This also makes a new "root" that we can't delete by going
                // up more levels.  Otherwise "../.." would collapse to
                // nothing.
                top_index = dest_i;
              } else if (top_index == 2 && !source_root.empty()) {
                // |path| was passed in as a source-absolute path. Prepend
                // |source_root| to make |path| absolute. |source_root| must not
                // end with a slash unless we are at root.
                assert(source_root.size() == 1u ||
                       source_root[source_root.size() - 1u] != '/');
                size_t source_root_len = source_root.size();

                path->insert(0, source_root.data(), source_root_len);

                // |path| is now absolute, so |top_index| is 1. |dest_i| and
                // |src_i| should be incremented to keep the same relative
                // position. Comsume the leading "//" by decrementing |dest_i|.
                top_index = 1;
                pathbuf = &(*path)[0];
                dest_i += source_root_len - 2;
                src_i += source_root_len;

                // Just find the previous slash or the beginning of input.
                while (dest_i > 0 && pathbuf[dest_i - 1] != '/')
                  dest_i--;
              }
              // Otherwise we're at the beginning of a system-absolute path, or
              // a source-absolute path for which we don't know the absolute
              // path. Don't allow ".." to go up another level, and just eat it.
            } else {
              // Just find the previous slash or the beginning of input.
              while (dest_i > 0 && pathbuf[dest_i - 1] != '/')
                dest_i--;
            }
            src_i += consumed_len;
        }
      } else {
        // Dot not preceeded by a slash, copy it literally.
        pathbuf[dest_i++] = pathbuf[src_i++];
      }
    } else if (pathbuf[src_i] == '/') {
      if (src_i > 0 && pathbuf[src_i - 1] == '/') {
        // Two slashes in a row, skip over it.
        src_i++;
      } else {
        // Just one slash, copy it, normalizing to foward slash.
        pathbuf[dest_i] = '/';
        dest_i++;
        src_i++;
      }
    } else {
      // Input nothing special, just copy it.
      pathbuf[dest_i++] = pathbuf[src_i++];
    }
  }
  path->resize(dest_i);
}

//FIXME
/*
//FIXME this should be static/anonymized
std::string MakeRelativePath(const std::string& input,
                             const std::string& dest) {
  std::string ret;

  // Skip the common prefixes of the source and dest as long as they end in
  // a [back]slash.
  size_t common_prefix_len = 0;
  size_t max_common_length = std::min(input.size(), dest.size());
  for (size_t i = common_prefix_len; i < max_common_length; i++) {
    if (input[i] == '/' && dest[i] == '/')
      common_prefix_len = i + 1;
    else if (input[i] != dest[i])
      break;
  }

  // Invert the dest dir starting from the end of the common prefix.
  for (size_t i = common_prefix_len; i < dest.size(); i++) {
    if (dest[i] == '/')
      ret.append("../");
  }

  // Append any remaining unique input.
  ret.append(&input[common_prefix_len], input.size() - common_prefix_len);

  // If the result is still empty, the paths are the same.
  if (ret.empty())
    ret.push_back('.');

  return ret;
}

std::string RebasePath(const std::string& input,
                       const SourceDir& dest_dir,
                       const StringPiece& source_root) {
  std::string ret;
  DCHECK(source_root.empty() || !source_root.ends_with("/"));

  bool input_is_source_path = (input.size() >= 2 &&
                               input[0] == '/' && input[1] == '/');

  if (!source_root.empty() &&
      (!input_is_source_path || !dest_dir.is_source_absolute())) {
    std::string input_full;
    std::string dest_full;
    if (input_is_source_path) {
      source_root.AppendToString(&input_full);
      input_full.push_back('/');
      input_full.append(input, 2, std::string::npos);
    } else {
      input_full.append(input);
    }
    if (dest_dir.is_source_absolute()) {
      source_root.AppendToString(&dest_full);
      dest_full.push_back('/');
      dest_full.append(dest_dir.value(), 2, std::string::npos);
    } else {
      dest_full.append(dest_dir.value());
    }
    bool remove_slash = false;
    if (input_full.empty() || input_full.back() != '/') {
      input_full.push_back('/');
      remove_slash = true;
    }
    ret = MakeRelativePath(input_full, dest_full);
    if (remove_slash && ret.size() > 1)
      ret.resize(ret.size() - 1);
    return ret;
  }

  ret = MakeRelativePath(input, dest_dir.value());
  return ret;
}

std::string DirectoryWithNoLastSlash(const SourceDir& dir) {
  std::string ret;

  if (dir.value().empty()) {
    // Just keep input the same.
  } else if (dir.value() == "/") {
    ret.assign("/.");
  } else if (dir.value() == "//") {
    ret.assign("//.");
  } else {
    ret.assign(dir.value());
    ret.resize(ret.size() - 1);
  }
  return ret;
}

SourceDir SourceDirForPath(const base::FilePath& source_root,
                           const base::FilePath& path) {
  std::vector<base::FilePath::StringType> source_comp =
      GetPathComponents(source_root);
  std::vector<base::FilePath::StringType> path_comp =
      GetPathComponents(path);

  // See if path is inside the source root by looking for each of source root's
  // components at the beginning of path.
  bool is_inside_source;
  if (path_comp.size() < source_comp.size() || source_root.empty()) {
    // Too small to fit.
    is_inside_source = false;
  } else {
    is_inside_source = true;
    for (size_t i = 0; i < source_comp.size(); i++) {
      if (!FilesystemStringsEqual(source_comp[i], path_comp[i])) {
        is_inside_source = false;
        break;
      }
    }
  }

  std::string result_str;
  size_t initial_path_comp_to_use;
  if (is_inside_source) {
    // Construct a source-relative path beginning in // and skip all of the
    // shared directories.
    result_str = "//";
    initial_path_comp_to_use = source_comp.size();
  } else {
    // Not inside source code, construct a system-absolute path.
    result_str = "/";
    initial_path_comp_to_use = 0;
  }

  for (size_t i = initial_path_comp_to_use; i < path_comp.size(); i++) {
    result_str.append(FilePathToUTF8(path_comp[i]));
    result_str.push_back('/');
  }
  return SourceDir(result_str);
}

SourceDir SourceDirForCurrentDirectory(const base::FilePath& source_root) {
  base::FilePath cd;
  base::GetCurrentDirectory(&cd);
  return SourceDirForPath(source_root, cd);
}

std::string GetOutputSubdirName(const Label& toolchain_label, bool is_default) {
  // The default toolchain has no subdir.
  if (is_default)
    return std::string();

  // For now just assume the toolchain name is always a valid dir name. We may
  // want to clean up the in the future.
  return toolchain_label.name() + "/";
}

bool ContentsEqual(const base::FilePath& file_path, const std::string& data) {
  // Compare file and stream sizes first. Quick and will save us some time if
  // they are different sizes.
  int64_t file_size;
  if (!base::GetFileSize(file_path, &file_size) ||
      static_cast<size_t>(file_size) != data.size()) {
    return false;
  }

  std::string file_data;
  file_data.resize(file_size);
  if (!base::ReadFileToString(file_path, &file_data))
    return false;

  return file_data == data;
}

bool WriteFileIfChanged(const base::FilePath& file_path,
                        const std::string& data,
                        Err* err) {
  if (ContentsEqual(file_path, data))
    return true;

  return WriteFile(file_path, data, err);
}

bool WriteFile(const base::FilePath& file_path, const std::string& data,
               Err* err) {
  // Create the directory if necessary.
  if (!base::CreateDirectory(file_path.DirName())) {
    if (err) {
      *err =
          Err(Location(), "Unable to create directory.",
              "I was using \"" + FilePathToUTF8(file_path.DirName()) + "\".");
    }
    return false;
  }

  int size = static_cast<int>(data.size());
  bool write_success = base::WriteFile(file_path, data.c_str(), size) == size;
  if (!write_success && err) {
    *err = Err(Location(), "Unable to write file.",
               "I was writing \"" + FilePathToUTF8(file_path) + "\".");
  }

  return write_success;
}
*/

}  // namespace icl

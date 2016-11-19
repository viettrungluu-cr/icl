// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/standard_out.h"

#include <assert.h>
#include <stddef.h>

#include <vector>

//FIXME
//#include "base/command_line.h"
#include "icl/string_piece.h"
//FIXME
//#include "base/strings/string_split.h"
//#include "base/strings/string_util.h"
//#include "tools/gn/switches.h"

#include <stdio.h>
#include <unistd.h>

namespace icl {

namespace {

bool initialized = false;

bool is_console = false;

bool is_markdown = false;

void EnsureInitialized() {
  if (initialized)
    return;
  initialized = true;

//FIXME
/*
  const base::CommandLine* cmdline = base::CommandLine::ForCurrentProcess();
  if (cmdline->HasSwitch(switches::kMarkdown)) {
    // Output help in Markdown's syntax, not color-highlighted.
    is_markdown = true;
  }

  if (cmdline->HasSwitch(switches::kNoColor)) {
    // Force color off.
    is_console = false;
    return;
  }

  if (cmdline->HasSwitch(switches::kColor))
    is_console = true;
  else
*/
    is_console = isatty(fileno(stdout));
}

void WriteToStdOut(const std::string& output) {
  size_t written_bytes = fwrite(output.data(), 1, output.size(), stdout);
  assert(output.size() == written_bytes);
}

void OutputMarkdownDec(TextDecoration dec) {
  // The markdown rendering turns "dim" text to italics and any
  // other colored text to bold.

  if (dec == DECORATION_DIM)
    WriteToStdOut("*");
  else if (dec != DECORATION_NONE)
    WriteToStdOut("**");
}

}  // namespace

void OutputString(const std::string& output, TextDecoration dec) {
  EnsureInitialized();
  if (is_markdown) {
    OutputMarkdownDec(dec);
  } else if (is_console) {
    switch (dec) {
      case DECORATION_NONE:
        break;
      case DECORATION_DIM:
        WriteToStdOut("\e[2m");
        break;
      case DECORATION_RED:
        WriteToStdOut("\e[31m\e[1m");
        break;
      case DECORATION_GREEN:
        WriteToStdOut("\e[32m");
        break;
      case DECORATION_BLUE:
        WriteToStdOut("\e[34m\e[1m");
        break;
      case DECORATION_YELLOW:
        WriteToStdOut("\e[33m\e[1m");
        break;
    }
  }

  std::string tmpstr = output;
  if (is_markdown && dec == DECORATION_YELLOW) {
    // https://code.google.com/p/gitiles/issues/detail?id=77
    // Gitiles will replace "--" with an em dash in non-code text.
    // Figuring out all instances of this might be difficult, but we can
    // at least escape the instances where this shows up in a heading.
//FIXME
//    base::ReplaceSubstringsAfterOffset(&tmpstr, 0, "--", "\\--");
  }
  WriteToStdOut(tmpstr.data());

  if (is_markdown) {
    OutputMarkdownDec(dec);
  } else if (is_console && dec != DECORATION_NONE) {
    WriteToStdOut("\e[0m");
  }
}

void PrintShortHelp(const std::string& line) {
  EnsureInitialized();

  size_t colon_offset = line.find(':');
  size_t first_normal = 0;
  if (colon_offset != std::string::npos) {
    OutputString("  " + line.substr(0, colon_offset), DECORATION_YELLOW);
    first_normal = colon_offset;
  }

  // See if the colon is followed by a " [" and if so, dim the contents of [ ].
  if (first_normal > 0 &&
      line.size() > first_normal + 2 &&
      line[first_normal + 1] == ' ' && line[first_normal + 2] == '[') {
    size_t begin_bracket = first_normal + 2;
    OutputString(": ");
    first_normal = line.find(']', begin_bracket);
    if (first_normal == std::string::npos)
      first_normal = line.size();
    else
      first_normal++;
    OutputString(line.substr(begin_bracket, first_normal - begin_bracket),
                 DECORATION_DIM);
  }

  OutputString(line.substr(first_normal) + "\n");
}

void PrintLongHelp(const std::string& text) {
  EnsureInitialized();

//FIXME
/*
  bool first_header = true;
  bool in_body = false;
  for (const std::string& line : base::SplitString(
           text, "\n", base::KEEP_WHITESPACE, base::SPLIT_WANT_ALL)) {
    // Check for a heading line.
    if (!line.empty() && line[0] != ' ') {
      if (is_markdown) {
        // GN's block-level formatting is converted to markdown as follows:
        // * The first heading is treated as an H2.
        // * Subsequent heading are treated as H3s.
        // * Any other text is wrapped in a code block and displayed as-is.
        //
        // Span-level formatting (the decorations) is converted inside
        // OutputString().
        if (in_body) {
          OutputString("```\n\n", DECORATION_NONE);
          in_body = false;
        }

        if (first_header) {
          OutputString("## ", DECORATION_NONE);
          first_header = false;
        } else {
          OutputString("### ", DECORATION_NONE);
        }
      }

      // Highlight up to the colon (if any).
      size_t chars_to_highlight = line.find(':');
      if (chars_to_highlight == std::string::npos)
        chars_to_highlight = line.size();

      OutputString(line.substr(0, chars_to_highlight), DECORATION_YELLOW);
      OutputString(line.substr(chars_to_highlight) + "\n");
      continue;
    } else if (is_markdown && !line.empty() && !in_body) {
      OutputString("```\n", DECORATION_NONE);
      in_body = true;
    }

    // Check for a comment.
    TextDecoration dec = DECORATION_NONE;
    for (const auto& elem : line) {
      if (elem == '#' && !is_markdown) {
        // Got a comment, draw dimmed.
        dec = DECORATION_DIM;
        break;
      } else if (elem != ' ') {
        break;
      }
    }

    OutputString(line + "\n", dec);
  }

  if (is_markdown && in_body)
    OutputString("\n```\n");
*/
}

}  // namespace icl

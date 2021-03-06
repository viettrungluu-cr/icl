// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// Copied from strings/stringpiece.cc with modifications

#include "icl/string_piece.h"

#include <limits.h>

#include <algorithm>
#include <ostream>

namespace icl {

namespace {

// For each character in characters_wanted, sets the index corresponding
// to the ASCII code of that character to 1 in table.  This is used by
// the find_.*_of methods below to tell whether or not a character is in
// the lookup table in constant time.
// The argument `table' must be an array that is large enough to hold all
// the possible values of an unsigned char.  Thus it should be be declared
// as follows:
//   bool table[UCHAR_MAX + 1]
inline void BuildLookupTable(const StringPiece& characters_wanted,
                             bool* table) {
  const size_t length = characters_wanted.length();
  const char* const data = characters_wanted.data();
  for (size_t i = 0; i < length; ++i)
    table[static_cast<unsigned char>(data[i])] = true;
}

}  // namespace

// static
const StringPiece::size_type StringPiece::npos;

StringPiece::size_type StringPiece::find(const StringPiece& s,
                                         size_type pos) const {
  if (pos > size())
    return npos;

  StringPiece::const_iterator result =
      std::search(begin() + pos, end(), s.begin(), s.end());
  const size_t xpos = static_cast<size_t>(result - begin());
  return xpos + s.size() <= size() ? xpos : npos;
}

StringPiece::size_type StringPiece::find(value_type c, size_type pos) const {
  if (pos >= size())
    return npos;

  StringPiece::const_iterator result = std::find(begin() + pos, end(), c);
  return result != end() ? static_cast<size_t>(result - begin()) : npos;
}

StringPiece::size_type StringPiece::rfind(const StringPiece& s,
                                          size_type pos) const {
  if (size() < s.size())
    return npos;

  if (s.empty())
    return std::min(size(), pos);

  StringPiece::const_iterator last =
      begin() + std::min(size() - s.size(), pos) + s.size();
  StringPiece::const_iterator result =
      std::find_end(begin(), last, s.begin(), s.end());
  return result != last ? static_cast<size_t>(result - begin()) : npos;
}

StringPiece::size_type StringPiece::rfind(value_type c, size_type pos) const {
  if (size() == 0)
    return npos;

  for (size_t i = std::min(pos, size() - 1); ;
       --i) {
    if (data()[i] == c)
      return i;
    if (i == 0)
      break;
  }
  return npos;
}

// 8-bit version using lookup table.
StringPiece::size_type StringPiece::find_first_of(const StringPiece& s,
                                                  size_type pos) const {
  if (size() == 0 || s.size() == 0)
    return npos;

  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (s.size() == 1)
    return find(s.data()[0], pos);

  bool lookup[UCHAR_MAX + 1] = { false };
  BuildLookupTable(s, lookup);
  for (size_t i = pos; i < size(); ++i) {
    if (lookup[static_cast<unsigned char>(data()[i])])
      return i;
  }
  return npos;
}

// 8-bit version using lookup table.
StringPiece::size_type StringPiece::find_first_not_of(const StringPiece& s,
                                                      size_type pos) const {
  if (size() == 0)
    return npos;
  if (s.size() == 0)
    return 0;

  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (s.size() == 1)
    return find_first_not_of(s.data()[0], pos);

  bool lookup[UCHAR_MAX + 1] = { false };
  BuildLookupTable(s, lookup);
  for (size_t i = pos; i < size(); ++i) {
    if (!lookup[static_cast<unsigned char>(data()[i])])
      return i;
  }
  return npos;
}

StringPiece::size_type StringPiece::find_first_not_of(value_type c,
                                                      size_type pos) const {
  if (size() == 0)
    return npos;

  for (; pos < size(); ++pos) {
    if (data()[pos] != c)
      return pos;
  }
  return npos;
}

// 8-bit version using lookup table.
StringPiece::size_type StringPiece::find_last_of(const StringPiece& s,
                                                 size_type pos) const {
  if (size() == 0 || s.size() == 0)
    return npos;

  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (s.size() == 1)
    return rfind(s.data()[0], pos);

  bool lookup[UCHAR_MAX + 1] = { false };
  BuildLookupTable(s, lookup);
  for (size_t i = std::min(pos, size() - 1); ; --i) {
    if (lookup[static_cast<unsigned char>(data()[i])])
      return i;
    if (i == 0)
      break;
  }
  return npos;
}

// 8-bit version using lookup table.
StringPiece::size_type StringPiece::find_last_not_of(const StringPiece& s,
                                                     size_type pos) const {
  if (size() == 0)
    return npos;

  size_t i = std::min(pos, size() - 1);
  if (s.size() == 0)
    return i;

  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (s.size() == 1)
    return find_last_not_of(s.data()[0], pos);

  bool lookup[UCHAR_MAX + 1] = { false };
  BuildLookupTable(s, lookup);
  for (; ; --i) {
    if (!lookup[static_cast<unsigned char>(data()[i])])
      return i;
    if (i == 0)
      break;
  }
  return npos;
}

StringPiece::size_type StringPiece::find_last_not_of(value_type c,
                                                     size_type pos) const {
  if (size() == 0)
    return npos;

  for (size_t i = std::min(pos, size() - 1); ; --i) {
    if (data()[i] != c)
      return i;
    if (i == 0)
      break;
  }
  return npos;
}

std::ostream& operator<<(std::ostream& o, const StringPiece& piece) {
  o.write(piece.data(), static_cast<std::streamsize>(piece.size()));
  return o;
}

}  // namespace icl

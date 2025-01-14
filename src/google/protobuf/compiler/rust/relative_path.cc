// Protocol Buffers - Google's data interchange format
// Copyright 2023 Google LLC.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google LLC. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "google/protobuf/compiler/rust/relative_path.h"

#include <iostream>
#include <string>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace rust {

std::vector<absl::string_view> RelativePath::Segments() const {
  return absl::StrSplit(this->path_, '/', absl::SkipWhitespace());
}

bool RelativePath::IsDirectory() const {
  return absl::EndsWith(this->path_, "/");
}

std::string RelativePath::Relative(const RelativePath& dest) const {
  ABSL_CHECK(!dest.IsDirectory())
      << "`dest` has to be a file path, but is a directory.";
  std::vector<absl::string_view> current_segments = this->Segments();

  if (!current_segments.empty() && !this->IsDirectory()) {
    // `this` represents a file path, skip the last segment to get its
    // directory.
    current_segments.pop_back();
  }

  std::vector<absl::string_view> dest_segments = dest.Segments();

  // Find the lowest common ancestor.
  absl::c_reverse(current_segments);
  absl::c_reverse(dest_segments);
  while (true) {
    if (current_segments.empty()) break;
    if (dest_segments.empty()) break;
    if (current_segments.back() != dest_segments.back()) break;

    current_segments.pop_back();
    dest_segments.pop_back();
  }

  // Construct the relative path in reverse order.
  std::vector<absl::string_view> result;
  result.reserve(current_segments.size() + dest_segments.size());
  // Push the segments from the `dest` to the common ancestor.
  for (const auto& segment : dest_segments) {
    result.push_back(segment);
  }
  // Push `..` from the common ancestor to the current path.
  for (int i = 0; i < current_segments.size(); ++i) {
    result.push_back("..");
  }
  absl::c_reverse(result);
  if (dest.IsDirectory()) {
    // Convince the `StrJoin` below to add a trailing `/`.
    result.push_back("");
  }
  return absl::StrJoin(result, "/");
}

}  // namespace rust
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

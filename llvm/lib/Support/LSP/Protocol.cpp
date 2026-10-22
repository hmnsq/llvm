//===--- Protocol.cpp - Language Server Protocol Implementation -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the serialization code for the LSP structs.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/LSP/Protocol.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringSet.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/JSON.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Path.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
template <typename T>
ಸ್ಥಿರ ತಾರ್ಕಿಕ mapOptOrNull(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  ಧ್ರುವ llvm::json::Object *O = Params.getAsObject();
  ಪ್ರತಿಜ್ಞಾ(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  ಯದಿ (!V || V->getAsNull())
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

ವರ್ಣ LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

ಸ್ಥಿರ ತಾರ್ಕಿಕ isWindowsPath(StringRef Path) {
  ಪ್ರತ್ಯಾಗಮ Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isNetworkPath(StringRef Path) {
  ಪ್ರತ್ಯಾಗಮ Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ shouldEscapeInURI(unsigned ವರ್ಣ C) {
  // Unreserved characters.
  ಯದಿ ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ವಿಕಲ್ಪ (C) {
  ಪಕ್ಷ '-':
  ಪಕ್ಷ '_':
  ಪಕ್ಷ '.':
  ಪಕ್ಷ '~':
  // '/' is only reserved when parsing.
  ಪಕ್ಷ '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  ಪಕ್ಷ ':':
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
ಸ್ಥಿರ ಶೂನ್ಯ percentEncode(StringRef Content, std::string &Out) {
  ಪ್ರತಿ (unsigned ವರ್ಣ C : Content) {
    ಯದಿ (shouldEscapeInURI(C)) {
      Out.ತಳ್ಳು_back('%');
      Out.ತಳ್ಳು_back(llvm::hexdigit(C / 16));
      Out.ತಳ್ಳು_back(llvm::hexdigit(C % 16));
    } ಅನ್ಯಥಾ {
      Out.ತಳ್ಳು_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ಸ್ಥಿರ std::string percentDecode(StringRef Content) {
  std::string Result;
  ಪ್ರತಿ (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    ಯದಿ (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.ತಳ್ಳು_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } ಅನ್ಯಥಾ {
      Result.ತಳ್ಳು_back(*I);
    }
  }
  ಪ್ರತ್ಯಾಗಮ Result;
}

/// Return the set containing the supported URI schemes.
ಸ್ಥಿರ StringSet<> &getSupportedSchemes() {
  ಸ್ಥಿರ StringSet<> Schemes({"file", "test"});
  ಪ್ರತ್ಯಾಗಮ Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
ಸ್ಥಿರ ತಾರ್ಕಿಕ isStructurallyValidScheme(StringRef Scheme) {
  ಯದಿ (Scheme.empty())
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (!llvm::isAlpha(Scheme[0]))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ llvm::all_of(llvm::drop_begin(Scheme), [](ವರ್ಣ C) {
    ಪ್ರತ್ಯಾಗಮ llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

ಸ್ಥಿರ llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef ಬೇರು_2 = llvm::sys::path::ಬೇರು_name(AbsolutePath);
  ಯದಿ (isNetworkPath(ಬೇರು_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = ಬೇರು_2.drop_front(2);
    AbsolutePath.consume_front(ಬೇರು_2);
  } ಅನ್ಯಥಾ ಯದಿ (isWindowsPath(ಬೇರು_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  ಯದಿ (Authority.empty() && Body.empty())
    ಪ್ರತ್ಯಾಗಮ Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  ಯದಿ (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  ಪ್ರತ್ಯಾಗಮ Uri;
}

ಸ್ಥಿರ llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  ಯದಿ (!Body.starts_with("/"))
    ಪ್ರತ್ಯಾಗಮ llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  ಯದಿ (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } ಅನ್ಯಥಾ ಯದಿ (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  ಪ್ರತ್ಯಾಗಮ std::string(Path);
}

ಸ್ಥಿರ llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  ಯದಿ (Pos == StringRef::npos)
    ಪ್ರತ್ಯಾಗಮ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  ಯದಿ (!isStructurallyValidScheme(UriScheme))
    ಪ್ರತ್ಯಾಗಮ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  ಯದಿ (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  ಯದಿ (!getSupportedSchemes().contains(UriScheme)) {
    ಪ್ರತ್ಯಾಗಮ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  ಪ್ರತ್ಯಾಗಮ getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  ಯದಿ (!FilePath)
    ಪ್ರತ್ಯಾಗಮ FilePath.takeError();
  ಪ್ರತ್ಯಾಗಮ URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  ಯದಿ (!Uri)
    ಪ್ರತ್ಯಾಗಮ Uri.takeError();
  ಪ್ರತ್ಯಾಗಮ fromURI(*Uri);
}

StringRef URIForFile::scheme() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ uri().split(':').first; }

ಶೂನ್ಯ URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().ಸೇರಿಸು(Scheme);
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, URIForFile &Result,
                         llvm::json::Path Path) {
  ಯದಿ (std::optional<StringRef> Str = ಮೌಲ್ಯ.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    ಯದಿ (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }
    Result = std::move(*ExpectedUri);
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ URIForFile &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ಧ್ರುವ URIForFile &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ Os << ಮೌಲ್ಯ.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  ಧ್ರುವ llvm::json::Object *O = ಮೌಲ್ಯ.getAsObject();
  ಯದಿ (!O) {
    Path.report("expected object");
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಯದಿ (ಧ್ರುವ llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    ಯದಿ (ಧ್ರುವ llvm::json::Object *Document_ಚಿಹ್ನೆ =
            TextDocument->getObject("documentSymbol")) {
      ಯದಿ (std::optional<ತಾರ್ಕಿಕ> HierarchicalSupport =
              Document_ಚಿಹ್ನೆ->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_ಚಿಹ್ನೆ = *HierarchicalSupport;
    }
    ಯದಿ (auto *CodeAction = TextDocument->getObject("codeAction")) {
      ಯದಿ (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = ಸತ್ಯ;
    }
  }
  ಯದಿ (auto *Window = O->getObject("window")) {
    ಯದಿ (std::optional<ತಾರ್ಕಿಕ> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಯದಿ (!O || !O.map("name", Result.name))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, TraceLevel &Result,
                         llvm::json::Path Path) {
  ಯದಿ (std::optional<StringRef> Str = ಮೌಲ್ಯ.getAsString()) {
    ಯದಿ (*Str == "off") {
      Result = TraceLevel::Off;
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
    ಯದಿ (*Str == "messages") {
      Result = TraceLevel::Messages;
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
    ಯದಿ (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಯದಿ (!O)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.ಬೇರು_Uri);
  O.map("rootPath", Result.ಬೇರು_Path);
  mapOptOrNull(ಮೌಲ್ಯ, "clientInfo", Result.clientInfo, Path);

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ TextDocumentIdentifier &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ llvm::json::Object{{"uri", ಮೌಲ್ಯ.uri}};
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ
llvm::lsp::toJSON(ಧ್ರುವ VersionedTextDocumentIdentifier &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ llvm::json::Object{
      {"uri", ಮೌಲ್ಯ.uri},
      {"version", ಮೌಲ್ಯ.version},
  };
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("line", Result.ಸಾಲು_2) &&
         O.map("character", Result.character);
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ Position &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ llvm::json::Object{
      {"line", ಮೌಲ್ಯ.ಸಾಲು_2},
      {"character", ಮೌಲ್ಯ.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ಧ್ರುವ Position &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ Os << ಮೌಲ್ಯ.ಸಾಲು_2 << ':' << ಮೌಲ್ಯ.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ Range &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ llvm::json::Object{
      {"start", ಮೌಲ್ಯ.start},
      {"end", ಮೌಲ್ಯ.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ಧ್ರುವ Range &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ Os << ಮೌಲ್ಯ.start << '-' << ಮೌಲ್ಯ.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ Location &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ llvm::json::Object{
      {"uri", ಮೌಲ್ಯ.uri},
      {"range", ಮೌಲ್ಯ.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ಧ್ರುವ Location &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ Os << ಮೌಲ್ಯ.range << '@' << ಮೌಲ್ಯ.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ fromJSON(ಮೌಲ್ಯ, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  ಪ್ರತ್ಯಾಗಮ O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) ಧ್ರುವ {
  // If there is no range, the full document changed.
  ಯದಿ (!range) {
    Contents = text;
    ಪ್ರತ್ಯಾಗಮ success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  ಯದಿ (!RangeLoc.isValid())
    ಪ್ರತ್ಯಾಗಮ failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  ಪ್ರತ್ಯಾಗಮ success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  ಪ್ರತಿ (ಧ್ರುವ auto &Change : Changes)
    ಯದಿ (failed(Change.applyTo(Contents)))
      ಪ್ರತ್ಯಾಗಮ failure();
  ಪ್ರತ್ಯಾಗಮ success();
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

ಸ್ಥಿರ llvm::StringRef toTextKind(MarkupKind Kind) {
  ವಿಕಲ್ಪ (Kind) {
  ಪಕ್ಷ MarkupKind::PlainText:
    ಪ್ರತ್ಯಾಗಮ "plaintext";
  ಪಕ್ಷ MarkupKind::Markdown:
    ಪ್ರತ್ಯಾಗಮ "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  ಪ್ರತ್ಯಾಗಮ Os << toTextKind(Kind);
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ MarkupContent &Mc) {
  ಯದಿ (Mc.ಮೌಲ್ಯ_2.empty())
    ಪ್ರತ್ಯಾಗಮ nullptr;

  ಪ್ರತ್ಯಾಗಮ llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.ಮೌಲ್ಯ_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  ಯದಿ (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  ಪ್ರತ್ಯಾಗಮ std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ Document_ಚಿಹ್ನೆ &ಚಿಹ್ನೆ) {
  llvm::json::Object Result{{"name", ಚಿಹ್ನೆ.name},
                            {"kind", static_cast<ಪೂರ್ಣಾಙ್ಕ>(ಚಿಹ್ನೆ.kind)},
                            {"range", ಚಿಹ್ನೆ.range},
                            {"selectionRange", ಚಿಹ್ನೆ.selectionRange}};

  ಯದಿ (!ಚಿಹ್ನೆ.detail.empty())
    Result["detail"] = ಚಿಹ್ನೆ.detail;
  ಯದಿ (!ಚಿಹ್ನೆ.children.empty())
    Result["children"] = ಚಿಹ್ನೆ.children;
  ಪ್ರತ್ಯಾಗಮ std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         Document_ಚಿಹ್ನೆ_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ DiagnosticRelatedInformation &Info) {
  ಪ್ರತ್ಯಾಗಮ llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(DiagnosticTag Tag) {
  ಪ್ರತ್ಯಾಗಮ static_cast<ಪೂರ್ಣಾಙ್ಕ>(Tag);
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  ಯದಿ (std::optional<int64_t> I = ಮೌಲ್ಯ.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (ಪೂರ್ಣಾಙ್ಕ)Diag.severity},
      {"message", Diag.message},
  };
  ಯದಿ (Diag.category)
    Result["category"] = *Diag.category;
  ಯದಿ (!Diag.source.empty())
    Result["source"] = Diag.source;
  ಯದಿ (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  ಯದಿ (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  ಪ್ರತ್ಯಾಗಮ std::move(Result);
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಯದಿ (!O)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪೂರ್ಣಾಙ್ಕ Severity = 0;
  ಯದಿ (!mapOptOrNull(ಮೌಲ್ಯ, "severity", Severity, Path))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  Result.severity = (DiagnosticSeverity)Severity;

  ಪ್ರತ್ಯಾಗಮ O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(ಮೌಲ್ಯ, "category", Result.category, Path) &&
         mapOptOrNull(ಮೌಲ್ಯ, "source", Result.source, Path) &&
         mapOptOrNull(ಮೌಲ್ಯ, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(ಮೌಲ್ಯ, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ PublishDiagnosticsParams &Params) {
  ಪ್ರತ್ಯಾಗಮ llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ TextEdit &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ llvm::json::Object{
      {"range", ಮೌಲ್ಯ.range},
      {"newText", ಮೌಲ್ಯ.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ಧ್ರುವ TextEdit &ಮೌಲ್ಯ) {
  Os << ಮೌಲ್ಯ.range << " => \"";
  llvm::printEscapedString(ಮೌಲ್ಯ.newText, Os);
  ಪ್ರತ್ಯಾಗಮ Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  ಯದಿ (std::optional<int64_t> Int_ಮೌಲ್ಯ = ಮೌಲ್ಯ.getAsInteger()) {
    ಯದಿ (*Int_ಮೌಲ್ಯ < static_cast<ಪೂರ್ಣಾಙ್ಕ>(CompletionItemKind::Text) ||
        *Int_ಮೌಲ್ಯ > static_cast<ಪೂರ್ಣಾಙ್ಕ>(CompletionItemKind::TypeParameter))
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    Result = static_cast<CompletionItemKind>(*Int_ಮೌಲ್ಯ);
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  ಯದಿ (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    ಪ್ರತ್ಯಾಗಮ Kind;

  // Provide some fall backs for common kinds that are close enough.
  ವಿಕಲ್ಪ (Kind) {
  ಪಕ್ಷ CompletionItemKind::Folder:
    ಪ್ರತ್ಯಾಗಮ CompletionItemKind::File;
  ಪಕ್ಷ CompletionItemKind::EnumMember:
    ಪ್ರತ್ಯಾಗಮ CompletionItemKind::Enum;
  ಪಕ್ಷ CompletionItemKind::Struct:
    ಪ್ರತ್ಯಾಗಮ CompletionItemKind::Class;
  ಸ್ವಭಾವ:
    ಪ್ರತ್ಯಾಗಮ CompletionItemKind::Text;
  }
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  ಯದಿ (ಧ್ರುವ llvm::json::Array *Array_ಮೌಲ್ಯ = ಮೌಲ್ಯ.getAsArray()) {
    ಪ್ರತಿ (size_t I = 0, E = Array_ಮೌಲ್ಯ->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      ಯದಿ (fromJSON((*Array_ಮೌಲ್ಯ)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ CompletionItem &ಮೌಲ್ಯ) {
  ಪ್ರತಿಜ್ಞಾ(!ಮೌಲ್ಯ.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", ಮೌಲ್ಯ.label}};
  ಯದಿ (ಮೌಲ್ಯ.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<ಪೂರ್ಣಾಙ್ಕ>(ಮೌಲ್ಯ.kind);
  ಯದಿ (!ಮೌಲ್ಯ.detail.empty())
    Result["detail"] = ಮೌಲ್ಯ.detail;
  ಯದಿ (ಮೌಲ್ಯ.documentation)
    Result["documentation"] = ಮೌಲ್ಯ.documentation;
  ಯದಿ (!ಮೌಲ್ಯ.sortText.empty())
    Result["sortText"] = ಮೌಲ್ಯ.sortText;
  ಯದಿ (!ಮೌಲ್ಯ.filterText.empty())
    Result["filterText"] = ಮೌಲ್ಯ.filterText;
  ಯದಿ (!ಮೌಲ್ಯ.ಸೇರಿಸು_Text.empty())
    Result["insertText"] = ಮೌಲ್ಯ.ಸೇರಿಸು_Text;
  ಯದಿ (ಮೌಲ್ಯ.ಸೇರಿಸು_Text_Format != ಸೇರಿಸು_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<ಪೂರ್ಣಾಙ್ಕ>(ಮೌಲ್ಯ.ಸೇರಿಸು_Text_Format);
  ಯದಿ (ಮೌಲ್ಯ.textEdit)
    Result["textEdit"] = *ಮೌಲ್ಯ.textEdit;
  ಯದಿ (!ಮೌಲ್ಯ.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(ಮೌಲ್ಯ.additionalTextEdits);
  }
  ಯದಿ (ಮೌಲ್ಯ.deprecated)
    Result["deprecated"] = ಮೌಲ್ಯ.deprecated;
  ಪ್ರತ್ಯಾಗಮ std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ಧ್ರುವ CompletionItem &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ Os << ಮೌಲ್ಯ.label << " - " << toJSON(ಮೌಲ್ಯ);
}

ತಾರ್ಕಿಕ llvm::lsp::operator<(ಧ್ರುವ CompletionItem &Lhs,
                          ಧ್ರುವ CompletionItem &Rhs) {
  ಪ್ರತ್ಯಾಗಮ (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ CompletionList &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ llvm::json::Object{
      {"isIncomplete", ಮೌಲ್ಯ.isIncomplete},
      {"items", llvm::json::Array(ಮೌಲ್ಯ.ವಸ್ತುಗಳು)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪೂರ್ಣಾಙ್ಕ TriggerKind;
  ಯದಿ (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(ಮೌಲ್ಯ, "triggerCharacter", Result.triggerCharacter, Path))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         CompletionParams &Result, llvm::json::Path Path) {
  ಯದಿ (!fromJSON(ಮೌಲ್ಯ, static_cast<TextDocumentPositionParams &>(Result), Path))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (ಧ್ರುವ llvm::json::ಮೌಲ್ಯ *Context = ಮೌಲ್ಯ.getAsObject()->get("context"))
    ಪ್ರತ್ಯಾಗಮ fromJSON(*Context, Result.context, Path.field("context"));
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ ParameterInformation &ಮೌಲ್ಯ) {
  ಪ್ರತಿಜ್ಞಾ((ಮೌಲ್ಯ.labelOffsets || !ಮೌಲ್ಯ.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  ಯದಿ (ಮೌಲ್ಯ.labelOffsets)
    Result["label"] = llvm::json::Array(
        {ಮೌಲ್ಯ.labelOffsets->first, ಮೌಲ್ಯ.labelOffsets->second});
  ಅನ್ಯಥಾ
    Result["label"] = ಮೌಲ್ಯ.labelString;
  ಯದಿ (!ಮೌಲ್ಯ.documentation.empty())
    Result["documentation"] = ಮೌಲ್ಯ.documentation;
  ಪ್ರತ್ಯಾಗಮ std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ SignatureInformation &ಮೌಲ್ಯ) {
  ಪ್ರತಿಜ್ಞಾ(!ಮೌಲ್ಯ.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", ಮೌಲ್ಯ.label},
      {"parameters", llvm::json::Array(ಮೌಲ್ಯ.parameters)},
  };
  ಯದಿ (!ಮೌಲ್ಯ.documentation.empty())
    Result["documentation"] = ಮೌಲ್ಯ.documentation;
  ಪ್ರತ್ಯಾಗಮ std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ಧ್ರುವ SignatureInformation &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ Os << ಮೌಲ್ಯ.label << " - " << toJSON(ಮೌಲ್ಯ);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ SignatureHelp &ಮೌಲ್ಯ) {
  ಪ್ರತಿಜ್ಞಾ(ಮೌಲ್ಯ.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  ಪ್ರತಿಜ್ಞಾ(ಮೌಲ್ಯ.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  ಪ್ರತ್ಯಾಗಮ llvm::json::Object{
      {"activeSignature", ಮೌಲ್ಯ.activeSignature},
      {"activeParameter", ಮೌಲ್ಯ.activeParameter},
      {"signatures", llvm::json::Array(ಮೌಲ್ಯ.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ DocumentLink &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ llvm::json::Object{
      {"range", ಮೌಲ್ಯ.range},
      {"target", ಮೌಲ್ಯ.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ InlayHint &ಮೌಲ್ಯ) {
  ಪ್ರತ್ಯಾಗಮ llvm::json::Object{{"position", ಮೌಲ್ಯ.position},
                            {"kind", (ಪೂರ್ಣಾಙ್ಕ)ಮೌಲ್ಯ.kind},
                            {"label", ಮೌಲ್ಯ.label},
                            {"paddingLeft", ಮೌಲ್ಯ.padding_ಎಡ},
                            {"paddingRight", ಮೌಲ್ಯ.padding_ಬಲ}};
}
ತಾರ್ಕಿಕ llvm::lsp::operator==(ಧ್ರುವ InlayHint &Lhs, ಧ್ರುವ InlayHint &Rhs) {
  ಪ್ರತ್ಯಾಗಮ std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
ತಾರ್ಕಿಕ llvm::lsp::operator<(ಧ್ರುವ InlayHint &Lhs, ಧ್ರುವ InlayHint &Rhs) {
  ಪ್ರತ್ಯಾಗಮ std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind ಮೌಲ್ಯ) {
  ವಿಕಲ್ಪ (ಮೌಲ್ಯ) {
  ಪಕ್ಷ InlayHintKind::Parameter:
    ಪ್ರತ್ಯಾಗಮ Os << "parameter";
  ಪಕ್ಷ InlayHintKind::Type:
    ಪ್ರತ್ಯಾಗಮ Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಯದಿ (!O || !O.map("diagnostics", Result.diagnostics))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  O.map("only", Result.only);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಧ್ರುವ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, Path);
  ಪ್ರತ್ಯಾಗಮ O && O.map("changes", Result.changes);
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ WorkspaceEdit &ಮೌಲ್ಯ) {
  llvm::json::Object FileChanges;
  ಪ್ರತಿ (auto &Change : ಮೌಲ್ಯ.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  ಪ್ರತ್ಯಾಗಮ llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ಧ್ರುವ llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
ಧ್ರುವ llvm::StringLiteral CodeAction::kRefactor = "refactor";
ಧ್ರುವ llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ CodeAction &ಮೌಲ್ಯ) {
  llvm::json::Object CodeAction{{"title", ಮೌಲ್ಯ.title}};
  ಯದಿ (ಮೌಲ್ಯ.kind)
    CodeAction["kind"] = *ಮೌಲ್ಯ.kind;
  ಯದಿ (ಮೌಲ್ಯ.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*ಮೌಲ್ಯ.diagnostics);
  ಯದಿ (ಮೌಲ್ಯ.isPreferred)
    CodeAction["isPreferred"] = ಸತ್ಯ;
  ಯದಿ (ಮೌಲ್ಯ.edit)
    CodeAction["edit"] = *ಮೌಲ್ಯ.edit;
  ಪ್ರತ್ಯಾಗಮ std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<ಪೂರ್ಣಾಙ್ಕ>(Params.type)},
      {"message", Params.message},
  };
  ಯದಿ (Params.actions)
    Out["actions"] = *Params.actions;
  ಪ್ರತ್ಯಾಗಮ Out;
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::toJSON(ಧ್ರುವ MessageActionItem &Params) {
  ಪ್ರತ್ಯಾಗಮ llvm::json::Object{{"title", Params.title}};
}

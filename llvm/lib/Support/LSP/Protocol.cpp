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

#অন্তর্ভাৱ "llvm/Support/LSP/Protocol.h"
#অন্তর্ভাৱ "llvm/ADT/StringExtras.h"
#অন্তর্ভাৱ "llvm/ADT/StringSet.h"
#অন্তর্ভাৱ "llvm/Support/ErrorHandling.h"
#অন্তর্ভাৱ "llvm/Support/JSON.h"
#অন্তর্ভাৱ "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাৱ "llvm/Support/Path.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"

using নামাকাশ llvm;
using নামাকাশ llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
template <typename T>
স্থির তার্কিক mapOptOrNull(ধ্রুৱ llvm::json::মান &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  ধ্রুৱ llvm::json::Object *O = Params.getAsObject();
  প্রতিজ্ঞা(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  যদি (!V || V->getAsNull())
    ঘূৰাই সঁচা;
  ঘূৰাই fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

ৱর্ণ LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

স্থির তার্কিক isWindowsPath(StringRef Path) {
  ঘূৰাই Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

স্থির তার্কিক isNetworkPath(StringRef Path) {
  ঘূৰাই Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

স্থির তার্কিক shouldEscapeInURI(unsigned ৱর্ণ C) {
  // Unreserved characters.
  যদি ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    ঘূৰাই মিছা;

  ৱিকল্প (C) {
  পক্ষ '-':
  পক্ষ '_':
  পক্ষ '.':
  পক্ষ '~':
  // '/' is only reserved when parsing.
  পক্ষ '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  পক্ষ ':':
    ঘূৰাই মিছা;
  }
  ঘূৰাই সঁচা;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
স্থির শূন্য percentEncode(StringRef Content, std::string &Out) {
  বাবে (unsigned ৱর্ণ C : Content) {
    যদি (shouldEscapeInURI(C)) {
      Out.ঠেলো_back('%');
      Out.ঠেলো_back(llvm::hexdigit(C / 16));
      Out.ঠেলো_back(llvm::hexdigit(C % 16));
    } অন্যথা {
      Out.ঠেলো_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
স্থির std::string percentDecode(StringRef Content) {
  std::string Result;
  বাবে (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    যদি (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.ঠেলো_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } অন্যথা {
      Result.ঠেলো_back(*I);
    }
  }
  ঘূৰাই Result;
}

/// Return the set containing the supported URI schemes.
স্থির StringSet<> &getSupportedSchemes() {
  স্থির StringSet<> Schemes({"file", "test"});
  ঘূৰাই Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
স্থির তার্কিক isStructurallyValidScheme(StringRef Scheme) {
  যদি (Scheme.empty())
    ঘূৰাই মিছা;
  যদি (!llvm::isAlpha(Scheme[0]))
    ঘূৰাই মিছা;
  ঘূৰাই llvm::all_of(llvm::drop_begin(Scheme), [](ৱর্ণ C) {
    ঘূৰাই llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

স্থির llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef মূল_2 = llvm::sys::path::মূল_name(AbsolutePath);
  যদি (isNetworkPath(মূল_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = মূল_2.drop_front(2);
    AbsolutePath.consume_front(মূল_2);
  } অন্যথা যদি (isWindowsPath(মূল_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  যদি (Authority.empty() && Body.empty())
    ঘূৰাই Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  যদি (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  ঘূৰাই Uri;
}

স্থির llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  যদি (!Body.starts_with("/"))
    ঘূৰাই llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  যদি (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } অন্যথা যদি (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  ঘূৰাই std::string(Path);
}

স্থির llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  যদি (Pos == StringRef::npos)
    ঘূৰাই llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  যদি (!isStructurallyValidScheme(UriScheme))
    ঘূৰাই llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  যদি (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  যদি (!getSupportedSchemes().contains(UriScheme)) {
    ঘূৰাই llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  ঘূৰাই getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  যদি (!FilePath)
    ঘূৰাই FilePath.takeError();
  ঘূৰাই URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  যদি (!Uri)
    ঘূৰাই Uri.takeError();
  ঘূৰাই fromURI(*Uri);
}

StringRef URIForFile::scheme() ধ্রুৱ { ঘূৰাই uri().split(':').first; }

শূন্য URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().ঢোকাও(Scheme);
}

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান, URIForFile &Result,
                         llvm::json::Path Path) {
  যদি (std::optional<StringRef> Str = মান.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    যদি (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      ঘূৰাই মিছা;
    }
    Result = std::move(*ExpectedUri);
    ঘূৰাই সঁচা;
  }
  ঘূৰাই মিছা;
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ URIForFile &মান) {
  ঘূৰাই মান.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ধ্রুৱ URIForFile &মান) {
  ঘূৰাই Os << মান.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  ধ্রুৱ llvm::json::Object *O = মান.getAsObject();
  যদি (!O) {
    Path.report("expected object");
    ঘূৰাই মিছা;
  }
  যদি (ধ্রুৱ llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    যদি (ধ্রুৱ llvm::json::Object *Document_প্ৰতীক =
            TextDocument->getObject("documentSymbol")) {
      যদি (std::optional<তার্কিক> HierarchicalSupport =
              Document_প্ৰতীক->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_প্ৰতীক = *HierarchicalSupport;
    }
    যদি (auto *CodeAction = TextDocument->getObject("codeAction")) {
      যদি (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = সঁচা;
    }
  }
  যদি (auto *Window = O->getObject("window")) {
    যদি (std::optional<তার্কিক> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  ঘূৰাই সঁচা;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  যদি (!O || !O.map("name", Result.name))
    ঘূৰাই মিছা;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  ঘূৰাই সঁচা;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান, TraceLevel &Result,
                         llvm::json::Path Path) {
  যদি (std::optional<StringRef> Str = মান.getAsString()) {
    যদি (*Str == "off") {
      Result = TraceLevel::Off;
      ঘূৰাই সঁচা;
    }
    যদি (*Str == "messages") {
      Result = TraceLevel::Messages;
      ঘূৰাই সঁচা;
    }
    যদি (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      ঘূৰাই সঁচা;
    }
  }
  ঘূৰাই মিছা;
}

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  যদি (!O)
    ঘূৰাই মিছা;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.মূল_Uri);
  O.map("rootPath", Result.মূল_Path);
  mapOptOrNull(মান, "clientInfo", Result.clientInfo, Path);

  ঘূৰাই সঁচা;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ TextDocumentIdentifier &মান) {
  ঘূৰাই llvm::json::Object{{"uri", মান.uri}};
}

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::মান
llvm::lsp::toJSON(ধ্রুৱ VersionedTextDocumentIdentifier &মান) {
  ঘূৰাই llvm::json::Object{
      {"uri", মান.uri},
      {"version", মান.version},
  };
}

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("line", Result.সাৰি_2) &&
         O.map("character", Result.character);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ Position &মান) {
  ঘূৰাই llvm::json::Object{
      {"line", মান.সাৰি_2},
      {"character", মান.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ধ্রুৱ Position &মান) {
  ঘূৰাই Os << মান.সাৰি_2 << ':' << মান.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ Range &মান) {
  ঘূৰাই llvm::json::Object{
      {"start", মান.start},
      {"end", মান.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ধ্রুৱ Range &মান) {
  ঘূৰাই Os << মান.start << '-' << মান.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ Location &মান) {
  ঘূৰাই llvm::json::Object{
      {"uri", মান.uri},
      {"range", মান.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ধ্রুৱ Location &মান) {
  ঘূৰাই Os << মান.range << '@' << মান.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই fromJSON(মান, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  ঘূৰাই O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) ধ্রুৱ {
  // If there is no range, the full document changed.
  যদি (!range) {
    Contents = text;
    ঘূৰাই success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  যদি (!RangeLoc.isValid())
    ঘূৰাই failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  ঘূৰাই success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  বাবে (ধ্রুৱ auto &Change : Changes)
    যদি (failed(Change.applyTo(Contents)))
      ঘূৰাই failure();
  ঘূৰাই success();
}

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

স্থির llvm::StringRef toTextKind(MarkupKind Kind) {
  ৱিকল্প (Kind) {
  পক্ষ MarkupKind::PlainText:
    ঘূৰাই "plaintext";
  পক্ষ MarkupKind::Markdown:
    ঘূৰাই "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  ঘূৰাই Os << toTextKind(Kind);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ MarkupContent &Mc) {
  যদি (Mc.মান_2.empty())
    ঘূৰাই nullptr;

  ঘূৰাই llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.মান_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  যদি (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  ঘূৰাই std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ Document_প্ৰতীক &প্ৰতীক) {
  llvm::json::Object Result{{"name", প্ৰতীক.name},
                            {"kind", static_cast<পূর্ণাঙ্ক>(প্ৰতীক.kind)},
                            {"range", প্ৰতীক.range},
                            {"selectionRange", প্ৰতীক.selectionRange}};

  যদি (!প্ৰতীক.detail.empty())
    Result["detail"] = প্ৰতীক.detail;
  যদি (!প্ৰতীক.children.empty())
    Result["children"] = প্ৰতীক.children;
  ঘূৰাই std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         Document_প্ৰতীক_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ DiagnosticRelatedInformation &Info) {
  ঘূৰাই llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(DiagnosticTag Tag) {
  ঘূৰাই static_cast<পূর্ণাঙ্ক>(Tag);
}

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  যদি (std::optional<int64_t> I = মান.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    ঘূৰাই সঁচা;
  }

  ঘূৰাই মিছা;
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (পূর্ণাঙ্ক)Diag.severity},
      {"message", Diag.message},
  };
  যদি (Diag.category)
    Result["category"] = *Diag.category;
  যদি (!Diag.source.empty())
    Result["source"] = Diag.source;
  যদি (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  যদি (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  ঘূৰাই std::move(Result);
}

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  যদি (!O)
    ঘূৰাই মিছা;
  পূর্ণাঙ্ক Severity = 0;
  যদি (!mapOptOrNull(মান, "severity", Severity, Path))
    ঘূৰাই মিছা;
  Result.severity = (DiagnosticSeverity)Severity;

  ঘূৰাই O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(মান, "category", Result.category, Path) &&
         mapOptOrNull(মান, "source", Result.source, Path) &&
         mapOptOrNull(মান, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(মান, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ PublishDiagnosticsParams &Params) {
  ঘূৰাই llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ TextEdit &মান) {
  ঘূৰাই llvm::json::Object{
      {"range", মান.range},
      {"newText", মান.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ধ্রুৱ TextEdit &মান) {
  Os << মান.range << " => \"";
  llvm::printEscapedString(মান.newText, Os);
  ঘূৰাই Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  যদি (std::optional<int64_t> Int_মান = মান.getAsInteger()) {
    যদি (*Int_মান < static_cast<পূর্ণাঙ্ক>(CompletionItemKind::Text) ||
        *Int_মান > static_cast<পূর্ণাঙ্ক>(CompletionItemKind::TypeParameter))
      ঘূৰাই মিছা;
    Result = static_cast<CompletionItemKind>(*Int_মান);
    ঘূৰাই সঁচা;
  }
  ঘূৰাই মিছা;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  যদি (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    ঘূৰাই Kind;

  // Provide some fall backs for common kinds that are close enough.
  ৱিকল্প (Kind) {
  পক্ষ CompletionItemKind::Folder:
    ঘূৰাই CompletionItemKind::File;
  পক্ষ CompletionItemKind::EnumMember:
    ঘূৰাই CompletionItemKind::Enum;
  পক্ষ CompletionItemKind::Struct:
    ঘূৰাই CompletionItemKind::Class;
  স্ৱভাৱ:
    ঘূৰাই CompletionItemKind::Text;
  }
}

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  যদি (ধ্রুৱ llvm::json::Array *Array_মান = মান.getAsArray()) {
    বাবে (size_t I = 0, E = Array_মান->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      যদি (fromJSON((*Array_মান)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    ঘূৰাই সঁচা;
  }
  ঘূৰাই মিছা;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ CompletionItem &মান) {
  প্রতিজ্ঞা(!মান.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", মান.label}};
  যদি (মান.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<পূর্ণাঙ্ক>(মান.kind);
  যদি (!মান.detail.empty())
    Result["detail"] = মান.detail;
  যদি (মান.documentation)
    Result["documentation"] = মান.documentation;
  যদি (!মান.sortText.empty())
    Result["sortText"] = মান.sortText;
  যদি (!মান.filterText.empty())
    Result["filterText"] = মান.filterText;
  যদি (!মান.ঢোকাও_Text.empty())
    Result["insertText"] = মান.ঢোকাও_Text;
  যদি (মান.ঢোকাও_Text_Format != ঢোকাও_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<পূর্ণাঙ্ক>(মান.ঢোকাও_Text_Format);
  যদি (মান.textEdit)
    Result["textEdit"] = *মান.textEdit;
  যদি (!মান.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(মান.additionalTextEdits);
  }
  যদি (মান.deprecated)
    Result["deprecated"] = মান.deprecated;
  ঘূৰাই std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ধ্রুৱ CompletionItem &মান) {
  ঘূৰাই Os << মান.label << " - " << toJSON(মান);
}

তার্কিক llvm::lsp::operator<(ধ্রুৱ CompletionItem &Lhs,
                          ধ্রুৱ CompletionItem &Rhs) {
  ঘূৰাই (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ CompletionList &মান) {
  ঘূৰাই llvm::json::Object{
      {"isIncomplete", মান.isIncomplete},
      {"items", llvm::json::Array(মান.বস্তুসমূহ)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  পূর্ণাঙ্ক TriggerKind;
  যদি (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(মান, "triggerCharacter", Result.triggerCharacter, Path))
    ঘূৰাই মিছা;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  ঘূৰাই সঁচা;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         CompletionParams &Result, llvm::json::Path Path) {
  যদি (!fromJSON(মান, static_cast<TextDocumentPositionParams &>(Result), Path))
    ঘূৰাই মিছা;
  যদি (ধ্রুৱ llvm::json::মান *Context = মান.getAsObject()->get("context"))
    ঘূৰাই fromJSON(*Context, Result.context, Path.field("context"));
  ঘূৰাই সঁচা;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ ParameterInformation &মান) {
  প্রতিজ্ঞা((মান.labelOffsets || !মান.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  যদি (মান.labelOffsets)
    Result["label"] = llvm::json::Array(
        {মান.labelOffsets->first, মান.labelOffsets->second});
  অন্যথা
    Result["label"] = মান.labelString;
  যদি (!মান.documentation.empty())
    Result["documentation"] = মান.documentation;
  ঘূৰাই std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ SignatureInformation &মান) {
  প্রতিজ্ঞা(!মান.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", মান.label},
      {"parameters", llvm::json::Array(মান.parameters)},
  };
  যদি (!মান.documentation.empty())
    Result["documentation"] = মান.documentation;
  ঘূৰাই std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ধ্রুৱ SignatureInformation &মান) {
  ঘূৰাই Os << মান.label << " - " << toJSON(মান);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ SignatureHelp &মান) {
  প্রতিজ্ঞা(মান.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  প্রতিজ্ঞা(মান.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  ঘূৰাই llvm::json::Object{
      {"activeSignature", মান.activeSignature},
      {"activeParameter", মান.activeParameter},
      {"signatures", llvm::json::Array(মান.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ DocumentLink &মান) {
  ঘূৰাই llvm::json::Object{
      {"range", মান.range},
      {"target", মান.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ InlayHint &মান) {
  ঘূৰাই llvm::json::Object{{"position", মান.position},
                            {"kind", (পূর্ণাঙ্ক)মান.kind},
                            {"label", মান.label},
                            {"paddingLeft", মান.padding_বাম},
                            {"paddingRight", মান.padding_ডান}};
}
তার্কিক llvm::lsp::operator==(ধ্রুৱ InlayHint &Lhs, ধ্রুৱ InlayHint &Rhs) {
  ঘূৰাই std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
তার্কিক llvm::lsp::operator<(ধ্রুৱ InlayHint &Lhs, ধ্রুৱ InlayHint &Rhs) {
  ঘূৰাই std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind মান) {
  ৱিকল্প (মান) {
  পক্ষ InlayHintKind::Parameter:
    ঘূৰাই Os << "parameter";
  পক্ষ InlayHintKind::Type:
    ঘূৰাই Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  যদি (!O || !O.map("diagnostics", Result.diagnostics))
    ঘূৰাই মিছা;
  O.map("only", Result.only);
  ঘূৰাই সঁচা;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

তার্কিক llvm::lsp::fromJSON(ধ্রুৱ llvm::json::মান &মান, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ঘূৰাই O && O.map("changes", Result.changes);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ WorkspaceEdit &মান) {
  llvm::json::Object FileChanges;
  বাবে (auto &Change : মান.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  ঘূৰাই llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ধ্রুৱ llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
ধ্রুৱ llvm::StringLiteral CodeAction::kRefactor = "refactor";
ধ্রুৱ llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ CodeAction &মান) {
  llvm::json::Object CodeAction{{"title", মান.title}};
  যদি (মান.kind)
    CodeAction["kind"] = *মান.kind;
  যদি (মান.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*মান.diagnostics);
  যদি (মান.isPreferred)
    CodeAction["isPreferred"] = সঁচা;
  যদি (মান.edit)
    CodeAction["edit"] = *মান.edit;
  ঘূৰাই std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<পূর্ণাঙ্ক>(Params.type)},
      {"message", Params.message},
  };
  যদি (Params.actions)
    Out["actions"] = *Params.actions;
  ঘূৰাই Out;
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুৱ MessageActionItem &Params) {
  ঘূৰাই llvm::json::Object{{"title", Params.title}};
}

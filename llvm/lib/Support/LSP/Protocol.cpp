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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/LSP/Protocol.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringSet.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/JSON.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Path.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ T>
ਸਥਿਰ ਤਾਰਕਿਕ mapOptOrNull(ਅਚਲ llvm::json::ਮੁੱਲ &Params,
                         llvm::StringLiteral Prop, T &ਬਾਹਰਲਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  ਅਚਲ llvm::json::Object *O = Params.getAsObject();
  ਪ੍ਰਤਿਜ੍ਞਾ(O);

  // Field is missing or null.
  ਸਵੈਚਾਲਿਤ *V = O->get(Prop);
  ਜੇ (!V || V->getAsNull())
    ਵਾਪਸ ਸੱਚ;
  ਵਾਪਸ fromJSON(*V, ਬਾਹਰਲਾ, ਰਸਤਾ.ਖੇਤਰ(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

ਅੱਖਰ LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

ਸਥਿਰ ਤਾਰਕਿਕ is_Windows_ਰਸਤਾ(StringRef ਰਸਤਾ) {
  ਵਾਪਸ ਰਸਤਾ.ਮਾਪ_2() > 1 && llvm::isAlpha(ਰਸਤਾ[0]) && ਰਸਤਾ[1] == ':';
}

ਸਥਿਰ ਤਾਰਕਿਕ is_ਜਾਲ_ਰਸਤਾ(StringRef ਰਸਤਾ) {
  ਵਾਪਸ ਰਸਤਾ.ਮਾਪ_2() > 2 && ਰਸਤਾ[0] == ਰਸਤਾ[1] &&
         llvm::sys::ਰਸਤਾ_2::is_separator(ਰਸਤਾ[0]);
}

ਸਥਿਰ ਤਾਰਕਿਕ should_Escape_ਵਿੱਚ_URI(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ C) {
  // Unreserved characters.
  ਜੇ ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    ਵਾਪਸ ਝੂਠ;

  ਚੋਣ (C) {
  ਹਾਲਤ '-':
  ਹਾਲਤ '_':
  ਹਾਲਤ '.':
  ਹਾਲਤ '~':
  // '/' is only reserved when parsing.
  ਹਾਲਤ '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  ਹਾਲਤ ':':
    ਵਾਪਸ ਝੂਠ;
  }
  ਵਾਪਸ ਸੱਚ;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
ਸਥਿਰ ਖਾਲੀ percentEncode(StringRef Content, std::string &ਬਾਹਰਲਾ) {
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ C : Content) {
    ਜੇ (should_Escape_ਵਿੱਚ_URI(C)) {
      ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ('%');
      ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(llvm::hexdigit(C / 16));
      ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(llvm::hexdigit(C % 16));
    } ਨਹੀਂਤਾਂ {
      ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ਸਥਿਰ std::string percentDecode(StringRef Content) {
  std::string ਨਤੀਜਾ;
  ਲਈ (ਸਵੈਚਾਲਿਤ I = Content.begin(), E = Content.ਸਿਰਾ(); I != E; ++I) {
    ਜੇ (*I == '%' && I + 2 < Content.ਸਿਰਾ() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      ਨਤੀਜਾ.ਧੱਕੋ_ਪਿੱਠ(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } ਨਹੀਂਤਾਂ {
      ਨਤੀਜਾ.ਧੱਕੋ_ਪਿੱਠ(*I);
    }
  }
  ਵਾਪਸ ਨਤੀਜਾ;
}

/// Return the set containing the supported URI schemes.
ਸਥਿਰ String_ਸੈੱਟ<> &getSupportedSchemes() {
  ਸਥਿਰ String_ਸੈੱਟ<> Schemes({"file", "test"});
  ਵਾਪਸ Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
ਸਥਿਰ ਤਾਰਕਿਕ isStructurallyValidScheme(StringRef Scheme) {
  ਜੇ (Scheme.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (!llvm::isAlpha(Scheme[0]))
    ਵਾਪਸ ਝੂਠ;
  ਵਾਪਸ llvm::all_ਦਾ(llvm::drop_begin(Scheme), [](ਅੱਖਰ C) {
    ਵਾਪਸ llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

ਸਥਿਰ llvm::Expected<std::string> uri_From_Absolute_ਰਸਤਾ(StringRef Absolute_ਰਸਤਾ,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef ਜੜ੍ਹ_2 = llvm::sys::ਰਸਤਾ_2::ਜੜ੍ਹ_name(Absolute_ਰਸਤਾ);
  ਜੇ (is_ਜਾਲ_ਰਸਤਾ(ਜੜ੍ਹ_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = ਜੜ੍ਹ_2.drop_front(2);
    Absolute_ਰਸਤਾ.consume_front(ਜੜ੍ਹ_2);
  } ਨਹੀਂਤਾਂ ਜੇ (is_Windows_ਰਸਤਾ(ਜੜ੍ਹ_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::ਰਸਤਾ_2::convert_ਮੰਜ਼ਿਲ_slash(Absolute_ਰਸਤਾ);

  std::string Uri = Scheme.str() + ":";
  ਜੇ (Authority.ਖਾਲੀ_ਥਾਵਾਂ() && Body.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  ਜੇ (!Authority.ਖਾਲੀ_ਥਾਵਾਂ() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  ਵਾਪਸ Uri;
}

ਸਥਿਰ llvm::Expected<std::string> get_Absolute_ਰਸਤਾ(StringRef Authority,
                                                   StringRef Body) {
  ਜੇ (!Body.starts_with("/"))
    ਵਾਪਸ llvm::createStringError(
        llvm::inconvertible_Error_ਕੋਡ(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> ਰਸਤਾ;
  ਜੇ (!Authority.ਖਾਲੀ_ਥਾਵਾਂ()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਰਸਤਾ);
  } ਨਹੀਂਤਾਂ ਜੇ (is_Windows_ਰਸਤਾ(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  ਰਸਤਾ.append(Body);
  llvm::sys::ਰਸਤਾ_2::native(ਰਸਤਾ);
  ਵਾਪਸ std::string(ਰਸਤਾ);
}

ਸਥਿਰ llvm::Expected<std::string> parse_File_ਰਸਤਾ_From_URI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.ਲੱਭੋ(':');
  ਜੇ (Pos == StringRef::npos)
    ਵਾਪਸ llvm::createStringError(llvm::inconvertible_Error_ਕੋਡ(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  ਜੇ (!isStructurallyValidScheme(UriScheme))
    ਵਾਪਸ llvm::createStringError(llvm::inconvertible_Error_ਕੋਡ(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  ਜੇ (Uri.consume_front("//")) {
    Pos = Uri.ਲੱਭੋ('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  ਜੇ (!getSupportedSchemes().ਸ਼ਾਮਲ_ਹੈ(UriScheme)) {
    ਵਾਪਸ llvm::createStringError(llvm::inconvertible_Error_ਕੋਡ(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  ਵਾਪਸ get_Absolute_ਰਸਤਾ(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> File_ਰਸਤਾ = parse_File_ਰਸਤਾ_From_URI(Uri);
  ਜੇ (!File_ਰਸਤਾ)
    ਵਾਪਸ File_ਰਸਤਾ.takeError();
  ਵਾਪਸ URIForFile(std::move(*File_ਰਸਤਾ), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uri_From_Absolute_ਰਸਤਾ(AbsoluteFilepath, Scheme);
  ਜੇ (!Uri)
    ਵਾਪਸ Uri.takeError();
  ਵਾਪਸ fromURI(*Uri);
}

StringRef URIForFile::scheme() ਅਚਲ { ਵਾਪਸ uri().ਵੰਡੋ(':').ਪਹਿਲਾ_2; }

ਖਾਲੀ URIForFile::ਰਜਿਸਟਰ_Supported_Scheme(StringRef Scheme) {
  getSupportedSchemes().ਪਾਓ(Scheme);
}

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ, URIForFile &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  ਜੇ (std::optional<StringRef> Str = ਮੁੱਲ.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    ਜੇ (!ExpectedUri) {
      ਰਸਤਾ.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      ਵਾਪਸ ਝੂਠ;
    }
    ਨਤੀਜਾ = std::move(*ExpectedUri);
    ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ ਝੂਠ;
}

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ URIForFile &ਮੁੱਲ) {
  ਵਾਪਸ ਮੁੱਲ.uri();
}

raw_ostream &llvm::lsp::ਸੰਚਾਲਕ<<(raw_ostream &Os, ਅਚਲ URIForFile &ਮੁੱਲ) {
  ਵਾਪਸ Os << ਮੁੱਲ.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         ClientCapabilities &ਨਤੀਜਾ, llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  ਅਚਲ llvm::json::Object *O = ਮੁੱਲ.getAsObject();
  ਜੇ (!O) {
    ਰਸਤਾ.report("expected object");
    ਵਾਪਸ ਝੂਠ;
  }
  ਜੇ (ਅਚਲ llvm::json::Object *ਪਾਠ_Document = O->getObject("textDocument")) {
    ਜੇ (ਅਚਲ llvm::json::Object *Document_ਚਿੰਨ੍ਹ =
            ਪਾਠ_Document->getObject("documentSymbol")) {
      ਜੇ (std::optional<ਤਾਰਕਿਕ> ਦਰਜਾਵਾਰ_Support =
              Document_ਚਿੰਨ੍ਹ->getBoolean("hierarchicalDocumentSymbolSupport"))
        ਨਤੀਜਾ.ਦਰਜਾਵਾਰ_Document_ਚਿੰਨ੍ਹ = *ਦਰਜਾਵਾਰ_Support;
    }
    ਜੇ (ਸਵੈਚਾਲਿਤ *ਕੋਡ_Action = ਪਾਠ_Document->getObject("codeAction")) {
      ਜੇ (ਕੋਡ_Action->getObject("codeActionLiteralSupport"))
        ਨਤੀਜਾ.ਕੋਡ_Action_Structure = ਸੱਚ;
    }
  }
  ਜੇ (ਸਵੈਚਾਲਿਤ *ਖਿੜਕੀ = O->getObject("window")) {
    ਜੇ (std::optional<ਤਾਰਕਿਕ> Work_ਪੂਰਾ_ਤਰੱਕੀ_Support =
            ਖਿੜਕੀ->getBoolean("workDoneProgress"))
      ਨਤੀਜਾ.work_ਪੂਰਾ_ਤਰੱਕੀ = *Work_ਪੂਰਾ_ਤਰੱਕੀ_Support;
  }
  ਵਾਪਸ ਸੱਚ;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ, ClientInfo &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਜੇ (!O || !O.map("name", ਨਤੀਜਾ.name))
    ਵਾਪਸ ਝੂਠ;

  // Don't fail if we can't parse version.
  O.map("version", ਨਤੀਜਾ.ਸੰਸਕਰਣ_2);
  ਵਾਪਸ ਸੱਚ;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ, TraceLevel &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  ਜੇ (std::optional<StringRef> Str = ਮੁੱਲ.getAsString()) {
    ਜੇ (*Str == "off") {
      ਨਤੀਜਾ = TraceLevel::Off;
      ਵਾਪਸ ਸੱਚ;
    }
    ਜੇ (*Str == "messages") {
      ਨਤੀਜਾ = TraceLevel::Messages;
      ਵਾਪਸ ਸੱਚ;
    }
    ਜੇ (*Str == "verbose") {
      ਨਤੀਜਾ = TraceLevel::Verbose;
      ਵਾਪਸ ਸੱਚ;
    }
  }
  ਵਾਪਸ ਝੂਠ;
}

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         InitializeParams &ਨਤੀਜਾ, llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਜੇ (!O)
    ਵਾਪਸ ਝੂਠ;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", ਨਤੀਜਾ.capabilities);
  O.map("trace", ਨਤੀਜਾ.trace);
  O.map("rootUri", ਨਤੀਜਾ.ਜੜ੍ਹ_Uri);
  O.map("rootPath", ਨਤੀਜਾ.ਜੜ੍ਹ_ਰਸਤਾ);
  mapOptOrNull(ਮੁੱਲ, "clientInfo", ਨਤੀਜਾ.clientInfo, ਰਸਤਾ);

  ਵਾਪਸ ਸੱਚ;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         ਪਾਠ_Document_ਵਸਤੂ &ਨਤੀਜਾ, llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("uri", ਨਤੀਜਾ.uri) &&
         O.map("languageId", ਨਤੀਜਾ.languageId) && O.map("text", ਨਤੀਜਾ.ਪਾਠ) &&
         O.map("version", ਨਤੀਜਾ.ਸੰਸਕਰਣ_2);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ ਪਾਠ_Document_Identifier &ਮੁੱਲ) {
  ਵਾਪਸ llvm::json::Object{{"uri", ਮੁੱਲ.uri}};
}

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         ਪਾਠ_Document_Identifier &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("uri", ਨਤੀਜਾ.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::ਮੁੱਲ
llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ Versioned_ਪਾਠ_Document_Identifier &ਮੁੱਲ) {
  ਵਾਪਸ llvm::json::Object{
      {"uri", ਮੁੱਲ.uri},
      {"version", ਮੁੱਲ.ਸੰਸਕਰਣ_2},
  };
}

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         Versioned_ਪਾਠ_Document_Identifier &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("uri", ਨਤੀਜਾ.uri) && O.map("version", ਨਤੀਜਾ.ਸੰਸਕਰਣ_2);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ, Position &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("line", ਨਤੀਜਾ.ਸਤਰ_2) &&
         O.map("character", ਨਤੀਜਾ.character);
}

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ Position &ਮੁੱਲ) {
  ਵਾਪਸ llvm::json::Object{
      {"line", ਮੁੱਲ.ਸਤਰ_2},
      {"character", ਮੁੱਲ.character},
  };
}

raw_ostream &llvm::lsp::ਸੰਚਾਲਕ<<(raw_ostream &Os, ਅਚਲ Position &ਮੁੱਲ) {
  ਵਾਪਸ Os << ਮੁੱਲ.ਸਤਰ_2 << ':' << ਮੁੱਲ.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ, Range &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("start", ਨਤੀਜਾ.ਸ਼ੁਰੂ_2) && O.map("end", ਨਤੀਜਾ.ਸਿਰਾ);
}

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ Range &ਮੁੱਲ) {
  ਵਾਪਸ llvm::json::Object{
      {"start", ਮੁੱਲ.ਸ਼ੁਰੂ_2},
      {"end", ਮੁੱਲ.ਸਿਰਾ},
  };
}

raw_ostream &llvm::lsp::ਸੰਚਾਲਕ<<(raw_ostream &Os, ਅਚਲ Range &ਮੁੱਲ) {
  ਵਾਪਸ Os << ਮੁੱਲ.ਸ਼ੁਰੂ_2 << '-' << ਮੁੱਲ.ਸਿਰਾ;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ, Location &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("uri", ਨਤੀਜਾ.uri) && O.map("range", ਨਤੀਜਾ.range);
}

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ Location &ਮੁੱਲ) {
  ਵਾਪਸ llvm::json::Object{
      {"uri", ਮੁੱਲ.uri},
      {"range", ਮੁੱਲ.range},
  };
}

raw_ostream &llvm::lsp::ਸੰਚਾਲਕ<<(raw_ostream &Os, ਅਚਲ Location &ਮੁੱਲ) {
  ਵਾਪਸ Os << ਮੁੱਲ.range << '@' << ਮੁੱਲ.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         ਪਾਠ_Document_Position_Params &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("textDocument", ਨਤੀਜਾ.ਪਾਠ_Document_2) &&
         O.map("position", ਨਤੀਜਾ.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         ReferenceContext &ਨਤੀਜਾ, llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.mapOptional("includeDeclaration", ਨਤੀਜਾ.includeDeclaration);
}

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         ReferenceParams &ਨਤੀਜਾ, llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  ਪਾਠ_Document_Position_Params &ਅਧਾਰ = ਨਤੀਜਾ;
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ fromJSON(ਮੁੱਲ, ਅਧਾਰ, ਰਸਤਾ) && O &&
         O.mapOptional("context", ਨਤੀਜਾ.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         Did_Open_ਪਾਠ_Document_Params &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("textDocument", ਨਤੀਜਾ.ਪਾਠ_Document_2);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         Did_Close_ਪਾਠ_Document_Params &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("textDocument", ਨਤੀਜਾ.ਪਾਠ_Document_2);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &Params,
                         Did_Save_ਪਾਠ_Document_Params &R, llvm::json::ਰਸਤਾ P) {
  llvm::json::ObjectMapper O(Params, P);
  ਵਾਪਸ O && O.map("textDocument", R.ਪਾਠ_Document_2);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

Logical_ਨਤੀਜਾ
ਪਾਠ_Document_Content_Change_Event::apply_ਮੰਜ਼ਿਲ(std::string &Contents) ਅਚਲ {
  // If there is no range, the full document changed.
  ਜੇ (!range) {
    Contents = ਪਾਠ;
    ਵਾਪਸ success();
  }

  // Try to map the replacement range to the content.
  llvm::ਸਰੋਤ_Mgr TmpScrMgr;
  TmpScrMgr.ਜੋੜੋ_New_ਸਰੋਤ_ਬਫਰ(llvm::ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_Mem_ਬਫਰ(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  ਜੇ (!RangeLoc.isValid())
    ਵਾਪਸ failure();

  Contents.replace(RangeLoc.ਸ਼ੁਰੂ.get_ਸੂਚਕ() - Contents.ਅੰਕੜੇ(),
                   RangeLoc.ਸਿਰਾ_2.get_ਸੂਚਕ() - RangeLoc.ਸ਼ੁਰੂ.get_ਸੂਚਕ(),
                   ਪਾਠ);
  ਵਾਪਸ success();
}

Logical_ਨਤੀਜਾ ਪਾਠ_Document_Content_Change_Event::apply_ਮੰਜ਼ਿਲ(
    ArrayRef<ਪਾਠ_Document_Content_Change_Event> Changes, std::string &Contents) {
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &Change : Changes)
    ਜੇ (failed(Change.apply_ਮੰਜ਼ਿਲ(Contents)))
      ਵਾਪਸ failure();
  ਵਾਪਸ success();
}

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         ਪਾਠ_Document_Content_Change_Event &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("range", ਨਤੀਜਾ.range) &&
         O.map("rangeLength", ਨਤੀਜਾ.range_ਲੰਮਾਈ) && O.map("text", ਨਤੀਜਾ.ਪਾਠ);
}

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         Did_Change_ਪਾਠ_Document_Params &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("textDocument", ਨਤੀਜਾ.ਪਾਠ_Document_2) &&
         O.map("contentChanges", ਨਤੀਜਾ.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

ਸਥਿਰ llvm::StringRef ਮੰਜ਼ਿਲ_ਪਾਠ_ਕਿਸਮ(Markup_ਕਿਸਮ ਕਿਸਮ_3) {
  ਚੋਣ (ਕਿਸਮ_3) {
  ਹਾਲਤ Markup_ਕਿਸਮ::Plain_ਪਾਠ:
    ਵਾਪਸ "plaintext";
  ਹਾਲਤ Markup_ਕਿਸਮ::Markdown:
    ਵਾਪਸ "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::ਸੰਚਾਲਕ<<(raw_ostream &Os, Markup_ਕਿਸਮ ਕਿਸਮ_3) {
  ਵਾਪਸ Os << ਮੰਜ਼ਿਲ_ਪਾਠ_ਕਿਸਮ(ਕਿਸਮ_3);
}

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ MarkupContent &Mc) {
  ਜੇ (Mc.ਮੁੱਲ_2.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

  ਵਾਪਸ llvm::json::Object{
      {"kind", ਮੰਜ਼ਿਲ_ਪਾਠ_ਕਿਸਮ(Mc.ਕਿਸਮ_2)},
      {"value", Mc.ਮੁੱਲ_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ Hover &Hover) {
  llvm::json::Object ਨਤੀਜਾ{{"contents", ਮੰਜ਼ਿਲ_JSON(Hover.contents)}};
  ਜੇ (Hover.range)
    ਨਤੀਜਾ["range"] = ਮੰਜ਼ਿਲ_JSON(*Hover.range);
  ਵਾਪਸ std::move(ਨਤੀਜਾ);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ Document_ਚਿੰਨ੍ਹ &ਚਿੰਨ੍ਹ) {
  llvm::json::Object ਨਤੀਜਾ{{"name", ਚਿੰਨ੍ਹ.name},
                            {"kind", static_cast<ਪੂਰਨ>(ਚਿੰਨ੍ਹ.ਕਿਸਮ_2)},
                            {"range", ਚਿੰਨ੍ਹ.range},
                            {"selectionRange", ਚਿੰਨ੍ਹ.selectionRange}};

  ਜੇ (!ਚਿੰਨ੍ਹ.detail.ਖਾਲੀ_ਥਾਵਾਂ())
    ਨਤੀਜਾ["detail"] = ਚਿੰਨ੍ਹ.detail;
  ਜੇ (!ਚਿੰਨ੍ਹ.children.ਖਾਲੀ_ਥਾਵਾਂ())
    ਨਤੀਜਾ["children"] = ਚਿੰਨ੍ਹ.children;
  ਵਾਪਸ std::move(ਨਤੀਜਾ);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         Document_ਚਿੰਨ੍ਹ_Params &ਨਤੀਜਾ, llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("textDocument", ਨਤੀਜਾ.ਪਾਠ_Document_2);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         DiagnosticRelatedInformation &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("location", ਨਤੀਜਾ.location) &&
         O.map("message", ਨਤੀਜਾ.ਸੁਨੇਹਾ_2);
}

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ DiagnosticRelatedInformation &Info) {
  ਵਾਪਸ llvm::json::Object{
      {"location", Info.location},
      {"message", Info.ਸੁਨੇਹਾ_2},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(Diagnostic_ਟੈਗ ਟੈਗ) {
  ਵਾਪਸ static_cast<ਪੂਰਨ>(ਟੈਗ);
}

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ, Diagnostic_ਟੈਗ &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  ਜੇ (std::optional<int64_t> I = ਮੁੱਲ.getAsInteger()) {
    ਨਤੀਜਾ = (Diagnostic_ਟੈਗ)*I;
    ਵਾਪਸ ਸੱਚ;
  }

  ਵਾਪਸ ਝੂਠ;
}

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ Diagnostic &Diag) {
  llvm::json::Object ਨਤੀਜਾ{
      {"range", Diag.range},
      {"severity", (ਪੂਰਨ)Diag.severity},
      {"message", Diag.ਸੁਨੇਹਾ_2},
  };
  ਜੇ (Diag.category)
    ਨਤੀਜਾ["category"] = *Diag.category;
  ਜੇ (!Diag.ਸਰੋਤ_2.ਖਾਲੀ_ਥਾਵਾਂ())
    ਨਤੀਜਾ["source"] = Diag.ਸਰੋਤ_2;
  ਜੇ (Diag.relatedInformation)
    ਨਤੀਜਾ["relatedInformation"] = *Diag.relatedInformation;
  ਜੇ (!Diag.ਟੈਗ_ਸੂਚੀ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਨਤੀਜਾ["tags"] = Diag.ਟੈਗ_ਸੂਚੀ;
  ਵਾਪਸ std::move(ਨਤੀਜਾ);
}

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ, Diagnostic &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਜੇ (!O)
    ਵਾਪਸ ਝੂਠ;
  ਪੂਰਨ Severity = 0;
  ਜੇ (!mapOptOrNull(ਮੁੱਲ, "severity", Severity, ਰਸਤਾ))
    ਵਾਪਸ ਝੂਠ;
  ਨਤੀਜਾ.severity = (DiagnosticSeverity)Severity;

  ਵਾਪਸ O.map("range", ਨਤੀਜਾ.range) && O.map("message", ਨਤੀਜਾ.ਸੁਨੇਹਾ_2) &&
         mapOptOrNull(ਮੁੱਲ, "category", ਨਤੀਜਾ.category, ਰਸਤਾ) &&
         mapOptOrNull(ਮੁੱਲ, "source", ਨਤੀਜਾ.ਸਰੋਤ_2, ਰਸਤਾ) &&
         mapOptOrNull(ਮੁੱਲ, "relatedInformation", ਨਤੀਜਾ.relatedInformation,
                      ਰਸਤਾ) &&
         mapOptOrNull(ਮੁੱਲ, "tags", ਨਤੀਜਾ.ਟੈਗ_ਸੂਚੀ, ਰਸਤਾ);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ PublishDiagnosticsParams &Params) {
  ਵਾਪਸ llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.ਸੰਸਕਰਣ_2},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ, ਪਾਠ_Edit &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("range", ਨਤੀਜਾ.range) && O.map("newText", ਨਤੀਜਾ.new_ਪਾਠ);
}

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ ਪਾਠ_Edit &ਮੁੱਲ) {
  ਵਾਪਸ llvm::json::Object{
      {"range", ਮੁੱਲ.range},
      {"newText", ਮੁੱਲ.new_ਪਾਠ},
  };
}

raw_ostream &llvm::lsp::ਸੰਚਾਲਕ<<(raw_ostream &Os, ਅਚਲ ਪਾਠ_Edit &ਮੁੱਲ) {
  Os << ਮੁੱਲ.range << " => \"";
  llvm::printEscapedString(ਮੁੱਲ.new_ਪਾਠ, Os);
  ਵਾਪਸ Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         Completion_ਵਸਤੂ_ਕਿਸਮ &ਨਤੀਜਾ, llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  ਜੇ (std::optional<int64_t> Int_ਮੁੱਲ = ਮੁੱਲ.getAsInteger()) {
    ਜੇ (*Int_ਮੁੱਲ < static_cast<ਪੂਰਨ>(Completion_ਵਸਤੂ_ਕਿਸਮ::ਪਾਠ_2) ||
        *Int_ਮੁੱਲ > static_cast<ਪੂਰਨ>(Completion_ਵਸਤੂ_ਕਿਸਮ::TypeParameter))
      ਵਾਪਸ ਝੂਠ;
    ਨਤੀਜਾ = static_cast<Completion_ਵਸਤੂ_ਕਿਸਮ>(*Int_ਮੁੱਲ);
    ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ ਝੂਠ;
}

Completion_ਵਸਤੂ_ਕਿਸਮ llvm::lsp::adjust_ਕਿਸਮ_ਮੰਜ਼ਿਲ_Capability(
    Completion_ਵਸਤੂ_ਕਿਸਮ ਕਿਸਮ_3,
    Completion_ਵਸਤੂ_ਕਿਸਮ_Bitset &Supported_Completion_ਵਸਤੂ_Kinds) {
  size_t ਕਿਸਮ_Val = static_cast<size_t>(ਕਿਸਮ_3);
  ਜੇ (ਕਿਸਮ_Val >= k_Completion_ਵਸਤੂ_ਕਿਸਮ_Min &&
      ਕਿਸਮ_Val <= Supported_Completion_ਵਸਤੂ_Kinds.ਮਾਪ_2() &&
      Supported_Completion_ਵਸਤੂ_Kinds[ਕਿਸਮ_Val])
    ਵਾਪਸ ਕਿਸਮ_3;

  // Provide some fall backs for common kinds that are close enough.
  ਚੋਣ (ਕਿਸਮ_3) {
  ਹਾਲਤ Completion_ਵਸਤੂ_ਕਿਸਮ::Folder:
    ਵਾਪਸ Completion_ਵਸਤੂ_ਕਿਸਮ::File;
  ਹਾਲਤ Completion_ਵਸਤੂ_ਕਿਸਮ::EnumMember:
    ਵਾਪਸ Completion_ਵਸਤੂ_ਕਿਸਮ::Enum;
  ਹਾਲਤ Completion_ਵਸਤੂ_ਕਿਸਮ::Struct:
    ਵਾਪਸ Completion_ਵਸਤੂ_ਕਿਸਮ::Class;
  ਮੂਲ:
    ਵਾਪਸ Completion_ਵਸਤੂ_ਕਿਸਮ::ਪਾਠ_2;
  }
}

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         Completion_ਵਸਤੂ_ਕਿਸਮ_Bitset &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  ਜੇ (ਅਚਲ llvm::json::Array *Array_ਮੁੱਲ = ਮੁੱਲ.getAsArray()) {
    ਲਈ (size_t I = 0, E = Array_ਮੁੱਲ->ਮਾਪ_2(); I < E; ++I) {
      Completion_ਵਸਤੂ_ਕਿਸਮ ਕਿਸਮ_ਬਾਹਰਲਾ;
      ਜੇ (fromJSON((*Array_ਮੁੱਲ)[I], ਕਿਸਮ_ਬਾਹਰਲਾ, ਰਸਤਾ.ਸੂਚਕਾਂਕ(I)))
        ਨਤੀਜਾ.ਸੈੱਟ(size_t(ਕਿਸਮ_ਬਾਹਰਲਾ));
    }
    ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ ਝੂਠ;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ Completion_ਵਸਤੂ &ਮੁੱਲ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!ਮੁੱਲ.label.ਖਾਲੀ_ਥਾਵਾਂ() && "completion item label is required");
  llvm::json::Object ਨਤੀਜਾ{{"label", ਮੁੱਲ.label}};
  ਜੇ (ਮੁੱਲ.ਕਿਸਮ_2 != Completion_ਵਸਤੂ_ਕਿਸਮ::Missing)
    ਨਤੀਜਾ["kind"] = static_cast<ਪੂਰਨ>(ਮੁੱਲ.ਕਿਸਮ_2);
  ਜੇ (!ਮੁੱਲ.detail.ਖਾਲੀ_ਥਾਵਾਂ())
    ਨਤੀਜਾ["detail"] = ਮੁੱਲ.detail;
  ਜੇ (ਮੁੱਲ.documentation)
    ਨਤੀਜਾ["documentation"] = ਮੁੱਲ.documentation;
  ਜੇ (!ਮੁੱਲ.ਛਾਂਟੋ_ਪਾਠ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਨਤੀਜਾ["sortText"] = ਮੁੱਲ.ਛਾਂਟੋ_ਪਾਠ;
  ਜੇ (!ਮੁੱਲ.filter_ਪਾਠ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਨਤੀਜਾ["filterText"] = ਮੁੱਲ.filter_ਪਾਠ;
  ਜੇ (!ਮੁੱਲ.ਪਾਓ_ਪਾਠ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਨਤੀਜਾ["insertText"] = ਮੁੱਲ.ਪਾਓ_ਪਾਠ;
  ਜੇ (ਮੁੱਲ.ਪਾਓ_ਪਾਠ_Format != ਪਾਓ_ਪਾਠ_Format_2::Missing)
    ਨਤੀਜਾ["insertTextFormat"] = static_cast<ਪੂਰਨ>(ਮੁੱਲ.ਪਾਓ_ਪਾਠ_Format);
  ਜੇ (ਮੁੱਲ.ਪਾਠ_Edit_2)
    ਨਤੀਜਾ["textEdit"] = *ਮੁੱਲ.ਪਾਠ_Edit_2;
  ਜੇ (!ਮੁੱਲ.additional_ਪਾਠ_Edits.ਖਾਲੀ_ਥਾਵਾਂ()) {
    ਨਤੀਜਾ["additionalTextEdits"] =
        llvm::json::Array(ਮੁੱਲ.additional_ਪਾਠ_Edits);
  }
  ਜੇ (ਮੁੱਲ.deprecated)
    ਨਤੀਜਾ["deprecated"] = ਮੁੱਲ.deprecated;
  ਵਾਪਸ std::move(ਨਤੀਜਾ);
}

raw_ostream &llvm::lsp::ਸੰਚਾਲਕ<<(raw_ostream &Os,
                                   ਅਚਲ Completion_ਵਸਤੂ &ਮੁੱਲ) {
  ਵਾਪਸ Os << ਮੁੱਲ.label << " - " << ਮੰਜ਼ਿਲ_JSON(ਮੁੱਲ);
}

ਤਾਰਕਿਕ llvm::lsp::ਸੰਚਾਲਕ<(ਅਚਲ Completion_ਵਸਤੂ &Lhs,
                          ਅਚਲ Completion_ਵਸਤੂ &Rhs) {
  ਵਾਪਸ (Lhs.ਛਾਂਟੋ_ਪਾਠ.ਖਾਲੀ_ਥਾਵਾਂ() ? Lhs.label : Lhs.ਛਾਂਟੋ_ਪਾਠ) <
         (Rhs.ਛਾਂਟੋ_ਪਾਠ.ਖਾਲੀ_ਥਾਵਾਂ() ? Rhs.label : Rhs.ਛਾਂਟੋ_ਪਾਠ);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ CompletionList &ਮੁੱਲ) {
  ਵਾਪਸ llvm::json::Object{
      {"isIncomplete", ਮੁੱਲ.isIncomplete},
      {"items", llvm::json::Array(ਮੁੱਲ.ਚੀਜ਼ਾਂ)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         CompletionContext &ਨਤੀਜਾ, llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਪੂਰਨ Trigger_ਕਿਸਮ;
  ਜੇ (!O || !O.map("triggerKind", Trigger_ਕਿਸਮ) ||
      !mapOptOrNull(ਮੁੱਲ, "triggerCharacter", ਨਤੀਜਾ.triggerCharacter, ਰਸਤਾ))
    ਵਾਪਸ ਝੂਠ;
  ਨਤੀਜਾ.trigger_ਕਿਸਮ = static_cast<Completion_Trigger_ਕਿਸਮ>(Trigger_ਕਿਸਮ);
  ਵਾਪਸ ਸੱਚ;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         CompletionParams &ਨਤੀਜਾ, llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  ਜੇ (!fromJSON(ਮੁੱਲ, static_cast<ਪਾਠ_Document_Position_Params &>(ਨਤੀਜਾ), ਰਸਤਾ))
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (ਅਚਲ llvm::json::ਮੁੱਲ *Context = ਮੁੱਲ.getAsObject()->get("context"))
    ਵਾਪਸ fromJSON(*Context, ਨਤੀਜਾ.context, ਰਸਤਾ.ਖੇਤਰ("context"));
  ਵਾਪਸ ਸੱਚ;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ ParameterInformation &ਮੁੱਲ) {
  ਪ੍ਰਤਿਜ੍ਞਾ((ਮੁੱਲ.labelOffsets || !ਮੁੱਲ.labelString.ਖਾਲੀ_ਥਾਵਾਂ()) &&
         "parameter information label is required");
  llvm::json::Object ਨਤੀਜਾ;
  ਜੇ (ਮੁੱਲ.labelOffsets)
    ਨਤੀਜਾ["label"] = llvm::json::Array(
        {ਮੁੱਲ.labelOffsets->ਪਹਿਲਾ_2, ਮੁੱਲ.labelOffsets->ਦੂਜਾ_2});
  ਨਹੀਂਤਾਂ
    ਨਤੀਜਾ["label"] = ਮੁੱਲ.labelString;
  ਜੇ (!ਮੁੱਲ.documentation.ਖਾਲੀ_ਥਾਵਾਂ())
    ਨਤੀਜਾ["documentation"] = ਮੁੱਲ.documentation;
  ਵਾਪਸ std::move(ਨਤੀਜਾ);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ SignatureInformation &ਮੁੱਲ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!ਮੁੱਲ.label.ਖਾਲੀ_ਥਾਵਾਂ() && "signature information label is required");
  llvm::json::Object ਨਤੀਜਾ{
      {"label", ਮੁੱਲ.label},
      {"parameters", llvm::json::Array(ਮੁੱਲ.parameters)},
  };
  ਜੇ (!ਮੁੱਲ.documentation.ਖਾਲੀ_ਥਾਵਾਂ())
    ਨਤੀਜਾ["documentation"] = ਮੁੱਲ.documentation;
  ਵਾਪਸ std::move(ਨਤੀਜਾ);
}

raw_ostream &llvm::lsp::ਸੰਚਾਲਕ<<(raw_ostream &Os,
                                   ਅਚਲ SignatureInformation &ਮੁੱਲ) {
  ਵਾਪਸ Os << ਮੁੱਲ.label << " - " << ਮੰਜ਼ਿਲ_JSON(ਮੁੱਲ);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ SignatureHelp &ਮੁੱਲ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਮੁੱਲ.ਸਰਗਰਮ_Signature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  ਪ੍ਰਤਿਜ੍ਞਾ(ਮੁੱਲ.ਸਰਗਰਮ_Parameter >= 0 &&
         "Unexpected negative value for active parameter index");
  ਵਾਪਸ llvm::json::Object{
      {"activeSignature", ਮੁੱਲ.ਸਰਗਰਮ_Signature},
      {"activeParameter", ਮੁੱਲ.ਸਰਗਰਮ_Parameter},
      {"signatures", llvm::json::Array(ਮੁੱਲ.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         DocumentLinkParams &ਨਤੀਜਾ, llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("textDocument", ਨਤੀਜਾ.ਪਾਠ_Document_2);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ DocumentLink &ਮੁੱਲ) {
  ਵਾਪਸ llvm::json::Object{
      {"range", ਮੁੱਲ.range},
      {"target", ਮੁੱਲ.ਨਿਸ਼ਾਨਾ},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         InlayHintsParams &ਨਤੀਜਾ, llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("textDocument", ਨਤੀਜਾ.ਪਾਠ_Document_2) &&
         O.map("range", ਨਤੀਜਾ.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ InlayHint &ਮੁੱਲ) {
  ਵਾਪਸ llvm::json::Object{{"position", ਮੁੱਲ.position},
                            {"kind", (ਪੂਰਨ)ਮੁੱਲ.ਕਿਸਮ_2},
                            {"label", ਮੁੱਲ.label},
                            {"paddingLeft", ਮੁੱਲ.padding_ਖੱਬਾ},
                            {"paddingRight", ਮੁੱਲ.padding_ਸੱਜਾ}};
}
ਤਾਰਕਿਕ llvm::lsp::ਸੰਚਾਲਕ==(ਅਚਲ InlayHint &Lhs, ਅਚਲ InlayHint &Rhs) {
  ਵਾਪਸ std::tie(Lhs.position, Lhs.ਕਿਸਮ_2, Lhs.label) ==
         std::tie(Rhs.position, Rhs.ਕਿਸਮ_2, Rhs.label);
}
ਤਾਰਕਿਕ llvm::lsp::ਸੰਚਾਲਕ<(ਅਚਲ InlayHint &Lhs, ਅਚਲ InlayHint &Rhs) {
  ਵਾਪਸ std::tie(Lhs.position, Lhs.ਕਿਸਮ_2, Lhs.label) <
         std::tie(Rhs.position, Rhs.ਕਿਸਮ_2, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::ਸੰਚਾਲਕ<<(llvm::raw_ostream &Os,
                                         Inlay_Hint_ਕਿਸਮ ਮੁੱਲ) {
  ਚੋਣ (ਮੁੱਲ) {
  ਹਾਲਤ Inlay_Hint_ਕਿਸਮ::Parameter:
    ਵਾਪਸ Os << "parameter";
  ਹਾਲਤ Inlay_Hint_ਕਿਸਮ::Type:
    ਵਾਪਸ Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         ਕੋਡ_Action_Context &ਨਤੀਜਾ, llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਜੇ (!O || !O.map("diagnostics", ਨਤੀਜਾ.diagnostics))
    ਵਾਪਸ ਝੂਠ;
  O.map("only", ਨਤੀਜਾ.only);
  ਵਾਪਸ ਸੱਚ;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ,
                         ਕੋਡ_Action_Params &ਨਤੀਜਾ, llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("textDocument", ਨਤੀਜਾ.ਪਾਠ_Document_2) &&
         O.map("range", ਨਤੀਜਾ.range) && O.map("context", ਨਤੀਜਾ.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

ਤਾਰਕਿਕ llvm::lsp::fromJSON(ਅਚਲ llvm::json::ਮੁੱਲ &ਮੁੱਲ, WorkspaceEdit &ਨਤੀਜਾ,
                         llvm::json::ਰਸਤਾ ਰਸਤਾ) {
  llvm::json::ObjectMapper O(ਮੁੱਲ, ਰਸਤਾ);
  ਵਾਪਸ O && O.map("changes", ਨਤੀਜਾ.changes);
}

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ WorkspaceEdit &ਮੁੱਲ) {
  llvm::json::Object FileChanges;
  ਲਈ (ਸਵੈਚਾਲਿਤ &Change : ਮੁੱਲ.changes)
    FileChanges[Change.ਪਹਿਲਾ_2] = llvm::json::Array(Change.ਦੂਜਾ_2);
  ਵਾਪਸ llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ਅਚਲ llvm::StringLiteral ਕੋਡ_Action::k_ਤੇਜ਼_Fix = "quickfix";
ਅਚਲ llvm::StringLiteral ਕੋਡ_Action::kRefactor = "refactor";
ਅਚਲ llvm::StringLiteral ਕੋਡ_Action::kInfo = "info";

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ ਕੋਡ_Action &ਮੁੱਲ) {
  llvm::json::Object ਕੋਡ_Action{{"title", ਮੁੱਲ.title}};
  ਜੇ (ਮੁੱਲ.ਕਿਸਮ_2)
    ਕੋਡ_Action["kind"] = *ਮੁੱਲ.ਕਿਸਮ_2;
  ਜੇ (ਮੁੱਲ.diagnostics)
    ਕੋਡ_Action["diagnostics"] = llvm::json::Array(*ਮੁੱਲ.diagnostics);
  ਜੇ (ਮੁੱਲ.isPreferred)
    ਕੋਡ_Action["isPreferred"] = ਸੱਚ;
  ਜੇ (ਮੁੱਲ.edit)
    ਕੋਡ_Action["edit"] = *ਮੁੱਲ.edit;
  ਵਾਪਸ std::move(ਕੋਡ_Action);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ Show_ਸੁਨੇਹਾ_Params &Params) {
  ਸਵੈਚਾਲਿਤ ਬਾਹਰਲਾ = llvm::json::Object{
      {"type", static_cast<ਪੂਰਨ>(Params.type)},
      {"message", Params.ਸੁਨੇਹਾ_2},
  };
  ਜੇ (Params.actions)
    ਬਾਹਰਲਾ["actions"] = *Params.actions;
  ਵਾਪਸ ਬਾਹਰਲਾ;
}

llvm::json::ਮੁੱਲ llvm::lsp::ਮੰਜ਼ਿਲ_JSON(ਅਚਲ ਸੁਨੇਹਾ_Action_ਵਸਤੂ &Params) {
  ਵਾਪਸ llvm::json::Object{{"title", Params.title}};
}

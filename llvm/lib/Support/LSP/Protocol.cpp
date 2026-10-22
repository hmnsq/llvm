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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
ಮಾದರಿ <ಬಗೆ_ಹೆಸರು T>
ಸ್ಥಿರ ತಾರ್ಕಿಕ mapOptOrNull(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &Params,
                         llvm::StringLiteral Prop, T &ಹೊರಹೋಗು,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  ಸ್ಥಿರಾಂಕ llvm::json::Object *O = Params.getAsObject();
  ಪ್ರತಿಜ್ಞಾ(O);

  // Field is missing or null.
  ಸ್ವಯಂಚಾಲಿತ *V = O->get(Prop);
  ಆದರೆ (!V || V->getAsNull())
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಹಿಂತಿರುಗಿ fromJSON(*V, ಹೊರಹೋಗು, ಮಾರ್ಗ.ಕ್ಷೇತ್ರ(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

ಅಕ್ಷರ LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

ಸ್ಥಿರ ತಾರ್ಕಿಕ is_Windows_ಮಾರ್ಗ(StringRef ಮಾರ್ಗ) {
  ಹಿಂತಿರುಗಿ ಮಾರ್ಗ.ಗಾತ್ರ_3() > 1 && llvm::isAlpha(ಮಾರ್ಗ[0]) && ಮಾರ್ಗ[1] == ':';
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ is_ಜಾಲ_ಮಾರ್ಗ(StringRef ಮಾರ್ಗ) {
  ಹಿಂತಿರುಗಿ ಮಾರ್ಗ.ಗಾತ್ರ_3() > 2 && ಮಾರ್ಗ[0] == ಮಾರ್ಗ[1] &&
         llvm::sys::ಮಾರ್ಗ_2::is_separator(ಮಾರ್ಗ[0]);
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ should_Escape_ಒಳ_URI(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ C) {
  // Unreserved characters.
  ಆದರೆ ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಆಯ್ಕೆ (C) {
  ಸ್ಥಿತಿ '-':
  ಸ್ಥಿತಿ '_':
  ಸ್ಥಿತಿ '.':
  ಸ್ಥಿತಿ '~':
  // '/' is only reserved when parsing.
  ಸ್ಥಿತಿ '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  ಸ್ಥಿತಿ ':':
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
ಸ್ಥಿರ ಶೂನ್ಯ percentEncode(StringRef Content, std::string &ಹೊರಹೋಗು) {
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ C : Content) {
    ಆದರೆ (should_Escape_ಒಳ_URI(C)) {
      ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ('%');
      ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(llvm::hexdigit(C / 16));
      ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(llvm::hexdigit(C % 16));
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ಸ್ಥಿರ std::string percentDecode(StringRef Content) {
  std::string ಫಲಿತಾಂಶ;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ I = Content.begin(), E = Content.ಕೊನೆ(); I != E; ++I) {
    ಆದರೆ (*I == '%' && I + 2 < Content.ಕೊನೆ() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      ಫಲಿತಾಂಶ.ತಳ್ಳು_ಹಿಂಭಾಗ(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಫಲಿತಾಂಶ.ತಳ್ಳು_ಹಿಂಭಾಗ(*I);
    }
  }
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

/// Return the set containing the supported URI schemes.
ಸ್ಥಿರ String_ಗಣ<> &getSupportedSchemes() {
  ಸ್ಥಿರ String_ಗಣ<> Schemes({"file", "test"});
  ಹಿಂತಿರುಗಿ Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
ಸ್ಥಿರ ತಾರ್ಕಿಕ isStructurallyValidScheme(StringRef Scheme) {
  ಆದರೆ (Scheme.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (!llvm::isAlpha(Scheme[0]))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ llvm::all_ದ(llvm::drop_begin(Scheme), [](ಅಕ್ಷರ C) {
    ಹಿಂತಿರುಗಿ llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

ಸ್ಥಿರ llvm::Expected<std::string> uri_From_Absolute_ಮಾರ್ಗ(StringRef Absolute_ಮಾರ್ಗ,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef ಬೇರು_2 = llvm::sys::ಮಾರ್ಗ_2::ಬೇರು_name(Absolute_ಮಾರ್ಗ);
  ಆದರೆ (is_ಜಾಲ_ಮಾರ್ಗ(ಬೇರು_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = ಬೇರು_2.drop_front(2);
    Absolute_ಮಾರ್ಗ.consume_front(ಬೇರು_2);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (is_Windows_ಮಾರ್ಗ(ಬೇರು_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::ಮಾರ್ಗ_2::convert_ಗಮ್ಯ_slash(Absolute_ಮಾರ್ಗ);

  std::string Uri = Scheme.str() + ":";
  ಆದರೆ (Authority.ಖಾಲಿಗಳು() && Body.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  ಆದರೆ (!Authority.ಖಾಲಿಗಳು() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  ಹಿಂತಿರುಗಿ Uri;
}

ಸ್ಥಿರ llvm::Expected<std::string> get_Absolute_ಮಾರ್ಗ(StringRef Authority,
                                                   StringRef Body) {
  ಆದರೆ (!Body.starts_with("/"))
    ಹಿಂತಿರುಗಿ llvm::createStringError(
        llvm::inconvertible_Error_ಸಂಕೇತ(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> ಮಾರ್ಗ;
  ಆದರೆ (!Authority.ಖಾಲಿಗಳು()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).ಗಮ್ಯ_ಸದಿಶ(ಮಾರ್ಗ);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (is_Windows_ಮಾರ್ಗ(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  ಮಾರ್ಗ.append(Body);
  llvm::sys::ಮಾರ್ಗ_2::native(ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ std::string(ಮಾರ್ಗ);
}

ಸ್ಥಿರ llvm::Expected<std::string> parse_File_ಮಾರ್ಗ_From_URI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.ಕಂಡುಹಿಡಿ(':');
  ಆದರೆ (Pos == StringRef::npos)
    ಹಿಂತಿರುಗಿ llvm::createStringError(llvm::inconvertible_Error_ಸಂಕೇತ(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  ಆದರೆ (!isStructurallyValidScheme(UriScheme))
    ಹಿಂತಿರುಗಿ llvm::createStringError(llvm::inconvertible_Error_ಸಂಕೇತ(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  ಆದರೆ (Uri.consume_front("//")) {
    Pos = Uri.ಕಂಡುಹಿಡಿ('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  ಆದರೆ (!getSupportedSchemes().ಒಳಗೊಂಡಿದೆ(UriScheme)) {
    ಹಿಂತಿರುಗಿ llvm::createStringError(llvm::inconvertible_Error_ಸಂಕೇತ(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  ಹಿಂತಿರುಗಿ get_Absolute_ಮಾರ್ಗ(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> File_ಮಾರ್ಗ = parse_File_ಮಾರ್ಗ_From_URI(Uri);
  ಆದರೆ (!File_ಮಾರ್ಗ)
    ಹಿಂತಿರುಗಿ File_ಮಾರ್ಗ.takeError();
  ಹಿಂತಿರುಗಿ URIForFile(std::move(*File_ಮಾರ್ಗ), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uri_From_Absolute_ಮಾರ್ಗ(AbsoluteFilepath, Scheme);
  ಆದರೆ (!Uri)
    ಹಿಂತಿರುಗಿ Uri.takeError();
  ಹಿಂತಿರುಗಿ fromURI(*Uri);
}

StringRef URIForFile::scheme() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ uri().ವಿಭಜಿಸು(':').ಮೊದಲ_2; }

ಶೂನ್ಯ URIForFile::ರಿಜಿಸ್ಟರ್_Supported_Scheme(StringRef Scheme) {
  getSupportedSchemes().ಸೇರಿಸು(Scheme);
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, URIForFile &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  ಆದರೆ (std::optional<StringRef> Str = ಮೌಲ್ಯ.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    ಆದರೆ (!ExpectedUri) {
      ಮಾರ್ಗ.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }
    ಫಲಿತಾಂಶ = std::move(*ExpectedUri);
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ URIForFile &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ.uri();
}

raw_ostream &llvm::lsp::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &Os, ಸ್ಥಿರಾಂಕ URIForFile &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ Os << ಮೌಲ್ಯ.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         ClientCapabilities &ಫಲಿತಾಂಶ, llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  ಸ್ಥಿರಾಂಕ llvm::json::Object *O = ಮೌಲ್ಯ.getAsObject();
  ಆದರೆ (!O) {
    ಮಾರ್ಗ.report("expected object");
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಆದರೆ (ಸ್ಥಿರಾಂಕ llvm::json::Object *ಪಠ್ಯ_Document = O->getObject("textDocument")) {
    ಆದರೆ (ಸ್ಥಿರಾಂಕ llvm::json::Object *Document_ಚಿಹ್ನೆ =
            ಪಠ್ಯ_Document->getObject("documentSymbol")) {
      ಆದರೆ (std::optional<ತಾರ್ಕಿಕ> ಶ್ರೇಣೀಕೃತ_Support =
              Document_ಚಿಹ್ನೆ->getBoolean("hierarchicalDocumentSymbolSupport"))
        ಫಲಿತಾಂಶ.ಶ್ರೇಣೀಕೃತ_Document_ಚಿಹ್ನೆ = *ಶ್ರೇಣೀಕೃತ_Support;
    }
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ಸಂಕೇತ_Action = ಪಠ್ಯ_Document->getObject("codeAction")) {
      ಆದರೆ (ಸಂಕೇತ_Action->getObject("codeActionLiteralSupport"))
        ಫಲಿತಾಂಶ.ಸಂಕೇತ_Action_Structure = ನಿಜ;
    }
  }
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ಕಿಟಕಿ = O->getObject("window")) {
    ಆದರೆ (std::optional<ತಾರ್ಕಿಕ> Work_ಮುಗಿದ_ಪ್ರಗತಿ_Support =
            ಕಿಟಕಿ->getBoolean("workDoneProgress"))
      ಫಲಿತಾಂಶ.work_ಮುಗಿದ_ಪ್ರಗತಿ = *Work_ಮುಗಿದ_ಪ್ರಗತಿ_Support;
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, ClientInfo &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಆದರೆ (!O || !O.map("name", ಫಲಿತಾಂಶ.name))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // Don't fail if we can't parse version.
  O.map("version", ಫಲಿತಾಂಶ.ಆವೃತ್ತಿ_2);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, TraceLevel &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  ಆದರೆ (std::optional<StringRef> Str = ಮೌಲ್ಯ.getAsString()) {
    ಆದರೆ (*Str == "off") {
      ಫಲಿತಾಂಶ = TraceLevel::Off;
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
    ಆದರೆ (*Str == "messages") {
      ಫಲಿತಾಂಶ = TraceLevel::Messages;
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
    ಆದರೆ (*Str == "verbose") {
      ಫಲಿತಾಂಶ = TraceLevel::Verbose;
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         InitializeParams &ಫಲಿತಾಂಶ, llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಆದರೆ (!O)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", ಫಲಿತಾಂಶ.capabilities);
  O.map("trace", ಫಲಿತಾಂಶ.trace);
  O.map("rootUri", ಫಲಿತಾಂಶ.ಬೇರು_Uri);
  O.map("rootPath", ಫಲಿತಾಂಶ.ಬೇರು_ಮಾರ್ಗ);
  mapOptOrNull(ಮೌಲ್ಯ, "clientInfo", ಫಲಿತಾಂಶ.clientInfo, ಮಾರ್ಗ);

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         ಪಠ್ಯ_Document_ಅಂಶ &ಫಲಿತಾಂಶ, llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("uri", ಫಲಿತಾಂಶ.uri) &&
         O.map("languageId", ಫಲಿತಾಂಶ.languageId) && O.map("text", ಫಲಿತಾಂಶ.ಪಠ್ಯ) &&
         O.map("version", ಫಲಿತಾಂಶ.ಆವೃತ್ತಿ_2);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ ಪಠ್ಯ_Document_Identifier &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ llvm::json::Object{{"uri", ಮೌಲ್ಯ.uri}};
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         ಪಠ್ಯ_Document_Identifier &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("uri", ಫಲಿತಾಂಶ.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ
llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ Versioned_ಪಠ್ಯ_Document_Identifier &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ llvm::json::Object{
      {"uri", ಮೌಲ್ಯ.uri},
      {"version", ಮೌಲ್ಯ.ಆವೃತ್ತಿ_2},
  };
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         Versioned_ಪಠ್ಯ_Document_Identifier &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("uri", ಫಲಿತಾಂಶ.uri) && O.map("version", ಫಲಿತಾಂಶ.ಆವೃತ್ತಿ_2);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, Position &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("line", ಫಲಿತಾಂಶ.ಸಾಲು_2) &&
         O.map("character", ಫಲಿತಾಂಶ.character);
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ Position &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ llvm::json::Object{
      {"line", ಮೌಲ್ಯ.ಸಾಲು_2},
      {"character", ಮೌಲ್ಯ.character},
  };
}

raw_ostream &llvm::lsp::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &Os, ಸ್ಥಿರಾಂಕ Position &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ Os << ಮೌಲ್ಯ.ಸಾಲು_2 << ':' << ಮೌಲ್ಯ.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, Range &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("start", ಫಲಿತಾಂಶ.ಆರಂಭ_2) && O.map("end", ಫಲಿತಾಂಶ.ಕೊನೆ);
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ Range &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ llvm::json::Object{
      {"start", ಮೌಲ್ಯ.ಆರಂಭ_2},
      {"end", ಮೌಲ್ಯ.ಕೊನೆ},
  };
}

raw_ostream &llvm::lsp::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &Os, ಸ್ಥಿರಾಂಕ Range &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ Os << ಮೌಲ್ಯ.ಆರಂಭ_2 << '-' << ಮೌಲ್ಯ.ಕೊನೆ;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, Location &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("uri", ಫಲಿತಾಂಶ.uri) && O.map("range", ಫಲಿತಾಂಶ.range);
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ Location &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ llvm::json::Object{
      {"uri", ಮೌಲ್ಯ.uri},
      {"range", ಮೌಲ್ಯ.range},
  };
}

raw_ostream &llvm::lsp::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &Os, ಸ್ಥಿರಾಂಕ Location &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ Os << ಮೌಲ್ಯ.range << '@' << ಮೌಲ್ಯ.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         ಪಠ್ಯ_Document_Position_Params &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("textDocument", ಫಲಿತಾಂಶ.ಪಠ್ಯ_Document_2) &&
         O.map("position", ಫಲಿತಾಂಶ.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         ReferenceContext &ಫಲಿತಾಂಶ, llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.mapOptional("includeDeclaration", ಫಲಿತಾಂಶ.includeDeclaration);
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         ReferenceParams &ಫಲಿತಾಂಶ, llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  ಪಠ್ಯ_Document_Position_Params &ತಳ = ಫಲಿತಾಂಶ;
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ fromJSON(ಮೌಲ್ಯ, ತಳ, ಮಾರ್ಗ) && O &&
         O.mapOptional("context", ಫಲಿತಾಂಶ.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         Did_Open_ಪಠ್ಯ_Document_Params &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("textDocument", ಫಲಿತಾಂಶ.ಪಠ್ಯ_Document_2);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         Did_Close_ಪಠ್ಯ_Document_Params &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("textDocument", ಫಲಿತಾಂಶ.ಪಠ್ಯ_Document_2);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &Params,
                         Did_Save_ಪಠ್ಯ_Document_Params &R, llvm::json::ಮಾರ್ಗ P) {
  llvm::json::ObjectMapper O(Params, P);
  ಹಿಂತಿರುಗಿ O && O.map("textDocument", R.ಪಠ್ಯ_Document_2);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

Logical_ಫಲಿತಾಂಶ
ಪಠ್ಯ_Document_Content_Change_Event::apply_ಗಮ್ಯ(std::string &Contents) ಸ್ಥಿರಾಂಕ {
  // If there is no range, the full document changed.
  ಆದರೆ (!range) {
    Contents = ಪಠ್ಯ;
    ಹಿಂತಿರುಗಿ success();
  }

  // Try to map the replacement range to the content.
  llvm::ಮೂಲ_Mgr TmpScrMgr;
  TmpScrMgr.ಕೂಡಿಸು_New_ಮೂಲ_ಬಫರ್(llvm::ಸ್ಮರಣೆ_ಬಫರ್::get_Mem_ಬಫರ್(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  ಆದರೆ (!RangeLoc.isValid())
    ಹಿಂತಿರುಗಿ failure();

  Contents.replace(RangeLoc.ಆರಂಭ.get_ಸೂಚಕ() - Contents.ದತ್ತಾಂಶ(),
                   RangeLoc.ಕೊನೆ_2.get_ಸೂಚಕ() - RangeLoc.ಆರಂಭ.get_ಸೂಚಕ(),
                   ಪಠ್ಯ);
  ಹಿಂತಿರುಗಿ success();
}

Logical_ಫಲಿತಾಂಶ ಪಠ್ಯ_Document_Content_Change_Event::apply_ಗಮ್ಯ(
    ArrayRef<ಪಠ್ಯ_Document_Content_Change_Event> Changes, std::string &Contents) {
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &Change : Changes)
    ಆದರೆ (failed(Change.apply_ಗಮ್ಯ(Contents)))
      ಹಿಂತಿರುಗಿ failure();
  ಹಿಂತಿರುಗಿ success();
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         ಪಠ್ಯ_Document_Content_Change_Event &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("range", ಫಲಿತಾಂಶ.range) &&
         O.map("rangeLength", ಫಲಿತಾಂಶ.range_ಉದ್ದಳತೆ) && O.map("text", ಫಲಿತಾಂಶ.ಪಠ್ಯ);
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         Did_Change_ಪಠ್ಯ_Document_Params &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("textDocument", ಫಲಿತಾಂಶ.ಪಠ್ಯ_Document_2) &&
         O.map("contentChanges", ಫಲಿತಾಂಶ.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

ಸ್ಥಿರ llvm::StringRef ಗಮ್ಯ_ಪಠ್ಯ_ಬಗೆ(Markup_ಬಗೆ ಬಗೆ_2) {
  ಆಯ್ಕೆ (ಬಗೆ_2) {
  ಸ್ಥಿತಿ Markup_ಬಗೆ::Plain_ಪಠ್ಯ:
    ಹಿಂತಿರುಗಿ "plaintext";
  ಸ್ಥಿತಿ Markup_ಬಗೆ::Markdown:
    ಹಿಂತಿರುಗಿ "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &Os, Markup_ಬಗೆ ಬಗೆ_2) {
  ಹಿಂತಿರುಗಿ Os << ಗಮ್ಯ_ಪಠ್ಯ_ಬಗೆ(ಬಗೆ_2);
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ MarkupContent &Mc) {
  ಆದರೆ (Mc.ಮೌಲ್ಯ_2.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

  ಹಿಂತಿರುಗಿ llvm::json::Object{
      {"kind", ಗಮ್ಯ_ಪಠ್ಯ_ಬಗೆ(Mc.ಬಗೆ)},
      {"value", Mc.ಮೌಲ್ಯ_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ Hover &Hover) {
  llvm::json::Object ಫಲಿತಾಂಶ{{"contents", ಗಮ್ಯ_JSON(Hover.contents)}};
  ಆದರೆ (Hover.range)
    ಫಲಿತಾಂಶ["range"] = ಗಮ್ಯ_JSON(*Hover.range);
  ಹಿಂತಿರುಗಿ std::move(ಫಲಿತಾಂಶ);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ Document_ಚಿಹ್ನೆ &ಚಿಹ್ನೆ) {
  llvm::json::Object ಫಲಿತಾಂಶ{{"name", ಚಿಹ್ನೆ.name},
                            {"kind", static_cast<ಪೂರ್ಣಾಂಕ>(ಚಿಹ್ನೆ.ಬಗೆ)},
                            {"range", ಚಿಹ್ನೆ.range},
                            {"selectionRange", ಚಿಹ್ನೆ.selectionRange}};

  ಆದರೆ (!ಚಿಹ್ನೆ.detail.ಖಾಲಿಗಳು())
    ಫಲಿತಾಂಶ["detail"] = ಚಿಹ್ನೆ.detail;
  ಆದರೆ (!ಚಿಹ್ನೆ.children.ಖಾಲಿಗಳು())
    ಫಲಿತಾಂಶ["children"] = ಚಿಹ್ನೆ.children;
  ಹಿಂತಿರುಗಿ std::move(ಫಲಿತಾಂಶ);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         Document_ಚಿಹ್ನೆ_Params &ಫಲಿತಾಂಶ, llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("textDocument", ಫಲಿತಾಂಶ.ಪಠ್ಯ_Document_2);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         DiagnosticRelatedInformation &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("location", ಫಲಿತಾಂಶ.location) &&
         O.map("message", ಫಲಿತಾಂಶ.ಸಂದೇಶ_2);
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ DiagnosticRelatedInformation &Info) {
  ಹಿಂತಿರುಗಿ llvm::json::Object{
      {"location", Info.location},
      {"message", Info.ಸಂದೇಶ_2},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(Diagnostic_ಟ್ಯಾಗ್ ಟ್ಯಾಗ್) {
  ಹಿಂತಿರುಗಿ static_cast<ಪೂರ್ಣಾಂಕ>(ಟ್ಯಾಗ್);
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, Diagnostic_ಟ್ಯಾಗ್ &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  ಆದರೆ (std::optional<int64_t> I = ಮೌಲ್ಯ.getAsInteger()) {
    ಫಲಿತಾಂಶ = (Diagnostic_ಟ್ಯಾಗ್)*I;
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ Diagnostic &Diag) {
  llvm::json::Object ಫಲಿತಾಂಶ{
      {"range", Diag.range},
      {"severity", (ಪೂರ್ಣಾಂಕ)Diag.severity},
      {"message", Diag.ಸಂದೇಶ_2},
  };
  ಆದರೆ (Diag.category)
    ಫಲಿತಾಂಶ["category"] = *Diag.category;
  ಆದರೆ (!Diag.ಮೂಲ_2.ಖಾಲಿಗಳು())
    ಫಲಿತಾಂಶ["source"] = Diag.ಮೂಲ_2;
  ಆದರೆ (Diag.relatedInformation)
    ಫಲಿತಾಂಶ["relatedInformation"] = *Diag.relatedInformation;
  ಆದರೆ (!Diag.ಟ್ಯಾಗುಗಳು.ಖಾಲಿಗಳು())
    ಫಲಿತಾಂಶ["tags"] = Diag.ಟ್ಯಾಗುಗಳು;
  ಹಿಂತಿರುಗಿ std::move(ಫಲಿತಾಂಶ);
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, Diagnostic &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಆದರೆ (!O)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಪೂರ್ಣಾಂಕ Severity = 0;
  ಆದರೆ (!mapOptOrNull(ಮೌಲ್ಯ, "severity", Severity, ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಫಲಿತಾಂಶ.severity = (DiagnosticSeverity)Severity;

  ಹಿಂತಿರುಗಿ O.map("range", ಫಲಿತಾಂಶ.range) && O.map("message", ಫಲಿತಾಂಶ.ಸಂದೇಶ_2) &&
         mapOptOrNull(ಮೌಲ್ಯ, "category", ಫಲಿತಾಂಶ.category, ಮಾರ್ಗ) &&
         mapOptOrNull(ಮೌಲ್ಯ, "source", ಫಲಿತಾಂಶ.ಮೂಲ_2, ಮಾರ್ಗ) &&
         mapOptOrNull(ಮೌಲ್ಯ, "relatedInformation", ಫಲಿತಾಂಶ.relatedInformation,
                      ಮಾರ್ಗ) &&
         mapOptOrNull(ಮೌಲ್ಯ, "tags", ಫಲಿತಾಂಶ.ಟ್ಯಾಗುಗಳು, ಮಾರ್ಗ);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ PublishDiagnosticsParams &Params) {
  ಹಿಂತಿರುಗಿ llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.ಆವೃತ್ತಿ_2},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, ಪಠ್ಯ_Edit &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("range", ಫಲಿತಾಂಶ.range) && O.map("newText", ಫಲಿತಾಂಶ.new_ಪಠ್ಯ);
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ ಪಠ್ಯ_Edit &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ llvm::json::Object{
      {"range", ಮೌಲ್ಯ.range},
      {"newText", ಮೌಲ್ಯ.new_ಪಠ್ಯ},
  };
}

raw_ostream &llvm::lsp::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &Os, ಸ್ಥಿರಾಂಕ ಪಠ್ಯ_Edit &ಮೌಲ್ಯ) {
  Os << ಮೌಲ್ಯ.range << " => \"";
  llvm::printEscapedString(ಮೌಲ್ಯ.new_ಪಠ್ಯ, Os);
  ಹಿಂತಿರುಗಿ Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         Completion_ಅಂಶ_ಬಗೆ &ಫಲಿತಾಂಶ, llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  ಆದರೆ (std::optional<int64_t> Int_ಮೌಲ್ಯ = ಮೌಲ್ಯ.getAsInteger()) {
    ಆದರೆ (*Int_ಮೌಲ್ಯ < static_cast<ಪೂರ್ಣಾಂಕ>(Completion_ಅಂಶ_ಬಗೆ::ಪಠ್ಯ_2) ||
        *Int_ಮೌಲ್ಯ > static_cast<ಪೂರ್ಣಾಂಕ>(Completion_ಅಂಶ_ಬಗೆ::TypeParameter))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    ಫಲಿತಾಂಶ = static_cast<Completion_ಅಂಶ_ಬಗೆ>(*Int_ಮೌಲ್ಯ);
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

Completion_ಅಂಶ_ಬಗೆ llvm::lsp::adjust_ಬಗೆ_ಗಮ್ಯ_Capability(
    Completion_ಅಂಶ_ಬಗೆ ಬಗೆ_2,
    Completion_ಅಂಶ_ಬಗೆ_Bitset &Supported_Completion_ಅಂಶ_Kinds) {
  size_t ಬಗೆ_Val = static_cast<size_t>(ಬಗೆ_2);
  ಆದರೆ (ಬಗೆ_Val >= k_Completion_ಅಂಶ_ಬಗೆ_Min &&
      ಬಗೆ_Val <= Supported_Completion_ಅಂಶ_Kinds.ಗಾತ್ರ_3() &&
      Supported_Completion_ಅಂಶ_Kinds[ಬಗೆ_Val])
    ಹಿಂತಿರುಗಿ ಬಗೆ_2;

  // Provide some fall backs for common kinds that are close enough.
  ಆಯ್ಕೆ (ಬಗೆ_2) {
  ಸ್ಥಿತಿ Completion_ಅಂಶ_ಬಗೆ::Folder:
    ಹಿಂತಿರುಗಿ Completion_ಅಂಶ_ಬಗೆ::File;
  ಸ್ಥಿತಿ Completion_ಅಂಶ_ಬಗೆ::EnumMember:
    ಹಿಂತಿರುಗಿ Completion_ಅಂಶ_ಬಗೆ::Enum;
  ಸ್ಥಿತಿ Completion_ಅಂಶ_ಬಗೆ::Struct:
    ಹಿಂತಿರುಗಿ Completion_ಅಂಶ_ಬಗೆ::Class;
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಹಿಂತಿರುಗಿ Completion_ಅಂಶ_ಬಗೆ::ಪಠ್ಯ_2;
  }
}

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         Completion_ಅಂಶ_ಬಗೆ_Bitset &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  ಆದರೆ (ಸ್ಥಿರಾಂಕ llvm::json::Array *Array_ಮೌಲ್ಯ = ಮೌಲ್ಯ.getAsArray()) {
    ಗಾಗಿ (size_t I = 0, E = Array_ಮೌಲ್ಯ->ಗಾತ್ರ_3(); I < E; ++I) {
      Completion_ಅಂಶ_ಬಗೆ ಬಗೆ_ಹೊರಹೋಗು;
      ಆದರೆ (fromJSON((*Array_ಮೌಲ್ಯ)[I], ಬಗೆ_ಹೊರಹೋಗು, ಮಾರ್ಗ.ಸೂಚ್ಯಂಕ(I)))
        ಫಲಿತಾಂಶ.ಗಣ(size_t(ಬಗೆ_ಹೊರಹೋಗು));
    }
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ Completion_ಅಂಶ &ಮೌಲ್ಯ) {
  ಪ್ರತಿಜ್ಞಾ(!ಮೌಲ್ಯ.label.ಖಾಲಿಗಳು() && "completion item label is required");
  llvm::json::Object ಫಲಿತಾಂಶ{{"label", ಮೌಲ್ಯ.label}};
  ಆದರೆ (ಮೌಲ್ಯ.ಬಗೆ != Completion_ಅಂಶ_ಬಗೆ::Missing)
    ಫಲಿತಾಂಶ["kind"] = static_cast<ಪೂರ್ಣಾಂಕ>(ಮೌಲ್ಯ.ಬಗೆ);
  ಆದರೆ (!ಮೌಲ್ಯ.detail.ಖಾಲಿಗಳು())
    ಫಲಿತಾಂಶ["detail"] = ಮೌಲ್ಯ.detail;
  ಆದರೆ (ಮೌಲ್ಯ.documentation)
    ಫಲಿತಾಂಶ["documentation"] = ಮೌಲ್ಯ.documentation;
  ಆದರೆ (!ಮೌಲ್ಯ.ವಿಂಗಡಿಸು_ಪಠ್ಯ.ಖಾಲಿಗಳು())
    ಫಲಿತಾಂಶ["sortText"] = ಮೌಲ್ಯ.ವಿಂಗಡಿಸು_ಪಠ್ಯ;
  ಆದರೆ (!ಮೌಲ್ಯ.filter_ಪಠ್ಯ.ಖಾಲಿಗಳು())
    ಫಲಿತಾಂಶ["filterText"] = ಮೌಲ್ಯ.filter_ಪಠ್ಯ;
  ಆದರೆ (!ಮೌಲ್ಯ.ಸೇರಿಸು_ಪಠ್ಯ.ಖಾಲಿಗಳು())
    ಫಲಿತಾಂಶ["insertText"] = ಮೌಲ್ಯ.ಸೇರಿಸು_ಪಠ್ಯ;
  ಆದರೆ (ಮೌಲ್ಯ.ಸೇರಿಸು_ಪಠ್ಯ_Format != ಸೇರಿಸು_ಪಠ್ಯ_Format_2::Missing)
    ಫಲಿತಾಂಶ["insertTextFormat"] = static_cast<ಪೂರ್ಣಾಂಕ>(ಮೌಲ್ಯ.ಸೇರಿಸು_ಪಠ್ಯ_Format);
  ಆದರೆ (ಮೌಲ್ಯ.ಪಠ್ಯ_Edit_2)
    ಫಲಿತಾಂಶ["textEdit"] = *ಮೌಲ್ಯ.ಪಠ್ಯ_Edit_2;
  ಆದರೆ (!ಮೌಲ್ಯ.additional_ಪಠ್ಯ_Edits.ಖಾಲಿಗಳು()) {
    ಫಲಿತಾಂಶ["additionalTextEdits"] =
        llvm::json::Array(ಮೌಲ್ಯ.additional_ಪಠ್ಯ_Edits);
  }
  ಆದರೆ (ಮೌಲ್ಯ.deprecated)
    ಫಲಿತಾಂಶ["deprecated"] = ಮೌಲ್ಯ.deprecated;
  ಹಿಂತಿರುಗಿ std::move(ಫಲಿತಾಂಶ);
}

raw_ostream &llvm::lsp::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &Os,
                                   ಸ್ಥಿರಾಂಕ Completion_ಅಂಶ &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ Os << ಮೌಲ್ಯ.label << " - " << ಗಮ್ಯ_JSON(ಮೌಲ್ಯ);
}

ತಾರ್ಕಿಕ llvm::lsp::ಕ್ರಿಯಾಕಾರಕ<(ಸ್ಥಿರಾಂಕ Completion_ಅಂಶ &Lhs,
                          ಸ್ಥಿರಾಂಕ Completion_ಅಂಶ &Rhs) {
  ಹಿಂತಿರುಗಿ (Lhs.ವಿಂಗಡಿಸು_ಪಠ್ಯ.ಖಾಲಿಗಳು() ? Lhs.label : Lhs.ವಿಂಗಡಿಸು_ಪಠ್ಯ) <
         (Rhs.ವಿಂಗಡಿಸು_ಪಠ್ಯ.ಖಾಲಿಗಳು() ? Rhs.label : Rhs.ವಿಂಗಡಿಸು_ಪಠ್ಯ);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ CompletionList &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ llvm::json::Object{
      {"isIncomplete", ಮೌಲ್ಯ.isIncomplete},
      {"items", llvm::json::Array(ಮೌಲ್ಯ.ವಸ್ತುಗಳು)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         CompletionContext &ಫಲಿತಾಂಶ, llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಪೂರ್ಣಾಂಕ Trigger_ಬಗೆ;
  ಆದರೆ (!O || !O.map("triggerKind", Trigger_ಬಗೆ) ||
      !mapOptOrNull(ಮೌಲ್ಯ, "triggerCharacter", ಫಲಿತಾಂಶ.triggerCharacter, ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಫಲಿತಾಂಶ.trigger_ಬಗೆ = static_cast<Completion_Trigger_ಬಗೆ>(Trigger_ಬಗೆ);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         CompletionParams &ಫಲಿತಾಂಶ, llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  ಆದರೆ (!fromJSON(ಮೌಲ್ಯ, static_cast<ಪಠ್ಯ_Document_Position_Params &>(ಫಲಿತಾಂಶ), ಮಾರ್ಗ))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ *Context = ಮೌಲ್ಯ.getAsObject()->get("context"))
    ಹಿಂತಿರುಗಿ fromJSON(*Context, ಫಲಿತಾಂಶ.context, ಮಾರ್ಗ.ಕ್ಷೇತ್ರ("context"));
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ ParameterInformation &ಮೌಲ್ಯ) {
  ಪ್ರತಿಜ್ಞಾ((ಮೌಲ್ಯ.labelOffsets || !ಮೌಲ್ಯ.labelString.ಖಾಲಿಗಳು()) &&
         "parameter information label is required");
  llvm::json::Object ಫಲಿತಾಂಶ;
  ಆದರೆ (ಮೌಲ್ಯ.labelOffsets)
    ಫಲಿತಾಂಶ["label"] = llvm::json::Array(
        {ಮೌಲ್ಯ.labelOffsets->ಮೊದಲ_2, ಮೌಲ್ಯ.labelOffsets->ಎರಡನೇ_2});
  ಇಲ್ಲದಿದ್ದರೆ
    ಫಲಿತಾಂಶ["label"] = ಮೌಲ್ಯ.labelString;
  ಆದರೆ (!ಮೌಲ್ಯ.documentation.ಖಾಲಿಗಳು())
    ಫಲಿತಾಂಶ["documentation"] = ಮೌಲ್ಯ.documentation;
  ಹಿಂತಿರುಗಿ std::move(ಫಲಿತಾಂಶ);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ SignatureInformation &ಮೌಲ್ಯ) {
  ಪ್ರತಿಜ್ಞಾ(!ಮೌಲ್ಯ.label.ಖಾಲಿಗಳು() && "signature information label is required");
  llvm::json::Object ಫಲಿತಾಂಶ{
      {"label", ಮೌಲ್ಯ.label},
      {"parameters", llvm::json::Array(ಮೌಲ್ಯ.parameters)},
  };
  ಆದರೆ (!ಮೌಲ್ಯ.documentation.ಖಾಲಿಗಳು())
    ಫಲಿತಾಂಶ["documentation"] = ಮೌಲ್ಯ.documentation;
  ಹಿಂತಿರುಗಿ std::move(ಫಲಿತಾಂಶ);
}

raw_ostream &llvm::lsp::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &Os,
                                   ಸ್ಥಿರಾಂಕ SignatureInformation &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ Os << ಮೌಲ್ಯ.label << " - " << ಗಮ್ಯ_JSON(ಮೌಲ್ಯ);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ SignatureHelp &ಮೌಲ್ಯ) {
  ಪ್ರತಿಜ್ಞಾ(ಮೌಲ್ಯ.ಸಕ್ರಿಯ_Signature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  ಪ್ರತಿಜ್ಞಾ(ಮೌಲ್ಯ.ಸಕ್ರಿಯ_Parameter >= 0 &&
         "Unexpected negative value for active parameter index");
  ಹಿಂತಿರುಗಿ llvm::json::Object{
      {"activeSignature", ಮೌಲ್ಯ.ಸಕ್ರಿಯ_Signature},
      {"activeParameter", ಮೌಲ್ಯ.ಸಕ್ರಿಯ_Parameter},
      {"signatures", llvm::json::Array(ಮೌಲ್ಯ.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         DocumentLinkParams &ಫಲಿತಾಂಶ, llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("textDocument", ಫಲಿತಾಂಶ.ಪಠ್ಯ_Document_2);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ DocumentLink &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ llvm::json::Object{
      {"range", ಮೌಲ್ಯ.range},
      {"target", ಮೌಲ್ಯ.ಗುರಿ},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         InlayHintsParams &ಫಲಿತಾಂಶ, llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("textDocument", ಫಲಿತಾಂಶ.ಪಠ್ಯ_Document_2) &&
         O.map("range", ಫಲಿತಾಂಶ.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ InlayHint &ಮೌಲ್ಯ) {
  ಹಿಂತಿರುಗಿ llvm::json::Object{{"position", ಮೌಲ್ಯ.position},
                            {"kind", (ಪೂರ್ಣಾಂಕ)ಮೌಲ್ಯ.ಬಗೆ},
                            {"label", ಮೌಲ್ಯ.label},
                            {"paddingLeft", ಮೌಲ್ಯ.padding_ಎಡ},
                            {"paddingRight", ಮೌಲ್ಯ.padding_ಬಲ}};
}
ತಾರ್ಕಿಕ llvm::lsp::ಕ್ರಿಯಾಕಾರಕ==(ಸ್ಥಿರಾಂಕ InlayHint &Lhs, ಸ್ಥಿರಾಂಕ InlayHint &Rhs) {
  ಹಿಂತಿರುಗಿ std::tie(Lhs.position, Lhs.ಬಗೆ, Lhs.label) ==
         std::tie(Rhs.position, Rhs.ಬಗೆ, Rhs.label);
}
ತಾರ್ಕಿಕ llvm::lsp::ಕ್ರಿಯಾಕಾರಕ<(ಸ್ಥಿರಾಂಕ InlayHint &Lhs, ಸ್ಥಿರಾಂಕ InlayHint &Rhs) {
  ಹಿಂತಿರುಗಿ std::tie(Lhs.position, Lhs.ಬಗೆ, Lhs.label) <
         std::tie(Rhs.position, Rhs.ಬಗೆ, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::ಕ್ರಿಯಾಕಾರಕ<<(llvm::raw_ostream &Os,
                                         Inlay_Hint_ಬಗೆ ಮೌಲ್ಯ) {
  ಆಯ್ಕೆ (ಮೌಲ್ಯ) {
  ಸ್ಥಿತಿ Inlay_Hint_ಬಗೆ::Parameter:
    ಹಿಂತಿರುಗಿ Os << "parameter";
  ಸ್ಥಿತಿ Inlay_Hint_ಬಗೆ::Type:
    ಹಿಂತಿರುಗಿ Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         ಸಂಕೇತ_Action_Context &ಫಲಿತಾಂಶ, llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಆದರೆ (!O || !O.map("diagnostics", ಫಲಿತಾಂಶ.diagnostics))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  O.map("only", ಫಲಿತಾಂಶ.only);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ,
                         ಸಂಕೇತ_Action_Params &ಫಲಿತಾಂಶ, llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("textDocument", ಫಲಿತಾಂಶ.ಪಠ್ಯ_Document_2) &&
         O.map("range", ಫಲಿತಾಂಶ.range) && O.map("context", ಫಲಿತಾಂಶ.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

ತಾರ್ಕಿಕ llvm::lsp::fromJSON(ಸ್ಥಿರಾಂಕ llvm::json::ಮೌಲ್ಯ &ಮೌಲ್ಯ, WorkspaceEdit &ಫಲಿತಾಂಶ,
                         llvm::json::ಮಾರ್ಗ ಮಾರ್ಗ) {
  llvm::json::ObjectMapper O(ಮೌಲ್ಯ, ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ O && O.map("changes", ಫಲಿತಾಂಶ.changes);
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ WorkspaceEdit &ಮೌಲ್ಯ) {
  llvm::json::Object FileChanges;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &Change : ಮೌಲ್ಯ.changes)
    FileChanges[Change.ಮೊದಲ_2] = llvm::json::Array(Change.ಎರಡನೇ_2);
  ಹಿಂತಿರುಗಿ llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ಸ್ಥಿರಾಂಕ llvm::StringLiteral ಸಂಕೇತ_Action::k_ಶೀಘ್ರ_Fix = "quickfix";
ಸ್ಥಿರಾಂಕ llvm::StringLiteral ಸಂಕೇತ_Action::kRefactor = "refactor";
ಸ್ಥಿರಾಂಕ llvm::StringLiteral ಸಂಕೇತ_Action::kInfo = "info";

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ ಸಂಕೇತ_Action &ಮೌಲ್ಯ) {
  llvm::json::Object ಸಂಕೇತ_Action{{"title", ಮೌಲ್ಯ.title}};
  ಆದರೆ (ಮೌಲ್ಯ.ಬಗೆ)
    ಸಂಕೇತ_Action["kind"] = *ಮೌಲ್ಯ.ಬಗೆ;
  ಆದರೆ (ಮೌಲ್ಯ.diagnostics)
    ಸಂಕೇತ_Action["diagnostics"] = llvm::json::Array(*ಮೌಲ್ಯ.diagnostics);
  ಆದರೆ (ಮೌಲ್ಯ.isPreferred)
    ಸಂಕೇತ_Action["isPreferred"] = ನಿಜ;
  ಆದರೆ (ಮೌಲ್ಯ.edit)
    ಸಂಕೇತ_Action["edit"] = *ಮೌಲ್ಯ.edit;
  ಹಿಂತಿರುಗಿ std::move(ಸಂಕೇತ_Action);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ Show_ಸಂದೇಶ_Params &Params) {
  ಸ್ವಯಂಚಾಲಿತ ಹೊರಹೋಗು = llvm::json::Object{
      {"type", static_cast<ಪೂರ್ಣಾಂಕ>(Params.type)},
      {"message", Params.ಸಂದೇಶ_2},
  };
  ಆದರೆ (Params.actions)
    ಹೊರಹೋಗು["actions"] = *Params.actions;
  ಹಿಂತಿರುಗಿ ಹೊರಹೋಗು;
}

llvm::json::ಮೌಲ್ಯ llvm::lsp::ಗಮ್ಯ_JSON(ಸ್ಥಿರಾಂಕ ಸಂದೇಶ_Action_ಅಂಶ &Params) {
  ಹಿಂತಿರುಗಿ llvm::json::Object{{"title", Params.title}};
}

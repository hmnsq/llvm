//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#അന്തര്ഭാവ "llvm/Support/Mustache.h"
#അന്തര്ഭാവ "llvm/ADT/SmallVector.h"
#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <sstream>

#define DEBUG_TYPE "mustache"

using നാമാകാശ llvm;
using നാമാകാശ llvm::mustache;

നാമാകാശ {

using Accessor = ArrayRef<StringRef>;

സ്ഥിതം തർക്കം isFalsey(സ്ഥിരം json::മൂല്യം &V) {
  തിരികെ V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().മൂല്യം_2()) ||
         (V.getAsArray() && V.getAsArray()->empty());
}

സ്ഥിതം തർക്കം isContextFalsey(സ്ഥിരം json::മൂല്യം *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  എങ്കിൽ (!V)
    തിരികെ സത്യം;
  തിരികെ isFalsey(*V);
}

സ്ഥിതം ശൂന്യം splitAndTrim(StringRef Str, SmallVectorImpl<StringRef> &Tokens) {
  size_t CurrentPos = 0;
  ആകുമ്പോൾ (CurrentPos < Str.size()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    എങ്കിൽ (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.size();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.slice(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t Start = Part.find_first_not_of(" \t\r\n");
    എങ്കിൽ (Start != StringRef::npos) {
      size_t End = Part.find_last_not_of(" \t\r\n");
      Tokens.തള്ളുക_back(Part.slice(Start, End + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

സ്ഥിതം Accessor splitMustacheString(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  SmallVector<StringRef> Tokens;
  എങ്കിൽ (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    Tokens.തള്ളുക_back(".");
  } അല്ലെങ്കിൽ {
    splitAndTrim(Str, Tokens);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.size());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  തിരികെ ArrayRef<StringRef>(ArenaTokens, Tokens.size());
}
} // namespace

നാമാകാശ llvm::mustache {

വർഗ്ഗം MustacheOutputStream : പൊതു raw_ostream {
പൊതു:
  MustacheOutputStream() = സ്ഥിരസ്ഥിതി;
  ~MustacheOutputStream() override = സ്ഥിരസ്ഥിതി;

  മായാ ശൂന്യം suspendIndentation() {}
  മായാ ശൂന്യം resumeIndentation() {}

സ്വകാര്യ:
  ശൂന്യം anchor() override;
};

ശൂന്യം MustacheOutputStream::anchor() {}

വർഗ്ഗം RawMustacheOutputStream : പൊതു MustacheOutputStream {
പൊതു:
  RawMustacheOutputStream(raw_ostream &OS) : OS(OS) { SetUnbuffered(); }

സ്വകാര്യ:
  raw_ostream &OS;

  ശൂന്യം write_impl(സ്ഥിരം അക്ഷരം *Ptr, size_t Size) override {
    OS.ലിഖ(Ptr, Size);
  }
  uint64_t current_pos() സ്ഥിരം override { തിരികെ OS.tell(); }
};

വർഗ്ഗം Token {
പൊതു:
  എണ്ണൽ വർഗ്ഗം Type {
    Text,
    Variable,
    Partial,
    SectionOpen,
    SectionClose,
    InvertSectionOpen,
    UnescapeVariable,
    Comment,
    SetDelimiter,
  };

  Token(StringRef Str)
      : TokenType(Type::Text), RawBody(Str), TokenBody(RawBody),
        Accessor_മൂല്യം({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, അക്ഷരം Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    എങ്കിൽ (TokenType == Type::Comment)
      തിരികെ;
    StringRef AccessorStr(ഇത്->TokenBody);
    എങ്കിൽ (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_മൂല്യം = splitMustacheString(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() സ്ഥിരം { തിരികെ Accessor_മൂല്യം; }

  Type getType() സ്ഥിരം { തിരികെ TokenType; }

  ശൂന്യം setIndentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() സ്ഥിരം { തിരികെ Indentation; }

  സ്ഥിതം Type getTokenType(അക്ഷരം Identifier) {
    തിരഞ്ഞെടുപ്പ് (Identifier) {
    അവസ്ഥ '#':
      തിരികെ Type::SectionOpen;
    അവസ്ഥ '/':
      തിരികെ Type::SectionClose;
    അവസ്ഥ '^':
      തിരികെ Type::InvertSectionOpen;
    അവസ്ഥ '!':
      തിരികെ Type::Comment;
    അവസ്ഥ '>':
      തിരികെ Type::Partial;
    അവസ്ഥ '&':
      തിരികെ Type::UnescapeVariable;
    അവസ്ഥ '=':
      തിരികെ Type::SetDelimiter;
    സ്ഥിരസ്ഥിതി:
      തിരികെ Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_മൂല്യം;
  size_t Indentation;
};

using EscapeMap = DenseMap<അക്ഷരം, std::string>;

വർഗ്ഗം ASTNode : പൊതു ilist_കണ്ണി<ASTNode> {
പൊതു:
  എണ്ണൽ Type {
    വേര്_2,
    Text,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::വേര്_2), Parent(nullptr), ParentContext(nullptr) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *Parent)
      : Ctx(Ctx), Ty(Type::Text), Body(Body), Parent(Parent),
        ParentContext(nullptr) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *Parent)
      : Ctx(Ctx), Ty(Ty), Parent(Parent), Accessor_മൂല്യം(Accessor),
        ParentContext(nullptr) {}

  ശൂന്യം addChild(AstPtr Child) { Children.തള്ളുക_back(Child); };

  ശൂന്യം setRawBody(StringRef NewBody) { RawBody = NewBody; };

  ശൂന്യം setIndentation(size_t NewIndentation) { Indentation = NewIndentation; };

  ശൂന്യം render(സ്ഥിരം llvm::json::മൂല്യം &Data, MustacheOutputStream &OS);

സ്വകാര്യ:
  ശൂന്യം renderLambdas(സ്ഥിരം llvm::json::മൂല്യം &Contexts,
                     MustacheOutputStream &OS, Lambda &L);

  ശൂന്യം renderSectionLambdas(സ്ഥിരം llvm::json::മൂല്യം &Contexts,
                            MustacheOutputStream &OS, SectionLambda &L);

  ശൂന്യം renderPartial(സ്ഥിരം llvm::json::മൂല്യം &Contexts,
                     MustacheOutputStream &OS, ASTNode *Partial);

  ശൂന്യം renderChild(സ്ഥിരം llvm::json::മൂല്യം &Context, MustacheOutputStream &OS);

  സ്ഥിരം llvm::json::മൂല്യം *findContext();

  ശൂന്യം render_വേര്(സ്ഥിരം json::മൂല്യം &CurrentCtx, MustacheOutputStream &OS);
  ശൂന്യം renderText(MustacheOutputStream &OS);
  ശൂന്യം renderPartial(സ്ഥിരം json::മൂല്യം &CurrentCtx, MustacheOutputStream &OS);
  ശൂന്യം renderVariable(സ്ഥിരം json::മൂല്യം &CurrentCtx, MustacheOutputStream &OS);
  ശൂന്യം renderUnescapeVariable(സ്ഥിരം json::മൂല്യം &CurrentCtx,
                              MustacheOutputStream &OS);
  ശൂന്യം renderSection(സ്ഥിരം json::മൂല്യം &CurrentCtx, MustacheOutputStream &OS);
  ശൂന്യം renderInvertSection(സ്ഥിരം json::മൂല്യം &CurrentCtx,
                           MustacheOutputStream &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  സ്ഥിരം ArrayRef<StringRef> Accessor_മൂല്യം;
  സ്ഥിരം llvm::json::മൂല്യം *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
സ്ഥിതം AstPtr create_വേര്_കണ്ണി(MustacheContext &Ctx) {
  തിരികെ പുതിയ (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

സ്ഥിതം AstPtr create_കണ്ണി(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  തിരികെ പുതിയ (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

സ്ഥിതം AstPtr create_Text_കണ്ണി(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  തിരികെ പുതിയ (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
}

// Function to check if there is meaningful text behind.
// We determine if a token has meaningful text behind
// if the right of previous token contains anything that is
// not a newline.
// For example:
//  "Stuff {{#Section}}" (returns true)
//   vs
//  "{{#Section}} \n" (returns false)
// We make an exception for when previous token is empty
// and the current token is the second token.
// For example:
//  "{{#Section}}"
സ്ഥിതം തർക്കം hasTextBehind(size_t Idx, സ്ഥിരം ArrayRef<Token> &Tokens) {
  എങ്കിൽ (Idx == 0)
    തിരികെ സത്യം;

  size_t PrevIdx = Idx - 1;
  എങ്കിൽ (Tokens[PrevIdx].getType() != Token::Type::Text)
    തിരികെ സത്യം;

  സ്ഥിരം Token &PrevToken = Tokens[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  തിരികെ !TokenBody.ends_with("\n") && !(TokenBody.empty() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
സ്ഥിതം തർക്കം hasTextAhead(size_t Idx, സ്ഥിരം ArrayRef<Token> &Tokens) {
  എങ്കിൽ (Idx >= Tokens.size() - 1)
    തിരികെ സത്യം;

  size_t NextIdx = Idx + 1;
  എങ്കിൽ (Tokens[NextIdx].getType() != Token::Type::Text)
    തിരികെ സത്യം;

  സ്ഥിരം Token &NextToken = Tokens[NextIdx];
  StringRef TokenBody = StringRef(NextToken.RawBody).ltrim(" ");
  തിരികെ !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

സ്ഥിതം തർക്കം requiresCleanUp(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  തിരികെ T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::SetDelimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
സ്ഥിതം ശൂന്യം stripTokenAhead(SmallVectorImpl<Token> &Tokens, size_t Idx) {
  Token &NextToken = Tokens[Idx + 1];
  StringRef NextTokenBody = NextToken.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  എങ്കിൽ (NextTokenBody.starts_with("\r\n"))
    NextToken.TokenBody = NextTokenBody.substr(2);
  അല്ലെങ്കിൽ എങ്കിൽ (NextTokenBody.starts_with("\n"))
    NextToken.TokenBody = NextTokenBody.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
ശൂന്യം stripTokenBefore(SmallVectorImpl<Token> &Tokens, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = Tokens[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.size() - Unindented.size();
  PrevToken.TokenBody = Unindented;
  CurrentToken.setIndentation(Indentation);
}

ഘടന Tag {
  എണ്ണൽ വർഗ്ഗം Kind {
    None,
    Normal, // {{...}}
    Triple, // {{{...}}}
  };

  Kind TagKind = Kind::None;
  StringRef Content;   // The content between the delimiters.
  StringRef FullMatch; // The entire tag, including delimiters.
  size_t StartPosition = StringRef::npos;
};

[[maybe_unused]] സ്ഥിതം സ്ഥിരം അക്ഷരം *tagKindToString(Tag::Kind K) {
  തിരഞ്ഞെടുപ്പ് (K) {
  അവസ്ഥ Tag::Kind::None:
    തിരികെ "None";
  അവസ്ഥ Tag::Kind::Normal:
    തിരികെ "Normal";
  അവസ്ഥ Tag::Kind::Triple:
    തിരികെ "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_unused]] സ്ഥിതം സ്ഥിരം അക്ഷരം *jsonKindToString(json::മൂല്യം::Kind K) {
  തിരഞ്ഞെടുപ്പ് (K) {
  അവസ്ഥ json::മൂല്യം::Kind::Null:
    തിരികെ "JSON_KIND_NULL";
  അവസ്ഥ json::മൂല്യം::Kind::Boolean:
    തിരികെ "JSON_KIND_BOOLEAN";
  അവസ്ഥ json::മൂല്യം::Kind::സംഖ്യ:
    തിരികെ "JSON_KIND_NUMBER";
  അവസ്ഥ json::മൂല്യം::Kind::String:
    തിരികെ "JSON_KIND_STRING";
  അവസ്ഥ json::മൂല്യം::Kind::Array:
    തിരികെ "JSON_KIND_ARRAY";
  അവസ്ഥ json::മൂല്യം::Kind::Object:
    തിരികെ "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
സ്ഥിതം SmallVector<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  SmallVector<Token> Tokens;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t TextStart = 0;

  സ്ഥിരം StringLiteral TripleOpen("{{{");
  സ്ഥിരം StringLiteral TripleClose("}}}");

  ആകുമ്പോൾ (Cursor < Template.size()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef TagOpen, TagClose;
    Tag::Kind Kind;

    // Determine which tag we've encountered.
    എങ്കിൽ (TemplateSuffix.starts_with(TripleOpen)) {
      Kind = Tag::Kind::Triple;
      TagOpen = TripleOpen;
      TagClose = TripleClose;
    } അല്ലെങ്കിൽ എങ്കിൽ (TemplateSuffix.starts_with(Open)) {
      Kind = Tag::Kind::Normal;
      TagOpen = Open;
      TagClose = Close;
    } അല്ലെങ്കിൽ {
      // Not at a tag, continue scanning.
      ++Cursor;
      തുടരുക;
    }

    // Found a tag, first add the preceding text.
    എങ്കിൽ (Cursor > TextStart)
      Tokens.emplace_back(Template.slice(TextStart, Cursor));

    // Find the closing tag.
    size_t EndPos = Template.find(TagClose, Cursor + TagOpen.size());
    എങ്കിൽ (EndPos == StringRef::npos) {
      // No closing tag, the rest is text.
      Tokens.emplace_back(Template.substr(Cursor));
      TextStart = Cursor = Template.size();
      നിർത്തുക;
    }

    // Extract tag content and full match.
    size_t ContentStart = Cursor + TagOpen.size();
    StringRef Content = Template.substr(ContentStart, EndPos - ContentStart);
    StringRef FullMatch =
        Template.substr(Cursor, (EndPos + TagClose.size()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << FullMatch << ", Content: " << Content
                      << ", Kind: " << tagKindToString(Kind) << "\n");
    എങ്കിൽ (Kind == Tag::Kind::Triple) {
      Tokens.emplace_back(FullMatch, Ctx.Saver.save("&" + Content), '&', Ctx);
    } അല്ലെങ്കിൽ { // Normal Tag
      StringRef Interpolated = Content;
      എങ്കിൽ (!Interpolated.trim().starts_with("=")) {
        അക്ഷരം Front = Interpolated.empty() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_back(FullMatch, Interpolated, Front, Ctx);
      } അല്ലെങ്കിൽ { // Set Delimiter
        Tokens.emplace_back(FullMatch, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](അക്ഷരം C) { തിരികെ C == '='; });
        DelimSpec = DelimSpec.trim();

        auto [NewOpen, NewClose] = DelimSpec.split(' ');
        LLVM_DEBUG(dbgs() << "[Set Delimiter] NewOpen: " << NewOpen
                          << ", NewClose: " << NewClose << "\n");
        Open = NewOpen;
        Close = NewClose;
      }
    }

    // Move past the tag for the next iteration.
    Cursor += FullMatch.size();
    TextStart = Cursor;
  }

  // Add any remaining text after the last tag.
  എങ്കിൽ (TextStart < Template.size())
    Tokens.emplace_back(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  size_t LastIdx = Tokens.size() - 1;
  വേണ്ടി (size_t Idx = 0, End = Tokens.size(); Idx < End; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    എങ്കിൽ (!requiresCleanUp(CurrentType))
      തുടരുക;

    തർക്കം HasTextBehind = hasTextBehind(Idx, Tokens);
    തർക്കം HasTextAhead = hasTextAhead(Idx, Tokens);

    എങ്കിൽ ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    എങ്കിൽ ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == LastIdx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  തിരികെ Tokens;
}

// Custom stream to escape strings.
വർഗ്ഗം EscapeStringStream : പൊതു MustacheOutputStream {
പൊതു:
  explicit EscapeStringStream(llvm::raw_ostream &WrappedStream,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.താക്കോലുകൾ().begin(), Escape.താക്കോലുകൾ().end()),
        WrappedStream(WrappedStream) {
    SetUnbuffered();
  }

സംരക്ഷിത:
  ശൂന്യം write_impl(സ്ഥിരം അക്ഷരം *Ptr, size_t Size) override {
    StringRef Data(Ptr, Size);
    size_t Start = 0;
    ആകുമ്പോൾ (Start < Size) {
      // Find the next character that needs to be escaped.
      size_t Next = Data.find_first_of(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      എങ്കിൽ (Next == StringRef::npos) {
        WrappedStream << Data.substr(Start);
        തിരികെ;
      }

      // Write the chunk of text before the escapable character.
      എങ്കിൽ (Next > Start)
        WrappedStream << Data.substr(Start, Next - Start);

      // Look up and write the escaped version of the character.
      WrappedStream << Escape[Data[Next]];
      Start = Next + 1;
    }
  }

  uint64_t current_pos() സ്ഥിരം override { തിരികെ WrappedStream.tell(); }

സ്വകാര്യ:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &WrappedStream;
};

// Custom stream to add indentation used to for rendering partials.
വർഗ്ഗം AddIndentationStringStream : പൊതു MustacheOutputStream {
പൊതു:
  explicit AddIndentationStringStream(raw_ostream &WrappedStream,
                                      size_t Indentation)
      : Indentation(Indentation), WrappedStream(WrappedStream),
        NeedsIndent(സത്യം), IsSuspended(അസത്യം) {
    SetUnbuffered();
  }

  ശൂന്യം suspendIndentation() override { IsSuspended = സത്യം; }
  ശൂന്യം resumeIndentation() override { IsSuspended = അസത്യം; }

സംരക്ഷിത:
  ശൂന്യം write_impl(സ്ഥിരം അക്ഷരം *Ptr, size_t Size) override {
    llvm::StringRef Data(Ptr, Size);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    വേണ്ടി (അക്ഷരം C : Data) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      എങ്കിൽ (NeedsIndent && C != '\n') {
        WrappedStream << Indent;
        NeedsIndent = അസത്യം;
      }
      WrappedStream << C;
      എങ്കിൽ (C == '\n' && !IsSuspended)
        NeedsIndent = സത്യം;
    }
  }

  uint64_t current_pos() സ്ഥിരം override { തിരികെ WrappedStream.tell(); }

സ്വകാര്യ:
  size_t Indentation;
  raw_ostream &WrappedStream;
  തർക്കം NeedsIndent;
  തർക്കം IsSuspended;
};

വർഗ്ഗം Parser {
പൊതു:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

സ്വകാര്യ:
  ശൂന്യം parseMustache(ASTNode *Parent);
  ശൂന്യം parseSection(ASTNode *Parent, ASTNode::Type Ty, സ്ഥിരം Accessor &A);

  MustacheContext &Ctx;
  SmallVector<Token> Tokens;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

ശൂന്യം Parser::parseSection(ASTNode *Parent, ASTNode::Type Ty,
                          സ്ഥിരം Accessor &A) {
  AstPtr Current_കണ്ണി = create_കണ്ണി(Ctx, Ty, A, Parent);
  size_t Start = CurrentPtr;
  parseMustache(Current_കണ്ണി);
  സ്ഥിരം size_t End = CurrentPtr - 1;

  size_t RawBodySize = 0;
  വേണ്ടി (size_t I = Start; I < End; ++I)
    RawBodySize += Tokens[I].RawBody.size();

  SmallString<128> RawBody;
  RawBody.reserve(RawBodySize);
  വേണ്ടി (std::size_t I = Start; I < End; ++I)
    RawBody += Tokens[I].RawBody;

  Current_കണ്ണി->setRawBody(Ctx.Saver.save(StringRef(RawBody)));
  Parent->addChild(Current_കണ്ണി);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr വേര്_കണ്ണി = create_വേര്_കണ്ണി(Ctx);
  parseMustache(വേര്_കണ്ണി);
  തിരികെ വേര്_കണ്ണി;
}

ശൂന്യം Parser::parseMustache(ASTNode *Parent) {

  ആകുമ്പോൾ (CurrentPtr < Tokens.size()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_കണ്ണി;

    തിരഞ്ഞെടുപ്പ് (CurrentToken.getType()) {
    അവസ്ഥ Token::Type::Text: {
      Current_കണ്ണി = create_Text_കണ്ണി(Ctx, CurrentToken.TokenBody, Parent);
      Parent->addChild(Current_കണ്ണി);
      നിർത്തുക;
    }
    അവസ്ഥ Token::Type::Variable: {
      Current_കണ്ണി = create_കണ്ണി(Ctx, ASTNode::Variable, A, Parent);
      Parent->addChild(Current_കണ്ണി);
      നിർത്തുക;
    }
    അവസ്ഥ Token::Type::UnescapeVariable: {
      Current_കണ്ണി = create_കണ്ണി(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->addChild(Current_കണ്ണി);
      നിർത്തുക;
    }
    അവസ്ഥ Token::Type::Partial: {
      Current_കണ്ണി = create_കണ്ണി(Ctx, ASTNode::Partial, A, Parent);
      Current_കണ്ണി->setIndentation(CurrentToken.getIndentation());
      Parent->addChild(Current_കണ്ണി);
      നിർത്തുക;
    }
    അവസ്ഥ Token::Type::SectionOpen: {
      parseSection(Parent, ASTNode::Section, A);
      നിർത്തുക;
    }
    അവസ്ഥ Token::Type::InvertSectionOpen: {
      parseSection(Parent, ASTNode::InvertSection, A);
      നിർത്തുക;
    }
    അവസ്ഥ Token::Type::Comment:
    അവസ്ഥ Token::Type::SetDelimiter:
      നിർത്തുക;
    അവസ്ഥ Token::Type::SectionClose:
      തിരികെ;
    }
  }
}
സ്ഥിതം ശൂന്യം toMustacheString(സ്ഥിരം json::മൂല്യം &Data, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << jsonKindToString(Data.kind()) << ", Data: " << Data
                    << "\n");
  തിരഞ്ഞെടുപ്പ് (Data.kind()) {
  അവസ്ഥ json::മൂല്യം::Null:
    തിരികെ;
  അവസ്ഥ json::മൂല്യം::സംഖ്യ: {
    auto അക്കം = *Data.get_As_സംഖ്യ();
    std::ostringstream SS;
    SS << അക്കം;
    OS << SS.str();
    തിരികെ;
  }
  അവസ്ഥ json::മൂല്യം::String: {
    OS << *Data.getAsString();
    തിരികെ;
  }

  അവസ്ഥ json::മൂല്യം::Array: {
    auto Arr = *Data.getAsArray();
    എങ്കിൽ (Arr.empty())
      തിരികെ;
    [[fallthrough]];
  }
  അവസ്ഥ json::മൂല്യം::Object:
  അവസ്ഥ json::മൂല്യം::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.മൂല്യം_2(Data);
    നിർത്തുക;
  }
  }
}

ശൂന്യം ASTNode::render_വേര്(സ്ഥിരം json::മൂല്യം &CurrentCtx,
                         MustacheOutputStream &OS) {
  renderChild(CurrentCtx, OS);
}

ശൂന്യം ASTNode::renderText(MustacheOutputStream &OS) { OS << Body; }

ശൂന്യം ASTNode::renderPartial(സ്ഥിരം json::മൂല്യം &CurrentCtx,
                            MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_മൂല്യം[0]
                    << ", Indentation:" << Indentation << "\n");
  auto Partial = Ctx.Partials.find(Accessor_മൂല്യം[0]);
  എങ്കിൽ (Partial != Ctx.Partials.end())
    renderPartial(CurrentCtx, OS, Partial->get_മൂല്യം());
}

ശൂന്യം ASTNode::renderVariable(സ്ഥിരം json::മൂല്യം &CurrentCtx,
                             MustacheOutputStream &OS) {
  auto Lambda = Ctx.Lambdas.find(Accessor_മൂല്യം[0]);
  എങ്കിൽ (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_മൂല്യം());
  } അല്ലെങ്കിൽ എങ്കിൽ (സ്ഥിരം json::മൂല്യം *ContextPtr = findContext()) {
    EscapeStringStream ES(OS, Ctx.Escapes);
    toMustacheString(*ContextPtr, ES);
  }
}

ശൂന്യം ASTNode::renderUnescapeVariable(സ്ഥിരം json::മൂല്യം &CurrentCtx,
                                     MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_മൂല്യം[0]
                    << "\n");
  auto Lambda = Ctx.Lambdas.find(Accessor_മൂല്യം[0]);
  എങ്കിൽ (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_മൂല്യം());
  } അല്ലെങ്കിൽ എങ്കിൽ (സ്ഥിരം json::മൂല്യം *ContextPtr = findContext()) {
    OS.suspendIndentation();
    toMustacheString(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

ശൂന്യം ASTNode::renderSection(സ്ഥിരം json::മൂല്യം &CurrentCtx,
                            MustacheOutputStream &OS) {
  auto SectionLambda = Ctx.SectionLambdas.find(Accessor_മൂല്യം[0]);
  എങ്കിൽ (SectionLambda != Ctx.SectionLambdas.end()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_മൂല്യം());
    തിരികെ;
  }

  സ്ഥിരം json::മൂല്യം *ContextPtr = findContext();
  എങ്കിൽ (isContextFalsey(ContextPtr))
    തിരികെ;

  എങ്കിൽ (സ്ഥിരം json::Array *Arr = ContextPtr->getAsArray()) {
    വേണ്ടി (സ്ഥിരം json::മൂല്യം &V : *Arr)
      renderChild(V, OS);
    തിരികെ;
  }
  renderChild(*ContextPtr, OS);
}

ശൂന്യം ASTNode::renderInvertSection(സ്ഥിരം json::മൂല്യം &CurrentCtx,
                                  MustacheOutputStream &OS) {
  തർക്കം IsLambda = Ctx.SectionLambdas.contains(Accessor_മൂല്യം[0]);
  സ്ഥിരം json::മൂല്യം *ContextPtr = findContext();
  എങ്കിൽ (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

ശൂന്യം ASTNode::render(സ്ഥിരം llvm::json::മൂല്യം &Data, MustacheOutputStream &OS) {
  എങ്കിൽ (Ty != വേര്_2 && Ty != Text && Accessor_മൂല്യം.empty())
    തിരികെ;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &Data;

  തിരഞ്ഞെടുപ്പ് (Ty) {
  അവസ്ഥ വേര്_2:
    render_വേര്(Data, OS);
    തിരികെ;
  അവസ്ഥ Text:
    renderText(OS);
    തിരികെ;
  അവസ്ഥ Partial:
    renderPartial(Data, OS);
    തിരികെ;
  അവസ്ഥ Variable:
    renderVariable(Data, OS);
    തിരികെ;
  അവസ്ഥ UnescapeVariable:
    renderUnescapeVariable(Data, OS);
    തിരികെ;
  അവസ്ഥ Section:
    renderSection(Data, OS);
    തിരികെ;
  അവസ്ഥ InvertSection:
    renderInvertSection(Data, OS);
    തിരികെ;
  }
  llvm_unreachable("Invalid ASTNode type");
}

സ്ഥിരം json::മൂല്യം *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  എങ്കിൽ (Accessor_മൂല്യം.empty())
    തിരികെ nullptr;
  എങ്കിൽ (Accessor_മൂല്യം[0] == ".")
    തിരികെ ParentContext;

  സ്ഥിരം json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = Accessor_മൂല്യം[0];
  ASTNode *CurrentParent = Parent;

  ആകുമ്പോൾ (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    എങ്കിൽ (CurrentParent->Ty != വേര്_2) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      തുടരുക;
    }
    തിരികെ nullptr;
  }
  സ്ഥിരം json::മൂല്യം *Context = nullptr;
  വേണ്ടി (auto [Idx, Acc] : enumerate(Accessor_മൂല്യം)) {
    സ്ഥിരം json::മൂല്യം *Current_മൂല്യം = CurrentContext->get(Acc);
    എങ്കിൽ (!Current_മൂല്യം)
      തിരികെ nullptr;
    എങ്കിൽ (Idx < Accessor_മൂല്യം.size() - 1) {
      CurrentContext = Current_മൂല്യം->getAsObject();
      എങ്കിൽ (!CurrentContext)
        തിരികെ nullptr;
    } അല്ലെങ്കിൽ {
      Context = Current_മൂല്യം;
    }
  }
  തിരികെ Context;
}

ശൂന്യം ASTNode::renderChild(സ്ഥിരം json::മൂല്യം &Contexts,
                          MustacheOutputStream &OS) {
  വേണ്ടി (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

ശൂന്യം ASTNode::renderPartial(സ്ഥിരം json::മൂല്യം &Contexts,
                            MustacheOutputStream &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  AddIndentationStringStream IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

ശൂന്യം ASTNode::renderLambdas(സ്ഥിരം llvm::json::മൂല്യം &Contexts,
                            MustacheOutputStream &OS, Lambda &L) {
  json::മൂല്യം LambdaResult = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(LambdaResult, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_കണ്ണി = P.parse();

  EscapeStringStream ES(OS, Ctx.Escapes);
  എങ്കിൽ (Ty == Variable) {
    Lambda_കണ്ണി->render(Contexts, ES);
    തിരികെ;
  }
  Lambda_കണ്ണി->render(Contexts, OS);
}

ശൂന്യം ASTNode::renderSectionLambdas(സ്ഥിരം llvm::json::മൂല്യം &Contexts,
                                   MustacheOutputStream &OS, SectionLambda &L) {
  json::മൂല്യം Return = L(RawBody.str());
  എങ്കിൽ (isFalsey(Return))
    തിരികെ;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_കണ്ണി = P.parse();
  Lambda_കണ്ണി->render(Contexts, OS);
}

ശൂന്യം Template::render(സ്ഥിരം llvm::json::മൂല്യം &Data, llvm::raw_ostream &OS) {
  RawMustacheOutputStream MOS(OS);
  വൃക്ഷം->render(Data, MOS);
}

ശൂന്യം Template::registerPartial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_വൃക്ഷം = P.parse();
  Ctx.Partials.തിരുകുക(std::make_pair(Name, Partial_വൃക്ഷം));
}

ശൂന്യം Template::registerLambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

ശൂന്യം Template::registerLambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

ശൂന്യം Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  വൃക്ഷം = P.parse();
  // The default behavior is to escape html entities.
  സ്ഥിരം EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), വൃക്ഷം(Other.വൃക്ഷം) {
  Other.വൃക്ഷം = nullptr;
}

Template::~Template() = സ്ഥിരസ്ഥിതി;

} // namespace llvm::mustache

#undef DEBUG_TYPE

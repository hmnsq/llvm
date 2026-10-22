//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ضمن "llvm/Support/Mustache.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <sstream>

#وصف_ڪريو DEBUG_TYPE "mustache"

استعمال نطاق llvm;
استعمال نطاق llvm::mustache;

نطاق {

استعمال Accessor = ArrayRef<StringRef>;

ساكن منطقي isFalsey(ثابت json::قدر &V) {
  أرجع V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().قدر_2()) ||
         (V.getAsArray() && V.getAsArray()->خالي_جايون());
}

ساكن منطقي isContextFalsey(ثابت json::قدر *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  إذا (!V)
    أرجع سچ;
  أرجع isFalsey(*V);
}

ساكن فراغ ورهايو_And_Trim(StringRef Str, Small_ويڪٽر_Impl<StringRef> &ٽوڪن) {
  size_t CurrentPos = 0;
  طالما (CurrentPos < Str.سائيز_2()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.ڳوليو('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    إذا (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.سائيز_2();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.ٽڪرو(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t شروعات = Part.ڳوليو_پهريون_not_جو(" \t\r\n");
    إذا (شروعات != StringRef::npos) {
      size_t پڇاڙي_2 = Part.ڳوليو_آخري_not_جو(" \t\r\n");
      ٽوڪن.ڌڪو_پٺ(Part.ٽڪرو(شروعات, پڇاڙي_2 + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

ساكن Accessor ورهايو_Mustache_String(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  Small_ويڪٽر<StringRef> ٽوڪن;
  إذا (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    ٽوڪن.ڌڪو_پٺ(".");
  } وإلا {
    ورهايو_And_Trim(Str, ٽوڪن);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *Arena_ٽوڪن = Ctx.Allocator.Allocate<StringRef>(ٽوڪن.سائيز_2());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(ٽوڪن, Arena_ٽوڪن);
  // Return an ArrayRef pointing to the stable arena memory.
  أرجع ArrayRef<StringRef>(Arena_ٽوڪن, ٽوڪن.سائيز_2());
}
} // namespace

نطاق llvm::mustache {

صنف Mustache_Output_وهڪرو : عام raw_ostream {
عام:
  Mustache_Output_وهڪرو() = افتراضي;
  ~Mustache_Output_وهڪرو() override = افتراضي;

  مجازي فراغ suspendIndentation() {}
  مجازي فراغ resumeIndentation() {}

خاص:
  فراغ anchor() override;
};

فراغ Mustache_Output_وهڪرو::anchor() {}

صنف Raw_Mustache_Output_وهڪرو : عام Mustache_Output_وهڪرو {
عام:
  Raw_Mustache_Output_وهڪرو(raw_ostream &OS) : OS(OS) { سيٽ_Unbuffered(); }

خاص:
  raw_ostream &OS;

  فراغ write_impl(ثابت حرف *Ptr, size_t سائيز) override {
    OS.اكتب(Ptr, سائيز);
  }
  uint64_t current_pos() ثابت override { أرجع OS.tell(); }
};

صنف Token {
عام:
  تعداد صنف Type {
    متن_2,
    Variable,
    Partial,
    SectionOpen,
    SectionClose,
    InvertSectionOpen,
    UnescapeVariable,
    Comment,
    سيٽ_Delimiter,
  };

  Token(StringRef Str)
      : TokenType(Type::متن_2), RawBody(Str), TokenBody(RawBody),
        Accessor_قدر({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, حرف Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    إذا (TokenType == Type::Comment)
      أرجع;
    StringRef AccessorStr(هذا->TokenBody);
    إذا (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_قدر = ورهايو_Mustache_String(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ثابت { أرجع Accessor_قدر; }

  Type getType() ثابت { أرجع TokenType; }

  فراغ سيٽ_Indentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() ثابت { أرجع Indentation; }

  ساكن Type getTokenType(حرف Identifier) {
    بدل (Identifier) {
    حالة '#':
      أرجع Type::SectionOpen;
    حالة '/':
      أرجع Type::SectionClose;
    حالة '^':
      أرجع Type::InvertSectionOpen;
    حالة '!':
      أرجع Type::Comment;
    حالة '>':
      أرجع Type::Partial;
    حالة '&':
      أرجع Type::UnescapeVariable;
    حالة '=':
      أرجع Type::سيٽ_Delimiter;
    افتراضي:
      أرجع Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_قدر;
  size_t Indentation;
};

استعمال EscapeMap = DenseMap<حرف, std::string>;

صنف ASTNode : عام ilist_ڳنڍ<ASTNode> {
عام:
  تعداد Type {
    پاڙ_2,
    متن_2,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::پاڙ_2), والد_2(خالي_اشارو), والد_Context(خالي_اشارو) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *والد_2)
      : Ctx(Ctx), Ty(Type::متن_2), Body(Body), والد_2(والد_2),
        والد_Context(خالي_اشارو) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *والد_2)
      : Ctx(Ctx), Ty(Ty), والد_2(والد_2), Accessor_قدر(Accessor),
        والد_Context(خالي_اشارو) {}

  فراغ جوڙيو_ٻار(AstPtr ٻار) { Children.ڌڪو_پٺ(ٻار); };

  فراغ سيٽ_Raw_Body(StringRef NewBody) { RawBody = NewBody; };

  فراغ سيٽ_Indentation(size_t NewIndentation) { Indentation = NewIndentation; };

  فراغ render(ثابت llvm::json::قدر &معلومات_2, Mustache_Output_وهڪرو &OS);

خاص:
  فراغ renderLambdas(ثابت llvm::json::قدر &Contexts,
                     Mustache_Output_وهڪرو &OS, Lambda &L);

  فراغ renderSectionLambdas(ثابت llvm::json::قدر &Contexts,
                            Mustache_Output_وهڪرو &OS, SectionLambda &L);

  فراغ renderPartial(ثابت llvm::json::قدر &Contexts,
                     Mustache_Output_وهڪرو &OS, ASTNode *Partial);

  فراغ render_ٻار(ثابت llvm::json::قدر &Context, Mustache_Output_وهڪرو &OS);

  ثابت llvm::json::قدر *ڳوليو_Context();

  فراغ render_پاڙ(ثابت json::قدر &CurrentCtx, Mustache_Output_وهڪرو &OS);
  فراغ render_متن(Mustache_Output_وهڪرو &OS);
  فراغ renderPartial(ثابت json::قدر &CurrentCtx, Mustache_Output_وهڪرو &OS);
  فراغ renderVariable(ثابت json::قدر &CurrentCtx, Mustache_Output_وهڪرو &OS);
  فراغ renderUnescapeVariable(ثابت json::قدر &CurrentCtx,
                              Mustache_Output_وهڪرو &OS);
  فراغ renderSection(ثابت json::قدر &CurrentCtx, Mustache_Output_وهڪرو &OS);
  فراغ renderInvertSection(ثابت json::قدر &CurrentCtx,
                           Mustache_Output_وهڪرو &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *والد_2;
  ASTNodeList Children;
  ثابت ArrayRef<StringRef> Accessor_قدر;
  ثابت llvm::json::قدر *والد_Context;
};

// A wrapper for arena allocator for ASTNodes
ساكن AstPtr create_پاڙ_ڳنڍ(MustacheContext &Ctx) {
  أرجع جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

ساكن AstPtr create_ڳنڍ(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *والد_2) {
  أرجع جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, والد_2);
}

ساكن AstPtr create_متن_ڳنڍ(MustacheContext &Ctx, StringRef Body,
                             ASTNode *والد_2) {
  أرجع جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, والد_2);
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
ساكن منطقي has_متن_Behind(size_t Idx, ثابت ArrayRef<Token> &ٽوڪن) {
  إذا (Idx == 0)
    أرجع سچ;

  size_t PrevIdx = Idx - 1;
  إذا (ٽوڪن[PrevIdx].getType() != Token::Type::متن_2)
    أرجع سچ;

  ثابت Token &PrevToken = ٽوڪن[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  أرجع !TokenBody.پڇاڙيون_with("\n") && !(TokenBody.خالي_جايون() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
ساكن منطقي has_متن_Ahead(size_t Idx, ثابت ArrayRef<Token> &ٽوڪن) {
  إذا (Idx >= ٽوڪن.سائيز_2() - 1)
    أرجع سچ;

  size_t ايندڙ_Idx = Idx + 1;
  إذا (ٽوڪن[ايندڙ_Idx].getType() != Token::Type::متن_2)
    أرجع سچ;

  ثابت Token &ايندڙ_Token = ٽوڪن[ايندڙ_Idx];
  StringRef TokenBody = StringRef(ايندڙ_Token.RawBody).ltrim(" ");
  أرجع !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

ساكن منطقي requires_Clean_مٿي(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  أرجع T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::سيٽ_Delimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
ساكن فراغ stripTokenAhead(Small_ويڪٽر_Impl<Token> &ٽوڪن, size_t Idx) {
  Token &ايندڙ_Token = ٽوڪن[Idx + 1];
  StringRef ايندڙ_Token_Body = ايندڙ_Token.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  إذا (ايندڙ_Token_Body.starts_with("\r\n"))
    ايندڙ_Token.TokenBody = ايندڙ_Token_Body.substr(2);
  وإلا إذا (ايندڙ_Token_Body.starts_with("\n"))
    ايندڙ_Token.TokenBody = ايندڙ_Token_Body.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
فراغ stripTokenBefore(Small_ويڪٽر_Impl<Token> &ٽوڪن, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = ٽوڪن[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.سائيز_2() - Unindented.سائيز_2();
  PrevToken.TokenBody = Unindented;
  CurrentToken.سيٽ_Indentation(Indentation);
}

بنية ٽيگ {
  تعداد صنف قسم_2 {
    None,
    عام_2, // {{...}}
    Triple, // {{{...}}}
  };

  قسم_2 ٽيگ_قسم = قسم_2::None;
  StringRef Content;   // The content between the delimiters.
  StringRef ڀريل_ميلاپ; // The entire tag, including delimiters.
  size_t شروعات_Position = StringRef::npos;
};

[[maybe_اڻ_استعمال_ٿيل]] ساكن ثابت حرف *ٽيگ_قسم_منزل_String(ٽيگ::قسم_2 K) {
  بدل (K) {
  حالة ٽيگ::قسم_2::None:
    أرجع "None";
  حالة ٽيگ::قسم_2::عام_2:
    أرجع "Normal";
  حالة ٽيگ::قسم_2::Triple:
    أرجع "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_اڻ_استعمال_ٿيل]] ساكن ثابت حرف *json_قسم_منزل_String(json::قدر::قسم_2 K) {
  بدل (K) {
  حالة json::قدر::قسم_2::Null:
    أرجع "JSON_KIND_NULL";
  حالة json::قدر::قسم_2::Boolean:
    أرجع "JSON_KIND_BOOLEAN";
  حالة json::قدر::قسم_2::عدد:
    أرجع "JSON_KIND_NUMBER";
  حالة json::قدر::قسم_2::String:
    أرجع "JSON_KIND_STRING";
  حالة json::قدر::قسم_2::Array:
    أرجع "JSON_KIND_ARRAY";
  حالة json::قدر::قسم_2::Object:
    أرجع "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
ساكن Small_ويڪٽر<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  Small_ويڪٽر<Token> ٽوڪن;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t متن_شروعات = 0;

  ثابت StringLiteral TripleOpen("{{{");
  ثابت StringLiteral TripleClose("}}}");

  طالما (Cursor < Template.سائيز_2()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef ٽيگ_Open, ٽيگ_Close;
    ٽيگ::قسم_2 قسم_2;

    // Determine which tag we've encountered.
    إذا (TemplateSuffix.starts_with(TripleOpen)) {
      قسم_2 = ٽيگ::قسم_2::Triple;
      ٽيگ_Open = TripleOpen;
      ٽيگ_Close = TripleClose;
    } وإلا إذا (TemplateSuffix.starts_with(Open)) {
      قسم_2 = ٽيگ::قسم_2::عام_2;
      ٽيگ_Open = Open;
      ٽيگ_Close = Close;
    } وإلا {
      // Not at a tag, continue scanning.
      ++Cursor;
      استمر;
    }

    // Found a tag, first add the preceding text.
    إذا (Cursor > متن_شروعات)
      ٽوڪن.emplace_پٺ(Template.ٽڪرو(متن_شروعات, Cursor));

    // Find the closing tag.
    size_t پڇاڙي_Pos = Template.ڳوليو(ٽيگ_Close, Cursor + ٽيگ_Open.سائيز_2());
    إذا (پڇاڙي_Pos == StringRef::npos) {
      // No closing tag, the rest is text.
      ٽوڪن.emplace_پٺ(Template.substr(Cursor));
      متن_شروعات = Cursor = Template.سائيز_2();
      اكسر;
    }

    // Extract tag content and full match.
    size_t Content_شروعات = Cursor + ٽيگ_Open.سائيز_2();
    StringRef Content = Template.substr(Content_شروعات, پڇاڙي_Pos - Content_شروعات);
    StringRef ڀريل_ميلاپ =
        Template.substr(Cursor, (پڇاڙي_Pos + ٽيگ_Close.سائيز_2()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << ڀريل_ميلاپ << ", Content: " << Content
                      << ", Kind: " << ٽيگ_قسم_منزل_String(قسم_2) << "\n");
    إذا (قسم_2 == ٽيگ::قسم_2::Triple) {
      ٽوڪن.emplace_پٺ(ڀريل_ميلاپ, Ctx.Saver.save("&" + Content), '&', Ctx);
    } وإلا { // Normal Tag
      StringRef Interpolated = Content;
      إذا (!Interpolated.trim().starts_with("=")) {
        حرف Front = Interpolated.خالي_جايون() ? ' ' : Interpolated.trim().front();
        ٽوڪن.emplace_پٺ(ڀريل_ميلاپ, Interpolated, Front, Ctx);
      } وإلا { // Set Delimiter
        ٽوڪن.emplace_پٺ(ڀريل_ميلاپ, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](حرف C) { أرجع C == '='; });
        DelimSpec = DelimSpec.trim();

        خودڪار [NewOpen, NewClose] = DelimSpec.ورهايو(' ');
        LLVM_DEBUG(dbgs() << "[Set Delimiter] NewOpen: " << NewOpen
                          << ", NewClose: " << NewClose << "\n");
        Open = NewOpen;
        Close = NewClose;
      }
    }

    // Move past the tag for the next iteration.
    Cursor += ڀريل_ميلاپ.سائيز_2();
    متن_شروعات = Cursor;
  }

  // Add any remaining text after the last tag.
  إذا (متن_شروعات < Template.سائيز_2())
    ٽوڪن.emplace_پٺ(Template.substr(متن_شروعات));

  // Fix up white spaces for standalone tags.
  size_t آخري_Idx = ٽوڪن.سائيز_2() - 1;
  لكل (size_t Idx = 0, پڇاڙي_2 = ٽوڪن.سائيز_2(); Idx < پڇاڙي_2; ++Idx) {
    Token &CurrentToken = ٽوڪن[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    إذا (!requires_Clean_مٿي(CurrentType))
      استمر;

    منطقي Has_متن_Behind = has_متن_Behind(Idx, ٽوڪن);
    منطقي Has_متن_Ahead = has_متن_Ahead(Idx, ٽوڪن);

    إذا ((!Has_متن_Ahead && !Has_متن_Behind) || (!Has_متن_Ahead && Idx == 0))
      stripTokenAhead(ٽوڪن, Idx);

    إذا ((!Has_متن_Behind && !Has_متن_Ahead) || (!Has_متن_Behind && Idx == آخري_Idx))
      stripTokenBefore(ٽوڪن, Idx, CurrentToken, CurrentType);
  }
  أرجع ٽوڪن;
}

// Custom stream to escape strings.
صنف Escape_String_وهڪرو : عام Mustache_Output_وهڪرو {
عام:
  واضح Escape_String_وهڪرو(llvm::raw_ostream &Wrapped_وهڪرو,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.ڪنجيون().begin(), Escape.ڪنجيون().پڇاڙي()),
        Wrapped_وهڪرو(Wrapped_وهڪرو) {
    سيٽ_Unbuffered();
  }

محمي:
  فراغ write_impl(ثابت حرف *Ptr, size_t سائيز) override {
    StringRef معلومات_2(Ptr, سائيز);
    size_t شروعات = 0;
    طالما (شروعات < سائيز) {
      // Find the next character that needs to be escaped.
      size_t ايندڙ_2 = معلومات_2.ڳوليو_پهريون_جو(EscapeChars.str(), شروعات);

      // If no escapable characters are found, write the rest of the string.
      إذا (ايندڙ_2 == StringRef::npos) {
        Wrapped_وهڪرو << معلومات_2.substr(شروعات);
        أرجع;
      }

      // Write the chunk of text before the escapable character.
      إذا (ايندڙ_2 > شروعات)
        Wrapped_وهڪرو << معلومات_2.substr(شروعات, ايندڙ_2 - شروعات);

      // Look up and write the escaped version of the character.
      Wrapped_وهڪرو << Escape[معلومات_2[ايندڙ_2]];
      شروعات = ايندڙ_2 + 1;
    }
  }

  uint64_t current_pos() ثابت override { أرجع Wrapped_وهڪرو.tell(); }

خاص:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &Wrapped_وهڪرو;
};

// Custom stream to add indentation used to for rendering partials.
صنف جوڙيو_Indentation_String_وهڪرو : عام Mustache_Output_وهڪرو {
عام:
  واضح جوڙيو_Indentation_String_وهڪرو(raw_ostream &Wrapped_وهڪرو,
                                      size_t Indentation)
      : Indentation(Indentation), Wrapped_وهڪرو(Wrapped_وهڪرو),
        NeedsIndent(سچ), IsSuspended(خطأ) {
    سيٽ_Unbuffered();
  }

  فراغ suspendIndentation() override { IsSuspended = سچ; }
  فراغ resumeIndentation() override { IsSuspended = خطأ; }

محمي:
  فراغ write_impl(ثابت حرف *Ptr, size_t سائيز) override {
    llvm::StringRef معلومات_2(Ptr, سائيز);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    لكل (حرف C : معلومات_2) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      إذا (NeedsIndent && C != '\n') {
        Wrapped_وهڪرو << Indent;
        NeedsIndent = خطأ;
      }
      Wrapped_وهڪرو << C;
      إذا (C == '\n' && !IsSuspended)
        NeedsIndent = سچ;
    }
  }

  uint64_t current_pos() ثابت override { أرجع Wrapped_وهڪرو.tell(); }

خاص:
  size_t Indentation;
  raw_ostream &Wrapped_وهڪرو;
  منطقي NeedsIndent;
  منطقي IsSuspended;
};

صنف Parser {
عام:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

خاص:
  فراغ parseMustache(ASTNode *والد_2);
  فراغ parseSection(ASTNode *والد_2, ASTNode::Type Ty, ثابت Accessor &A);

  MustacheContext &Ctx;
  Small_ويڪٽر<Token> ٽوڪن;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

فراغ Parser::parseSection(ASTNode *والد_2, ASTNode::Type Ty,
                          ثابت Accessor &A) {
  AstPtr Current_ڳنڍ = create_ڳنڍ(Ctx, Ty, A, والد_2);
  size_t شروعات = CurrentPtr;
  parseMustache(Current_ڳنڍ);
  ثابت size_t پڇاڙي_2 = CurrentPtr - 1;

  size_t Raw_Body_سائيز = 0;
  لكل (size_t I = شروعات; I < پڇاڙي_2; ++I)
    Raw_Body_سائيز += ٽوڪن[I].RawBody.سائيز_2();

  SmallString<128> RawBody;
  RawBody.reserve(Raw_Body_سائيز);
  لكل (std::size_t I = شروعات; I < پڇاڙي_2; ++I)
    RawBody += ٽوڪن[I].RawBody;

  Current_ڳنڍ->سيٽ_Raw_Body(Ctx.Saver.save(StringRef(RawBody)));
  والد_2->جوڙيو_ٻار(Current_ڳنڍ);
}

AstPtr Parser::parse() {
  ٽوڪن = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr پاڙ_ڳنڍ = create_پاڙ_ڳنڍ(Ctx);
  parseMustache(پاڙ_ڳنڍ);
  أرجع پاڙ_ڳنڍ;
}

فراغ Parser::parseMustache(ASTNode *والد_2) {

  طالما (CurrentPtr < ٽوڪن.سائيز_2()) {
    Token CurrentToken = ٽوڪن[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_ڳنڍ;

    بدل (CurrentToken.getType()) {
    حالة Token::Type::متن_2: {
      Current_ڳنڍ = create_متن_ڳنڍ(Ctx, CurrentToken.TokenBody, والد_2);
      والد_2->جوڙيو_ٻار(Current_ڳنڍ);
      اكسر;
    }
    حالة Token::Type::Variable: {
      Current_ڳنڍ = create_ڳنڍ(Ctx, ASTNode::Variable, A, والد_2);
      والد_2->جوڙيو_ٻار(Current_ڳنڍ);
      اكسر;
    }
    حالة Token::Type::UnescapeVariable: {
      Current_ڳنڍ = create_ڳنڍ(Ctx, ASTNode::UnescapeVariable, A, والد_2);
      والد_2->جوڙيو_ٻار(Current_ڳنڍ);
      اكسر;
    }
    حالة Token::Type::Partial: {
      Current_ڳنڍ = create_ڳنڍ(Ctx, ASTNode::Partial, A, والد_2);
      Current_ڳنڍ->سيٽ_Indentation(CurrentToken.getIndentation());
      والد_2->جوڙيو_ٻار(Current_ڳنڍ);
      اكسر;
    }
    حالة Token::Type::SectionOpen: {
      parseSection(والد_2, ASTNode::Section, A);
      اكسر;
    }
    حالة Token::Type::InvertSectionOpen: {
      parseSection(والد_2, ASTNode::InvertSection, A);
      اكسر;
    }
    حالة Token::Type::Comment:
    حالة Token::Type::سيٽ_Delimiter:
      اكسر;
    حالة Token::Type::SectionClose:
      أرجع;
    }
  }
}
ساكن فراغ منزل_Mustache_String(ثابت json::قدر &معلومات_2, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << json_قسم_منزل_String(معلومات_2.قسم()) << ", Data: " << معلومات_2
                    << "\n");
  بدل (معلومات_2.قسم()) {
  حالة json::قدر::Null:
    أرجع;
  حالة json::قدر::عدد: {
    خودڪار انگ = *معلومات_2.get_As_عدد();
    std::ostringstream SS;
    SS << انگ;
    OS << SS.str();
    أرجع;
  }
  حالة json::قدر::String: {
    OS << *معلومات_2.getAsString();
    أرجع;
  }

  حالة json::قدر::Array: {
    خودڪار Arr = *معلومات_2.getAsArray();
    إذا (Arr.خالي_جايون())
      أرجع;
    [[fallthrough]];
  }
  حالة json::قدر::Object:
  حالة json::قدر::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.قدر_2(معلومات_2);
    اكسر;
  }
  }
}

فراغ ASTNode::render_پاڙ(ثابت json::قدر &CurrentCtx,
                         Mustache_Output_وهڪرو &OS) {
  render_ٻار(CurrentCtx, OS);
}

فراغ ASTNode::render_متن(Mustache_Output_وهڪرو &OS) { OS << Body; }

فراغ ASTNode::renderPartial(ثابت json::قدر &CurrentCtx,
                            Mustache_Output_وهڪرو &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_قدر[0]
                    << ", Indentation:" << Indentation << "\n");
  خودڪار Partial = Ctx.Partials.ڳوليو(Accessor_قدر[0]);
  إذا (Partial != Ctx.Partials.پڇاڙي())
    renderPartial(CurrentCtx, OS, Partial->get_قدر());
}

فراغ ASTNode::renderVariable(ثابت json::قدر &CurrentCtx,
                             Mustache_Output_وهڪرو &OS) {
  خودڪار Lambda = Ctx.Lambdas.ڳوليو(Accessor_قدر[0]);
  إذا (Lambda != Ctx.Lambdas.پڇاڙي()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_قدر());
  } وإلا إذا (ثابت json::قدر *ContextPtr = ڳوليو_Context()) {
    Escape_String_وهڪرو ES(OS, Ctx.Escapes);
    منزل_Mustache_String(*ContextPtr, ES);
  }
}

فراغ ASTNode::renderUnescapeVariable(ثابت json::قدر &CurrentCtx,
                                     Mustache_Output_وهڪرو &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_قدر[0]
                    << "\n");
  خودڪار Lambda = Ctx.Lambdas.ڳوليو(Accessor_قدر[0]);
  إذا (Lambda != Ctx.Lambdas.پڇاڙي()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_قدر());
  } وإلا إذا (ثابت json::قدر *ContextPtr = ڳوليو_Context()) {
    OS.suspendIndentation();
    منزل_Mustache_String(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

فراغ ASTNode::renderSection(ثابت json::قدر &CurrentCtx,
                            Mustache_Output_وهڪرو &OS) {
  خودڪار SectionLambda = Ctx.SectionLambdas.ڳوليو(Accessor_قدر[0]);
  إذا (SectionLambda != Ctx.SectionLambdas.پڇاڙي()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_قدر());
    أرجع;
  }

  ثابت json::قدر *ContextPtr = ڳوليو_Context();
  إذا (isContextFalsey(ContextPtr))
    أرجع;

  إذا (ثابت json::Array *Arr = ContextPtr->getAsArray()) {
    لكل (ثابت json::قدر &V : *Arr)
      render_ٻار(V, OS);
    أرجع;
  }
  render_ٻار(*ContextPtr, OS);
}

فراغ ASTNode::renderInvertSection(ثابت json::قدر &CurrentCtx,
                                  Mustache_Output_وهڪرو &OS) {
  منطقي IsLambda = Ctx.SectionLambdas.شامل_آهي(Accessor_قدر[0]);
  ثابت json::قدر *ContextPtr = ڳوليو_Context();
  إذا (isContextFalsey(ContextPtr) && !IsLambda) {
    render_ٻار(CurrentCtx, OS);
  }
}

فراغ ASTNode::render(ثابت llvm::json::قدر &معلومات_2, Mustache_Output_وهڪرو &OS) {
  إذا (Ty != پاڙ_2 && Ty != متن_2 && Accessor_قدر.خالي_جايون())
    أرجع;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  والد_Context = &معلومات_2;

  بدل (Ty) {
  حالة پاڙ_2:
    render_پاڙ(معلومات_2, OS);
    أرجع;
  حالة متن_2:
    render_متن(OS);
    أرجع;
  حالة Partial:
    renderPartial(معلومات_2, OS);
    أرجع;
  حالة Variable:
    renderVariable(معلومات_2, OS);
    أرجع;
  حالة UnescapeVariable:
    renderUnescapeVariable(معلومات_2, OS);
    أرجع;
  حالة Section:
    renderSection(معلومات_2, OS);
    أرجع;
  حالة InvertSection:
    renderInvertSection(معلومات_2, OS);
    أرجع;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ثابت json::قدر *ASTNode::ڳوليو_Context() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  إذا (Accessor_قدر.خالي_جايون())
    أرجع خالي_اشارو;
  إذا (Accessor_قدر[0] == ".")
    أرجع والد_Context;

  ثابت json::Object *CurrentContext = والد_Context->getAsObject();
  StringRef CurrentAccessor = Accessor_قدر[0];
  ASTNode *Current_والد = والد_2;

  طالما (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    إذا (Current_والد->Ty != پاڙ_2) {
      CurrentContext = Current_والد->والد_Context->getAsObject();
      Current_والد = Current_والد->والد_2;
      استمر;
    }
    أرجع خالي_اشارو;
  }
  ثابت json::قدر *Context = خالي_اشارو;
  لكل (خودڪار [Idx, Acc] : enumerate(Accessor_قدر)) {
    ثابت json::قدر *Current_قدر = CurrentContext->get(Acc);
    إذا (!Current_قدر)
      أرجع خالي_اشارو;
    إذا (Idx < Accessor_قدر.سائيز_2() - 1) {
      CurrentContext = Current_قدر->getAsObject();
      إذا (!CurrentContext)
        أرجع خالي_اشارو;
    } وإلا {
      Context = Current_قدر;
    }
  }
  أرجع Context;
}

فراغ ASTNode::render_ٻار(ثابت json::قدر &Contexts,
                          Mustache_Output_وهڪرو &OS) {
  لكل (ASTNode &ٻار : Children)
    ٻار.render(Contexts, OS);
}

فراغ ASTNode::renderPartial(ثابت json::قدر &Contexts,
                            Mustache_Output_وهڪرو &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  جوڙيو_Indentation_String_وهڪرو IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

فراغ ASTNode::renderLambdas(ثابت llvm::json::قدر &Contexts,
                            Mustache_Output_وهڪرو &OS, Lambda &L) {
  json::قدر Lambda_نتيجو = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  منزل_Mustache_String(Lambda_نتيجو, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ڳنڍ = P.parse();

  Escape_String_وهڪرو ES(OS, Ctx.Escapes);
  إذا (Ty == Variable) {
    Lambda_ڳنڍ->render(Contexts, ES);
    أرجع;
  }
  Lambda_ڳنڍ->render(Contexts, OS);
}

فراغ ASTNode::renderSectionLambdas(ثابت llvm::json::قدر &Contexts,
                                   Mustache_Output_وهڪرو &OS, SectionLambda &L) {
  json::قدر Return = L(RawBody.str());
  إذا (isFalsey(Return))
    أرجع;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  منزل_Mustache_String(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ڳنڍ = P.parse();
  Lambda_ڳنڍ->render(Contexts, OS);
}

فراغ Template::render(ثابت llvm::json::قدر &معلومات_2, llvm::raw_ostream &OS) {
  Raw_Mustache_Output_وهڪرو MOS(OS);
  وڻ->render(معلومات_2, MOS);
}

فراغ Template::رجسٽر_Partial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_وڻ = P.parse();
  Ctx.Partials.وجهو(std::make_pair(Name, Partial_وڻ));
}

فراغ Template::رجسٽر_Lambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

فراغ Template::رجسٽر_Lambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

فراغ Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  وڻ = P.parse();
  // The default behavior is to escape html entities.
  ثابت EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), وڻ(Other.وڻ) {
  Other.وڻ = خالي_اشارو;
}

Template::~Template() = افتراضي;

} // namespace llvm::mustache

#وصف_رد DEBUG_TYPE

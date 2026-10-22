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

#تعریف_کرو DEBUG_TYPE "mustache"

ورتوں نطاق llvm;
ورتوں نطاق llvm::mustache;

نطاق {

ورتوں Accessor = ArrayRef<StringRef>;

ساكن منطقي isFalsey(ثابت json::قدر &V) {
  أرجع V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().قدر_2()) ||
         (V.getAsArray() && V.getAsArray()->خالی_تھاواں());
}

ساكن منطقي isContextFalsey(ثابت json::قدر *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  إذا (!V)
    أرجع سچ;
  أرجع isFalsey(*V);
}

ساكن فراغ ونڈو_And_Trim(StringRef Str, Small_سمتیہ_Impl<StringRef> &ٹوکن) {
  size_t CurrentPos = 0;
  طالما (CurrentPos < Str.ناپ_2()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.لبھو('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    إذا (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.ناپ_2();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.ٹکڑا(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t شروع = Part.لبھو_پہلا_not_دا(" \t\r\n");
    إذا (شروع != StringRef::npos) {
      size_t سرا_2 = Part.لبھو_آخری_not_دا(" \t\r\n");
      ٹوکن.ڈالو_پٹھ(Part.ٹکڑا(شروع, سرا_2 + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

ساكن Accessor ونڈو_Mustache_String(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  Small_سمتیہ<StringRef> ٹوکن;
  إذا (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    ٹوکن.ڈالو_پٹھ(".");
  } وإلا {
    ونڈو_And_Trim(Str, ٹوکن);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *Arena_ٹوکن = Ctx.Allocator.Allocate<StringRef>(ٹوکن.ناپ_2());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(ٹوکن, Arena_ٹوکن);
  // Return an ArrayRef pointing to the stable arena memory.
  أرجع ArrayRef<StringRef>(Arena_ٹوکن, ٹوکن.ناپ_2());
}
} // namespace

نطاق llvm::mustache {

صنف Mustache_Output_دھارا : عام raw_ostream {
عام:
  Mustache_Output_دھارا() = افتراضي;
  ~Mustache_Output_دھارا() override = افتراضي;

  مجازی فراغ suspendIndentation() {}
  مجازی فراغ resumeIndentation() {}

خاص:
  فراغ anchor() override;
};

فراغ Mustache_Output_دھارا::anchor() {}

صنف Raw_Mustache_Output_دھارا : عام Mustache_Output_دھارا {
عام:
  Raw_Mustache_Output_دھارا(raw_ostream &OS) : OS(OS) { سیٹ_Unbuffered(); }

خاص:
  raw_ostream &OS;

  فراغ write_impl(ثابت حرف *Ptr, size_t ناپ) override {
    OS.اكتب(Ptr, ناپ);
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
    سیٹ_Delimiter,
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
    Accessor_قدر = ونڈو_Mustache_String(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ثابت { أرجع Accessor_قدر; }

  Type getType() ثابت { أرجع TokenType; }

  فراغ سیٹ_Indentation(size_t NewIndentation) { Indentation = NewIndentation; }

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
      أرجع Type::سیٹ_Delimiter;
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

ورتوں EscapeMap = DenseMap<حرف, std::string>;

صنف ASTNode : عام ilist_گرہ<ASTNode> {
عام:
  تعداد Type {
    جڑ_2,
    متن_2,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::جڑ_2), ماپا_2(خالی_سوچک), ماپا_Context(خالی_سوچک) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *ماپا_2)
      : Ctx(Ctx), Ty(Type::متن_2), Body(Body), ماپا_2(ماپا_2),
        ماپا_Context(خالی_سوچک) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *ماپا_2)
      : Ctx(Ctx), Ty(Ty), ماپا_2(ماپا_2), Accessor_قدر(Accessor),
        ماپا_Context(خالی_سوچک) {}

  فراغ جوڑو_بچہ(AstPtr بچہ) { Children.ڈالو_پٹھ(بچہ); };

  فراغ سیٹ_Raw_Body(StringRef NewBody) { RawBody = NewBody; };

  فراغ سیٹ_Indentation(size_t NewIndentation) { Indentation = NewIndentation; };

  فراغ render(ثابت llvm::json::قدر &انکڑے_2, Mustache_Output_دھارا &OS);

خاص:
  فراغ renderLambdas(ثابت llvm::json::قدر &Contexts,
                     Mustache_Output_دھارا &OS, Lambda &L);

  فراغ renderSectionLambdas(ثابت llvm::json::قدر &Contexts,
                            Mustache_Output_دھارا &OS, SectionLambda &L);

  فراغ renderPartial(ثابت llvm::json::قدر &Contexts,
                     Mustache_Output_دھارا &OS, ASTNode *Partial);

  فراغ render_بچہ(ثابت llvm::json::قدر &Context, Mustache_Output_دھارا &OS);

  ثابت llvm::json::قدر *لبھو_Context();

  فراغ render_جڑ(ثابت json::قدر &CurrentCtx, Mustache_Output_دھارا &OS);
  فراغ render_متن(Mustache_Output_دھارا &OS);
  فراغ renderPartial(ثابت json::قدر &CurrentCtx, Mustache_Output_دھارا &OS);
  فراغ renderVariable(ثابت json::قدر &CurrentCtx, Mustache_Output_دھارا &OS);
  فراغ renderUnescapeVariable(ثابت json::قدر &CurrentCtx,
                              Mustache_Output_دھارا &OS);
  فراغ renderSection(ثابت json::قدر &CurrentCtx, Mustache_Output_دھارا &OS);
  فراغ renderInvertSection(ثابت json::قدر &CurrentCtx,
                           Mustache_Output_دھارا &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *ماپا_2;
  ASTNodeList Children;
  ثابت ArrayRef<StringRef> Accessor_قدر;
  ثابت llvm::json::قدر *ماپا_Context;
};

// A wrapper for arena allocator for ASTNodes
ساكن AstPtr create_جڑ_گرہ(MustacheContext &Ctx) {
  أرجع جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

ساكن AstPtr create_گرہ(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *ماپا_2) {
  أرجع جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, ماپا_2);
}

ساكن AstPtr create_متن_گرہ(MustacheContext &Ctx, StringRef Body,
                             ASTNode *ماپا_2) {
  أرجع جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, ماپا_2);
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
ساكن منطقي has_متن_Behind(size_t Idx, ثابت ArrayRef<Token> &ٹوکن) {
  إذا (Idx == 0)
    أرجع سچ;

  size_t PrevIdx = Idx - 1;
  إذا (ٹوکن[PrevIdx].getType() != Token::Type::متن_2)
    أرجع سچ;

  ثابت Token &PrevToken = ٹوکن[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  أرجع !TokenBody.سرے_with("\n") && !(TokenBody.خالی_تھاواں() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
ساكن منطقي has_متن_Ahead(size_t Idx, ثابت ArrayRef<Token> &ٹوکن) {
  إذا (Idx >= ٹوکن.ناپ_2() - 1)
    أرجع سچ;

  size_t اگلا_Idx = Idx + 1;
  إذا (ٹوکن[اگلا_Idx].getType() != Token::Type::متن_2)
    أرجع سچ;

  ثابت Token &اگلا_Token = ٹوکن[اگلا_Idx];
  StringRef TokenBody = StringRef(اگلا_Token.RawBody).ltrim(" ");
  أرجع !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

ساكن منطقي requires_Clean_اتے(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  أرجع T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::سیٹ_Delimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
ساكن فراغ stripTokenAhead(Small_سمتیہ_Impl<Token> &ٹوکن, size_t Idx) {
  Token &اگلا_Token = ٹوکن[Idx + 1];
  StringRef اگلا_Token_Body = اگلا_Token.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  إذا (اگلا_Token_Body.starts_with("\r\n"))
    اگلا_Token.TokenBody = اگلا_Token_Body.substr(2);
  وإلا إذا (اگلا_Token_Body.starts_with("\n"))
    اگلا_Token.TokenBody = اگلا_Token_Body.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
فراغ stripTokenBefore(Small_سمتیہ_Impl<Token> &ٹوکن, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = ٹوکن[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.ناپ_2() - Unindented.ناپ_2();
  PrevToken.TokenBody = Unindented;
  CurrentToken.سیٹ_Indentation(Indentation);
}

بنية ٹیگ {
  تعداد صنف قسم_2 {
    None,
    عام_2, // {{...}}
    Triple, // {{{...}}}
  };

  قسم_2 ٹیگ_قسم = قسم_2::None;
  StringRef Content;   // The content between the delimiters.
  StringRef بھریا_میل; // The entire tag, including delimiters.
  size_t شروع_Position = StringRef::npos;
};

[[maybe_بن_ورتیا]] ساكن ثابت حرف *ٹیگ_قسم_منزل_String(ٹیگ::قسم_2 K) {
  بدل (K) {
  حالة ٹیگ::قسم_2::None:
    أرجع "None";
  حالة ٹیگ::قسم_2::عام_2:
    أرجع "Normal";
  حالة ٹیگ::قسم_2::Triple:
    أرجع "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_بن_ورتیا]] ساكن ثابت حرف *json_قسم_منزل_String(json::قدر::قسم_2 K) {
  بدل (K) {
  حالة json::قدر::قسم_2::Null:
    أرجع "JSON_KIND_NULL";
  حالة json::قدر::قسم_2::Boolean:
    أرجع "JSON_KIND_BOOLEAN";
  حالة json::قدر::قسم_2::نمبر:
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
ساكن Small_سمتیہ<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  Small_سمتیہ<Token> ٹوکن;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t متن_شروع = 0;

  ثابت StringLiteral TripleOpen("{{{");
  ثابت StringLiteral TripleClose("}}}");

  طالما (Cursor < Template.ناپ_2()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef ٹیگ_Open, ٹیگ_Close;
    ٹیگ::قسم_2 قسم_2;

    // Determine which tag we've encountered.
    إذا (TemplateSuffix.starts_with(TripleOpen)) {
      قسم_2 = ٹیگ::قسم_2::Triple;
      ٹیگ_Open = TripleOpen;
      ٹیگ_Close = TripleClose;
    } وإلا إذا (TemplateSuffix.starts_with(Open)) {
      قسم_2 = ٹیگ::قسم_2::عام_2;
      ٹیگ_Open = Open;
      ٹیگ_Close = Close;
    } وإلا {
      // Not at a tag, continue scanning.
      ++Cursor;
      استمر;
    }

    // Found a tag, first add the preceding text.
    إذا (Cursor > متن_شروع)
      ٹوکن.emplace_پٹھ(Template.ٹکڑا(متن_شروع, Cursor));

    // Find the closing tag.
    size_t سرا_Pos = Template.لبھو(ٹیگ_Close, Cursor + ٹیگ_Open.ناپ_2());
    إذا (سرا_Pos == StringRef::npos) {
      // No closing tag, the rest is text.
      ٹوکن.emplace_پٹھ(Template.substr(Cursor));
      متن_شروع = Cursor = Template.ناپ_2();
      اكسر;
    }

    // Extract tag content and full match.
    size_t Content_شروع = Cursor + ٹیگ_Open.ناپ_2();
    StringRef Content = Template.substr(Content_شروع, سرا_Pos - Content_شروع);
    StringRef بھریا_میل =
        Template.substr(Cursor, (سرا_Pos + ٹیگ_Close.ناپ_2()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << بھریا_میل << ", Content: " << Content
                      << ", Kind: " << ٹیگ_قسم_منزل_String(قسم_2) << "\n");
    إذا (قسم_2 == ٹیگ::قسم_2::Triple) {
      ٹوکن.emplace_پٹھ(بھریا_میل, Ctx.Saver.save("&" + Content), '&', Ctx);
    } وإلا { // Normal Tag
      StringRef Interpolated = Content;
      إذا (!Interpolated.trim().starts_with("=")) {
        حرف Front = Interpolated.خالی_تھاواں() ? ' ' : Interpolated.trim().front();
        ٹوکن.emplace_پٹھ(بھریا_میل, Interpolated, Front, Ctx);
      } وإلا { // Set Delimiter
        ٹوکن.emplace_پٹھ(بھریا_میل, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](حرف C) { أرجع C == '='; });
        DelimSpec = DelimSpec.trim();

        خودکار [NewOpen, NewClose] = DelimSpec.ونڈو(' ');
        LLVM_DEBUG(dbgs() << "[Set Delimiter] NewOpen: " << NewOpen
                          << ", NewClose: " << NewClose << "\n");
        Open = NewOpen;
        Close = NewClose;
      }
    }

    // Move past the tag for the next iteration.
    Cursor += بھریا_میل.ناپ_2();
    متن_شروع = Cursor;
  }

  // Add any remaining text after the last tag.
  إذا (متن_شروع < Template.ناپ_2())
    ٹوکن.emplace_پٹھ(Template.substr(متن_شروع));

  // Fix up white spaces for standalone tags.
  size_t آخری_Idx = ٹوکن.ناپ_2() - 1;
  لكل (size_t Idx = 0, سرا_2 = ٹوکن.ناپ_2(); Idx < سرا_2; ++Idx) {
    Token &CurrentToken = ٹوکن[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    إذا (!requires_Clean_اتے(CurrentType))
      استمر;

    منطقي Has_متن_Behind = has_متن_Behind(Idx, ٹوکن);
    منطقي Has_متن_Ahead = has_متن_Ahead(Idx, ٹوکن);

    إذا ((!Has_متن_Ahead && !Has_متن_Behind) || (!Has_متن_Ahead && Idx == 0))
      stripTokenAhead(ٹوکن, Idx);

    إذا ((!Has_متن_Behind && !Has_متن_Ahead) || (!Has_متن_Behind && Idx == آخری_Idx))
      stripTokenBefore(ٹوکن, Idx, CurrentToken, CurrentType);
  }
  أرجع ٹوکن;
}

// Custom stream to escape strings.
صنف Escape_String_دھارا : عام Mustache_Output_دھارا {
عام:
  واضح Escape_String_دھارا(llvm::raw_ostream &Wrapped_دھارا,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.کلیدیں().begin(), Escape.کلیدیں().سرا()),
        Wrapped_دھارا(Wrapped_دھارا) {
    سیٹ_Unbuffered();
  }

محمي:
  فراغ write_impl(ثابت حرف *Ptr, size_t ناپ) override {
    StringRef انکڑے_2(Ptr, ناپ);
    size_t شروع = 0;
    طالما (شروع < ناپ) {
      // Find the next character that needs to be escaped.
      size_t اگلا_2 = انکڑے_2.لبھو_پہلا_دا(EscapeChars.str(), شروع);

      // If no escapable characters are found, write the rest of the string.
      إذا (اگلا_2 == StringRef::npos) {
        Wrapped_دھارا << انکڑے_2.substr(شروع);
        أرجع;
      }

      // Write the chunk of text before the escapable character.
      إذا (اگلا_2 > شروع)
        Wrapped_دھارا << انکڑے_2.substr(شروع, اگلا_2 - شروع);

      // Look up and write the escaped version of the character.
      Wrapped_دھارا << Escape[انکڑے_2[اگلا_2]];
      شروع = اگلا_2 + 1;
    }
  }

  uint64_t current_pos() ثابت override { أرجع Wrapped_دھارا.tell(); }

خاص:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &Wrapped_دھارا;
};

// Custom stream to add indentation used to for rendering partials.
صنف جوڑو_Indentation_String_دھارا : عام Mustache_Output_دھارا {
عام:
  واضح جوڑو_Indentation_String_دھارا(raw_ostream &Wrapped_دھارا,
                                      size_t Indentation)
      : Indentation(Indentation), Wrapped_دھارا(Wrapped_دھارا),
        NeedsIndent(سچ), IsSuspended(خطأ) {
    سیٹ_Unbuffered();
  }

  فراغ suspendIndentation() override { IsSuspended = سچ; }
  فراغ resumeIndentation() override { IsSuspended = خطأ; }

محمي:
  فراغ write_impl(ثابت حرف *Ptr, size_t ناپ) override {
    llvm::StringRef انکڑے_2(Ptr, ناپ);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    لكل (حرف C : انکڑے_2) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      إذا (NeedsIndent && C != '\n') {
        Wrapped_دھارا << Indent;
        NeedsIndent = خطأ;
      }
      Wrapped_دھارا << C;
      إذا (C == '\n' && !IsSuspended)
        NeedsIndent = سچ;
    }
  }

  uint64_t current_pos() ثابت override { أرجع Wrapped_دھارا.tell(); }

خاص:
  size_t Indentation;
  raw_ostream &Wrapped_دھارا;
  منطقي NeedsIndent;
  منطقي IsSuspended;
};

صنف Parser {
عام:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

خاص:
  فراغ parseMustache(ASTNode *ماپا_2);
  فراغ parseSection(ASTNode *ماپا_2, ASTNode::Type Ty, ثابت Accessor &A);

  MustacheContext &Ctx;
  Small_سمتیہ<Token> ٹوکن;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

فراغ Parser::parseSection(ASTNode *ماپا_2, ASTNode::Type Ty,
                          ثابت Accessor &A) {
  AstPtr Current_گرہ = create_گرہ(Ctx, Ty, A, ماپا_2);
  size_t شروع = CurrentPtr;
  parseMustache(Current_گرہ);
  ثابت size_t سرا_2 = CurrentPtr - 1;

  size_t Raw_Body_ناپ = 0;
  لكل (size_t I = شروع; I < سرا_2; ++I)
    Raw_Body_ناپ += ٹوکن[I].RawBody.ناپ_2();

  SmallString<128> RawBody;
  RawBody.reserve(Raw_Body_ناپ);
  لكل (std::size_t I = شروع; I < سرا_2; ++I)
    RawBody += ٹوکن[I].RawBody;

  Current_گرہ->سیٹ_Raw_Body(Ctx.Saver.save(StringRef(RawBody)));
  ماپا_2->جوڑو_بچہ(Current_گرہ);
}

AstPtr Parser::parse() {
  ٹوکن = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr جڑ_گرہ = create_جڑ_گرہ(Ctx);
  parseMustache(جڑ_گرہ);
  أرجع جڑ_گرہ;
}

فراغ Parser::parseMustache(ASTNode *ماپا_2) {

  طالما (CurrentPtr < ٹوکن.ناپ_2()) {
    Token CurrentToken = ٹوکن[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_گرہ;

    بدل (CurrentToken.getType()) {
    حالة Token::Type::متن_2: {
      Current_گرہ = create_متن_گرہ(Ctx, CurrentToken.TokenBody, ماپا_2);
      ماپا_2->جوڑو_بچہ(Current_گرہ);
      اكسر;
    }
    حالة Token::Type::Variable: {
      Current_گرہ = create_گرہ(Ctx, ASTNode::Variable, A, ماپا_2);
      ماپا_2->جوڑو_بچہ(Current_گرہ);
      اكسر;
    }
    حالة Token::Type::UnescapeVariable: {
      Current_گرہ = create_گرہ(Ctx, ASTNode::UnescapeVariable, A, ماپا_2);
      ماپا_2->جوڑو_بچہ(Current_گرہ);
      اكسر;
    }
    حالة Token::Type::Partial: {
      Current_گرہ = create_گرہ(Ctx, ASTNode::Partial, A, ماپا_2);
      Current_گرہ->سیٹ_Indentation(CurrentToken.getIndentation());
      ماپا_2->جوڑو_بچہ(Current_گرہ);
      اكسر;
    }
    حالة Token::Type::SectionOpen: {
      parseSection(ماپا_2, ASTNode::Section, A);
      اكسر;
    }
    حالة Token::Type::InvertSectionOpen: {
      parseSection(ماپا_2, ASTNode::InvertSection, A);
      اكسر;
    }
    حالة Token::Type::Comment:
    حالة Token::Type::سیٹ_Delimiter:
      اكسر;
    حالة Token::Type::SectionClose:
      أرجع;
    }
  }
}
ساكن فراغ منزل_Mustache_String(ثابت json::قدر &انکڑے_2, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << json_قسم_منزل_String(انکڑے_2.قسم()) << ", Data: " << انکڑے_2
                    << "\n");
  بدل (انکڑے_2.قسم()) {
  حالة json::قدر::Null:
    أرجع;
  حالة json::قدر::نمبر: {
    خودکار عدد = *انکڑے_2.get_As_نمبر();
    std::ostringstream SS;
    SS << عدد;
    OS << SS.str();
    أرجع;
  }
  حالة json::قدر::String: {
    OS << *انکڑے_2.getAsString();
    أرجع;
  }

  حالة json::قدر::Array: {
    خودکار Arr = *انکڑے_2.getAsArray();
    إذا (Arr.خالی_تھاواں())
      أرجع;
    [[fallthrough]];
  }
  حالة json::قدر::Object:
  حالة json::قدر::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.قدر_2(انکڑے_2);
    اكسر;
  }
  }
}

فراغ ASTNode::render_جڑ(ثابت json::قدر &CurrentCtx,
                         Mustache_Output_دھارا &OS) {
  render_بچہ(CurrentCtx, OS);
}

فراغ ASTNode::render_متن(Mustache_Output_دھارا &OS) { OS << Body; }

فراغ ASTNode::renderPartial(ثابت json::قدر &CurrentCtx,
                            Mustache_Output_دھارا &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_قدر[0]
                    << ", Indentation:" << Indentation << "\n");
  خودکار Partial = Ctx.Partials.لبھو(Accessor_قدر[0]);
  إذا (Partial != Ctx.Partials.سرا())
    renderPartial(CurrentCtx, OS, Partial->get_قدر());
}

فراغ ASTNode::renderVariable(ثابت json::قدر &CurrentCtx,
                             Mustache_Output_دھارا &OS) {
  خودکار Lambda = Ctx.Lambdas.لبھو(Accessor_قدر[0]);
  إذا (Lambda != Ctx.Lambdas.سرا()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_قدر());
  } وإلا إذا (ثابت json::قدر *ContextPtr = لبھو_Context()) {
    Escape_String_دھارا ES(OS, Ctx.Escapes);
    منزل_Mustache_String(*ContextPtr, ES);
  }
}

فراغ ASTNode::renderUnescapeVariable(ثابت json::قدر &CurrentCtx,
                                     Mustache_Output_دھارا &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_قدر[0]
                    << "\n");
  خودکار Lambda = Ctx.Lambdas.لبھو(Accessor_قدر[0]);
  إذا (Lambda != Ctx.Lambdas.سرا()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_قدر());
  } وإلا إذا (ثابت json::قدر *ContextPtr = لبھو_Context()) {
    OS.suspendIndentation();
    منزل_Mustache_String(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

فراغ ASTNode::renderSection(ثابت json::قدر &CurrentCtx,
                            Mustache_Output_دھارا &OS) {
  خودکار SectionLambda = Ctx.SectionLambdas.لبھو(Accessor_قدر[0]);
  إذا (SectionLambda != Ctx.SectionLambdas.سرا()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_قدر());
    أرجع;
  }

  ثابت json::قدر *ContextPtr = لبھو_Context();
  إذا (isContextFalsey(ContextPtr))
    أرجع;

  إذا (ثابت json::Array *Arr = ContextPtr->getAsArray()) {
    لكل (ثابت json::قدر &V : *Arr)
      render_بچہ(V, OS);
    أرجع;
  }
  render_بچہ(*ContextPtr, OS);
}

فراغ ASTNode::renderInvertSection(ثابت json::قدر &CurrentCtx,
                                  Mustache_Output_دھارا &OS) {
  منطقي IsLambda = Ctx.SectionLambdas.شامل_اے(Accessor_قدر[0]);
  ثابت json::قدر *ContextPtr = لبھو_Context();
  إذا (isContextFalsey(ContextPtr) && !IsLambda) {
    render_بچہ(CurrentCtx, OS);
  }
}

فراغ ASTNode::render(ثابت llvm::json::قدر &انکڑے_2, Mustache_Output_دھارا &OS) {
  إذا (Ty != جڑ_2 && Ty != متن_2 && Accessor_قدر.خالی_تھاواں())
    أرجع;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ماپا_Context = &انکڑے_2;

  بدل (Ty) {
  حالة جڑ_2:
    render_جڑ(انکڑے_2, OS);
    أرجع;
  حالة متن_2:
    render_متن(OS);
    أرجع;
  حالة Partial:
    renderPartial(انکڑے_2, OS);
    أرجع;
  حالة Variable:
    renderVariable(انکڑے_2, OS);
    أرجع;
  حالة UnescapeVariable:
    renderUnescapeVariable(انکڑے_2, OS);
    أرجع;
  حالة Section:
    renderSection(انکڑے_2, OS);
    أرجع;
  حالة InvertSection:
    renderInvertSection(انکڑے_2, OS);
    أرجع;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ثابت json::قدر *ASTNode::لبھو_Context() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  إذا (Accessor_قدر.خالی_تھاواں())
    أرجع خالی_سوچک;
  إذا (Accessor_قدر[0] == ".")
    أرجع ماپا_Context;

  ثابت json::Object *CurrentContext = ماپا_Context->getAsObject();
  StringRef CurrentAccessor = Accessor_قدر[0];
  ASTNode *Current_ماپا = ماپا_2;

  طالما (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    إذا (Current_ماپا->Ty != جڑ_2) {
      CurrentContext = Current_ماپا->ماپا_Context->getAsObject();
      Current_ماپا = Current_ماپا->ماپا_2;
      استمر;
    }
    أرجع خالی_سوچک;
  }
  ثابت json::قدر *Context = خالی_سوچک;
  لكل (خودکار [Idx, Acc] : enumerate(Accessor_قدر)) {
    ثابت json::قدر *Current_قدر = CurrentContext->get(Acc);
    إذا (!Current_قدر)
      أرجع خالی_سوچک;
    إذا (Idx < Accessor_قدر.ناپ_2() - 1) {
      CurrentContext = Current_قدر->getAsObject();
      إذا (!CurrentContext)
        أرجع خالی_سوچک;
    } وإلا {
      Context = Current_قدر;
    }
  }
  أرجع Context;
}

فراغ ASTNode::render_بچہ(ثابت json::قدر &Contexts,
                          Mustache_Output_دھارا &OS) {
  لكل (ASTNode &بچہ : Children)
    بچہ.render(Contexts, OS);
}

فراغ ASTNode::renderPartial(ثابت json::قدر &Contexts,
                            Mustache_Output_دھارا &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  جوڑو_Indentation_String_دھارا IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

فراغ ASTNode::renderLambdas(ثابت llvm::json::قدر &Contexts,
                            Mustache_Output_دھارا &OS, Lambda &L) {
  json::قدر Lambda_نتیجہ = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  منزل_Mustache_String(Lambda_نتیجہ, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_گرہ = P.parse();

  Escape_String_دھارا ES(OS, Ctx.Escapes);
  إذا (Ty == Variable) {
    Lambda_گرہ->render(Contexts, ES);
    أرجع;
  }
  Lambda_گرہ->render(Contexts, OS);
}

فراغ ASTNode::renderSectionLambdas(ثابت llvm::json::قدر &Contexts,
                                   Mustache_Output_دھارا &OS, SectionLambda &L) {
  json::قدر Return = L(RawBody.str());
  إذا (isFalsey(Return))
    أرجع;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  منزل_Mustache_String(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_گرہ = P.parse();
  Lambda_گرہ->render(Contexts, OS);
}

فراغ Template::render(ثابت llvm::json::قدر &انکڑے_2, llvm::raw_ostream &OS) {
  Raw_Mustache_Output_دھارا MOS(OS);
  درخت->render(انکڑے_2, MOS);
}

فراغ Template::رجسٹر_Partial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_درخت = P.parse();
  Ctx.Partials.داخل_کرو(std::make_pair(Name, Partial_درخت));
}

فراغ Template::رجسٹر_Lambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

فراغ Template::رجسٹر_Lambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

فراغ Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  درخت = P.parse();
  // The default behavior is to escape html entities.
  ثابت EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), درخت(Other.درخت) {
  Other.درخت = خالی_سوچک;
}

Template::~Template() = افتراضي;

} // namespace llvm::mustache

#تعریف_رد DEBUG_TYPE

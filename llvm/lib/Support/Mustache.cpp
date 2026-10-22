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

#تعریف_کن DEBUG_TYPE "mustache"

استفاده نطاق llvm;
استفاده نطاق llvm::mustache;

نطاق {

استفاده Accessor = ArrayRef<StringRef>;

ساكن منطقي isFalsey(ثابت json::مقدار &V) {
  أرجع V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().مقدار_2()) ||
         (V.getAsArray() && V.getAsArray()->خالی());
}

ساكن منطقي isContextFalsey(ثابت json::مقدار *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  إذا (!V)
    أرجع درست;
  أرجع isFalsey(*V);
}

ساكن فراغ تقسیم_And_Trim(StringRef Str, Small_وکتور_Impl<StringRef> &نشانهها) {
  size_t CurrentPos = 0;
  طالما (CurrentPos < Str.اندازه_2()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.پیدا_کن('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    إذا (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.اندازه_2();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.برش(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t شروع = Part.پیدا_کن_اولی_not_از(" \t\r\n");
    إذا (شروع != StringRef::npos) {
      size_t انتها_2 = Part.پیدا_کن_آخرین_not_از(" \t\r\n");
      نشانهها.افزودن_پشت(Part.برش(شروع, انتها_2 + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

ساكن Accessor تقسیم_Mustache_String(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  Small_وکتور<StringRef> نشانهها;
  إذا (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    نشانهها.افزودن_پشت(".");
  } وإلا {
    تقسیم_And_Trim(Str, نشانهها);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *Arena_نشانهها = Ctx.Allocator.Allocate<StringRef>(نشانهها.اندازه_2());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(نشانهها, Arena_نشانهها);
  // Return an ArrayRef pointing to the stable arena memory.
  أرجع ArrayRef<StringRef>(Arena_نشانهها, نشانهها.اندازه_2());
}
} // namespace

نطاق llvm::mustache {

صنف Mustache_Output_جریان : عام raw_ostream {
عام:
  Mustache_Output_جریان() = افتراضي;
  ~Mustache_Output_جریان() override = افتراضي;

  مجازی فراغ suspendIndentation() {}
  مجازی فراغ resumeIndentation() {}

خاص:
  فراغ anchor() override;
};

فراغ Mustache_Output_جریان::anchor() {}

صنف Raw_Mustache_Output_جریان : عام Mustache_Output_جریان {
عام:
  Raw_Mustache_Output_جریان(raw_ostream &OS) : OS(OS) { مجموعه_Unbuffered(); }

خاص:
  raw_ostream &OS;

  فراغ write_impl(ثابت حرف *Ptr, size_t اندازه) override {
    OS.اكتب(Ptr, اندازه);
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
    مجموعه_Delimiter,
  };

  Token(StringRef Str)
      : TokenType(Type::متن_2), RawBody(Str), TokenBody(RawBody),
        Accessor_مقدار({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, حرف Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    إذا (TokenType == Type::Comment)
      أرجع;
    StringRef AccessorStr(هذا->TokenBody);
    إذا (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_مقدار = تقسیم_Mustache_String(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ثابت { أرجع Accessor_مقدار; }

  Type getType() ثابت { أرجع TokenType; }

  فراغ مجموعه_Indentation(size_t NewIndentation) { Indentation = NewIndentation; }

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
      أرجع Type::مجموعه_Delimiter;
    افتراضي:
      أرجع Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_مقدار;
  size_t Indentation;
};

استفاده EscapeMap = DenseMap<حرف, std::string>;

صنف ASTNode : عام ilist_گره<ASTNode> {
عام:
  تعداد Type {
    ریشه_2,
    متن_2,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::ریشه_2), والد_2(اشاره_تهی), والد_Context(اشاره_تهی) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *والد_2)
      : Ctx(Ctx), Ty(Type::متن_2), Body(Body), والد_2(والد_2),
        والد_Context(اشاره_تهی) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *والد_2)
      : Ctx(Ctx), Ty(Ty), والد_2(والد_2), Accessor_مقدار(Accessor),
        والد_Context(اشاره_تهی) {}

  فراغ جمع_کن_فرزند(AstPtr فرزند) { Children.افزودن_پشت(فرزند); };

  فراغ مجموعه_Raw_Body(StringRef NewBody) { RawBody = NewBody; };

  فراغ مجموعه_Indentation(size_t NewIndentation) { Indentation = NewIndentation; };

  فراغ render(ثابت llvm::json::مقدار &داده_2, Mustache_Output_جریان &OS);

خاص:
  فراغ renderLambdas(ثابت llvm::json::مقدار &Contexts,
                     Mustache_Output_جریان &OS, Lambda &L);

  فراغ renderSectionLambdas(ثابت llvm::json::مقدار &Contexts,
                            Mustache_Output_جریان &OS, SectionLambda &L);

  فراغ renderPartial(ثابت llvm::json::مقدار &Contexts,
                     Mustache_Output_جریان &OS, ASTNode *Partial);

  فراغ render_فرزند(ثابت llvm::json::مقدار &Context, Mustache_Output_جریان &OS);

  ثابت llvm::json::مقدار *پیدا_کن_Context();

  فراغ render_ریشه(ثابت json::مقدار &CurrentCtx, Mustache_Output_جریان &OS);
  فراغ render_متن(Mustache_Output_جریان &OS);
  فراغ renderPartial(ثابت json::مقدار &CurrentCtx, Mustache_Output_جریان &OS);
  فراغ renderVariable(ثابت json::مقدار &CurrentCtx, Mustache_Output_جریان &OS);
  فراغ renderUnescapeVariable(ثابت json::مقدار &CurrentCtx,
                              Mustache_Output_جریان &OS);
  فراغ renderSection(ثابت json::مقدار &CurrentCtx, Mustache_Output_جریان &OS);
  فراغ renderInvertSection(ثابت json::مقدار &CurrentCtx,
                           Mustache_Output_جریان &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *والد_2;
  ASTNodeList Children;
  ثابت ArrayRef<StringRef> Accessor_مقدار;
  ثابت llvm::json::مقدار *والد_Context;
};

// A wrapper for arena allocator for ASTNodes
ساكن AstPtr create_ریشه_گره(MustacheContext &Ctx) {
  أرجع جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

ساكن AstPtr create_گره(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *والد_2) {
  أرجع جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, والد_2);
}

ساكن AstPtr create_متن_گره(MustacheContext &Ctx, StringRef Body,
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
ساكن منطقي has_متن_Behind(size_t Idx, ثابت ArrayRef<Token> &نشانهها) {
  إذا (Idx == 0)
    أرجع درست;

  size_t PrevIdx = Idx - 1;
  إذا (نشانهها[PrevIdx].getType() != Token::Type::متن_2)
    أرجع درست;

  ثابت Token &PrevToken = نشانهها[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  أرجع !TokenBody.دوسر_with("\n") && !(TokenBody.خالی() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
ساكن منطقي has_متن_Ahead(size_t Idx, ثابت ArrayRef<Token> &نشانهها) {
  إذا (Idx >= نشانهها.اندازه_2() - 1)
    أرجع درست;

  size_t بعدی_Idx = Idx + 1;
  إذا (نشانهها[بعدی_Idx].getType() != Token::Type::متن_2)
    أرجع درست;

  ثابت Token &بعدی_Token = نشانهها[بعدی_Idx];
  StringRef TokenBody = StringRef(بعدی_Token.RawBody).ltrim(" ");
  أرجع !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

ساكن منطقي requires_Clean_به_بالا(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  أرجع T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::مجموعه_Delimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
ساكن فراغ stripTokenAhead(Small_وکتور_Impl<Token> &نشانهها, size_t Idx) {
  Token &بعدی_Token = نشانهها[Idx + 1];
  StringRef بعدی_Token_Body = بعدی_Token.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  إذا (بعدی_Token_Body.starts_with("\r\n"))
    بعدی_Token.TokenBody = بعدی_Token_Body.substr(2);
  وإلا إذا (بعدی_Token_Body.starts_with("\n"))
    بعدی_Token.TokenBody = بعدی_Token_Body.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
فراغ stripTokenBefore(Small_وکتور_Impl<Token> &نشانهها, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = نشانهها[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.اندازه_2() - Unindented.اندازه_2();
  PrevToken.TokenBody = Unindented;
  CurrentToken.مجموعه_Indentation(Indentation);
}

بنية برچسب {
  تعداد صنف نوع_3 {
    None,
    نرمال, // {{...}}
    Triple, // {{{...}}}
  };

  نوع_3 برچسب_نوع = نوع_3::None;
  StringRef Content;   // The content between the delimiters.
  StringRef پر_تطابق; // The entire tag, including delimiters.
  size_t شروع_Position = StringRef::npos;
};

[[maybe_استفادهنشده]] ساكن ثابت حرف *برچسب_نوع_مقصد_String(برچسب::نوع_3 K) {
  بدل (K) {
  حالة برچسب::نوع_3::None:
    أرجع "None";
  حالة برچسب::نوع_3::نرمال:
    أرجع "Normal";
  حالة برچسب::نوع_3::Triple:
    أرجع "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_استفادهنشده]] ساكن ثابت حرف *json_نوع_مقصد_String(json::مقدار::نوع_3 K) {
  بدل (K) {
  حالة json::مقدار::نوع_3::Null:
    أرجع "JSON_KIND_NULL";
  حالة json::مقدار::نوع_3::Boolean:
    أرجع "JSON_KIND_BOOLEAN";
  حالة json::مقدار::نوع_3::عدد:
    أرجع "JSON_KIND_NUMBER";
  حالة json::مقدار::نوع_3::String:
    أرجع "JSON_KIND_STRING";
  حالة json::مقدار::نوع_3::Array:
    أرجع "JSON_KIND_ARRAY";
  حالة json::مقدار::نوع_3::Object:
    أرجع "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
ساكن Small_وکتور<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  Small_وکتور<Token> نشانهها;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t متن_شروع = 0;

  ثابت StringLiteral TripleOpen("{{{");
  ثابت StringLiteral TripleClose("}}}");

  طالما (Cursor < Template.اندازه_2()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef برچسب_Open, برچسب_Close;
    برچسب::نوع_3 نوع_3;

    // Determine which tag we've encountered.
    إذا (TemplateSuffix.starts_with(TripleOpen)) {
      نوع_3 = برچسب::نوع_3::Triple;
      برچسب_Open = TripleOpen;
      برچسب_Close = TripleClose;
    } وإلا إذا (TemplateSuffix.starts_with(Open)) {
      نوع_3 = برچسب::نوع_3::نرمال;
      برچسب_Open = Open;
      برچسب_Close = Close;
    } وإلا {
      // Not at a tag, continue scanning.
      ++Cursor;
      استمر;
    }

    // Found a tag, first add the preceding text.
    إذا (Cursor > متن_شروع)
      نشانهها.emplace_پشت(Template.برش(متن_شروع, Cursor));

    // Find the closing tag.
    size_t انتها_Pos = Template.پیدا_کن(برچسب_Close, Cursor + برچسب_Open.اندازه_2());
    إذا (انتها_Pos == StringRef::npos) {
      // No closing tag, the rest is text.
      نشانهها.emplace_پشت(Template.substr(Cursor));
      متن_شروع = Cursor = Template.اندازه_2();
      اكسر;
    }

    // Extract tag content and full match.
    size_t Content_شروع = Cursor + برچسب_Open.اندازه_2();
    StringRef Content = Template.substr(Content_شروع, انتها_Pos - Content_شروع);
    StringRef پر_تطابق =
        Template.substr(Cursor, (انتها_Pos + برچسب_Close.اندازه_2()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << پر_تطابق << ", Content: " << Content
                      << ", Kind: " << برچسب_نوع_مقصد_String(نوع_3) << "\n");
    إذا (نوع_3 == برچسب::نوع_3::Triple) {
      نشانهها.emplace_پشت(پر_تطابق, Ctx.Saver.save("&" + Content), '&', Ctx);
    } وإلا { // Normal Tag
      StringRef Interpolated = Content;
      إذا (!Interpolated.trim().starts_with("=")) {
        حرف Front = Interpolated.خالی() ? ' ' : Interpolated.trim().front();
        نشانهها.emplace_پشت(پر_تطابق, Interpolated, Front, Ctx);
      } وإلا { // Set Delimiter
        نشانهها.emplace_پشت(پر_تطابق, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](حرف C) { أرجع C == '='; });
        DelimSpec = DelimSpec.trim();

        خودکار [NewOpen, NewClose] = DelimSpec.تقسیم(' ');
        LLVM_DEBUG(dbgs() << "[Set Delimiter] NewOpen: " << NewOpen
                          << ", NewClose: " << NewClose << "\n");
        Open = NewOpen;
        Close = NewClose;
      }
    }

    // Move past the tag for the next iteration.
    Cursor += پر_تطابق.اندازه_2();
    متن_شروع = Cursor;
  }

  // Add any remaining text after the last tag.
  إذا (متن_شروع < Template.اندازه_2())
    نشانهها.emplace_پشت(Template.substr(متن_شروع));

  // Fix up white spaces for standalone tags.
  size_t آخرین_Idx = نشانهها.اندازه_2() - 1;
  لكل (size_t Idx = 0, انتها_2 = نشانهها.اندازه_2(); Idx < انتها_2; ++Idx) {
    Token &CurrentToken = نشانهها[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    إذا (!requires_Clean_به_بالا(CurrentType))
      استمر;

    منطقي Has_متن_Behind = has_متن_Behind(Idx, نشانهها);
    منطقي Has_متن_Ahead = has_متن_Ahead(Idx, نشانهها);

    إذا ((!Has_متن_Ahead && !Has_متن_Behind) || (!Has_متن_Ahead && Idx == 0))
      stripTokenAhead(نشانهها, Idx);

    إذا ((!Has_متن_Behind && !Has_متن_Ahead) || (!Has_متن_Behind && Idx == آخرین_Idx))
      stripTokenBefore(نشانهها, Idx, CurrentToken, CurrentType);
  }
  أرجع نشانهها;
}

// Custom stream to escape strings.
صنف Escape_String_جریان : عام Mustache_Output_جریان {
عام:
  صریح Escape_String_جریان(llvm::raw_ostream &Wrapped_جریان,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.کلیدها().begin(), Escape.کلیدها().انتها()),
        Wrapped_جریان(Wrapped_جریان) {
    مجموعه_Unbuffered();
  }

محمي:
  فراغ write_impl(ثابت حرف *Ptr, size_t اندازه) override {
    StringRef داده_2(Ptr, اندازه);
    size_t شروع = 0;
    طالما (شروع < اندازه) {
      // Find the next character that needs to be escaped.
      size_t بعدی_2 = داده_2.پیدا_کن_اولی_از(EscapeChars.str(), شروع);

      // If no escapable characters are found, write the rest of the string.
      إذا (بعدی_2 == StringRef::npos) {
        Wrapped_جریان << داده_2.substr(شروع);
        أرجع;
      }

      // Write the chunk of text before the escapable character.
      إذا (بعدی_2 > شروع)
        Wrapped_جریان << داده_2.substr(شروع, بعدی_2 - شروع);

      // Look up and write the escaped version of the character.
      Wrapped_جریان << Escape[داده_2[بعدی_2]];
      شروع = بعدی_2 + 1;
    }
  }

  uint64_t current_pos() ثابت override { أرجع Wrapped_جریان.tell(); }

خاص:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &Wrapped_جریان;
};

// Custom stream to add indentation used to for rendering partials.
صنف جمع_کن_Indentation_String_جریان : عام Mustache_Output_جریان {
عام:
  صریح جمع_کن_Indentation_String_جریان(raw_ostream &Wrapped_جریان,
                                      size_t Indentation)
      : Indentation(Indentation), Wrapped_جریان(Wrapped_جریان),
        NeedsIndent(درست), IsSuspended(خطأ) {
    مجموعه_Unbuffered();
  }

  فراغ suspendIndentation() override { IsSuspended = درست; }
  فراغ resumeIndentation() override { IsSuspended = خطأ; }

محمي:
  فراغ write_impl(ثابت حرف *Ptr, size_t اندازه) override {
    llvm::StringRef داده_2(Ptr, اندازه);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    لكل (حرف C : داده_2) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      إذا (NeedsIndent && C != '\n') {
        Wrapped_جریان << Indent;
        NeedsIndent = خطأ;
      }
      Wrapped_جریان << C;
      إذا (C == '\n' && !IsSuspended)
        NeedsIndent = درست;
    }
  }

  uint64_t current_pos() ثابت override { أرجع Wrapped_جریان.tell(); }

خاص:
  size_t Indentation;
  raw_ostream &Wrapped_جریان;
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
  Small_وکتور<Token> نشانهها;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

فراغ Parser::parseSection(ASTNode *والد_2, ASTNode::Type Ty,
                          ثابت Accessor &A) {
  AstPtr Current_گره = create_گره(Ctx, Ty, A, والد_2);
  size_t شروع = CurrentPtr;
  parseMustache(Current_گره);
  ثابت size_t انتها_2 = CurrentPtr - 1;

  size_t Raw_Body_اندازه = 0;
  لكل (size_t I = شروع; I < انتها_2; ++I)
    Raw_Body_اندازه += نشانهها[I].RawBody.اندازه_2();

  SmallString<128> RawBody;
  RawBody.reserve(Raw_Body_اندازه);
  لكل (std::size_t I = شروع; I < انتها_2; ++I)
    RawBody += نشانهها[I].RawBody;

  Current_گره->مجموعه_Raw_Body(Ctx.Saver.save(StringRef(RawBody)));
  والد_2->جمع_کن_فرزند(Current_گره);
}

AstPtr Parser::parse() {
  نشانهها = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr ریشه_گره = create_ریشه_گره(Ctx);
  parseMustache(ریشه_گره);
  أرجع ریشه_گره;
}

فراغ Parser::parseMustache(ASTNode *والد_2) {

  طالما (CurrentPtr < نشانهها.اندازه_2()) {
    Token CurrentToken = نشانهها[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_گره;

    بدل (CurrentToken.getType()) {
    حالة Token::Type::متن_2: {
      Current_گره = create_متن_گره(Ctx, CurrentToken.TokenBody, والد_2);
      والد_2->جمع_کن_فرزند(Current_گره);
      اكسر;
    }
    حالة Token::Type::Variable: {
      Current_گره = create_گره(Ctx, ASTNode::Variable, A, والد_2);
      والد_2->جمع_کن_فرزند(Current_گره);
      اكسر;
    }
    حالة Token::Type::UnescapeVariable: {
      Current_گره = create_گره(Ctx, ASTNode::UnescapeVariable, A, والد_2);
      والد_2->جمع_کن_فرزند(Current_گره);
      اكسر;
    }
    حالة Token::Type::Partial: {
      Current_گره = create_گره(Ctx, ASTNode::Partial, A, والد_2);
      Current_گره->مجموعه_Indentation(CurrentToken.getIndentation());
      والد_2->جمع_کن_فرزند(Current_گره);
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
    حالة Token::Type::مجموعه_Delimiter:
      اكسر;
    حالة Token::Type::SectionClose:
      أرجع;
    }
  }
}
ساكن فراغ مقصد_Mustache_String(ثابت json::مقدار &داده_2, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << json_نوع_مقصد_String(داده_2.نوع_2()) << ", Data: " << داده_2
                    << "\n");
  بدل (داده_2.نوع_2()) {
  حالة json::مقدار::Null:
    أرجع;
  حالة json::مقدار::عدد: {
    خودکار رقم = *داده_2.get_As_عدد();
    std::ostringstream SS;
    SS << رقم;
    OS << SS.str();
    أرجع;
  }
  حالة json::مقدار::String: {
    OS << *داده_2.getAsString();
    أرجع;
  }

  حالة json::مقدار::Array: {
    خودکار Arr = *داده_2.getAsArray();
    إذا (Arr.خالی())
      أرجع;
    [[fallthrough]];
  }
  حالة json::مقدار::Object:
  حالة json::مقدار::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.مقدار_2(داده_2);
    اكسر;
  }
  }
}

فراغ ASTNode::render_ریشه(ثابت json::مقدار &CurrentCtx,
                         Mustache_Output_جریان &OS) {
  render_فرزند(CurrentCtx, OS);
}

فراغ ASTNode::render_متن(Mustache_Output_جریان &OS) { OS << Body; }

فراغ ASTNode::renderPartial(ثابت json::مقدار &CurrentCtx,
                            Mustache_Output_جریان &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_مقدار[0]
                    << ", Indentation:" << Indentation << "\n");
  خودکار Partial = Ctx.Partials.پیدا_کن(Accessor_مقدار[0]);
  إذا (Partial != Ctx.Partials.انتها())
    renderPartial(CurrentCtx, OS, Partial->get_مقدار());
}

فراغ ASTNode::renderVariable(ثابت json::مقدار &CurrentCtx,
                             Mustache_Output_جریان &OS) {
  خودکار Lambda = Ctx.Lambdas.پیدا_کن(Accessor_مقدار[0]);
  إذا (Lambda != Ctx.Lambdas.انتها()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_مقدار());
  } وإلا إذا (ثابت json::مقدار *ContextPtr = پیدا_کن_Context()) {
    Escape_String_جریان ES(OS, Ctx.Escapes);
    مقصد_Mustache_String(*ContextPtr, ES);
  }
}

فراغ ASTNode::renderUnescapeVariable(ثابت json::مقدار &CurrentCtx,
                                     Mustache_Output_جریان &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_مقدار[0]
                    << "\n");
  خودکار Lambda = Ctx.Lambdas.پیدا_کن(Accessor_مقدار[0]);
  إذا (Lambda != Ctx.Lambdas.انتها()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_مقدار());
  } وإلا إذا (ثابت json::مقدار *ContextPtr = پیدا_کن_Context()) {
    OS.suspendIndentation();
    مقصد_Mustache_String(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

فراغ ASTNode::renderSection(ثابت json::مقدار &CurrentCtx,
                            Mustache_Output_جریان &OS) {
  خودکار SectionLambda = Ctx.SectionLambdas.پیدا_کن(Accessor_مقدار[0]);
  إذا (SectionLambda != Ctx.SectionLambdas.انتها()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_مقدار());
    أرجع;
  }

  ثابت json::مقدار *ContextPtr = پیدا_کن_Context();
  إذا (isContextFalsey(ContextPtr))
    أرجع;

  إذا (ثابت json::Array *Arr = ContextPtr->getAsArray()) {
    لكل (ثابت json::مقدار &V : *Arr)
      render_فرزند(V, OS);
    أرجع;
  }
  render_فرزند(*ContextPtr, OS);
}

فراغ ASTNode::renderInvertSection(ثابت json::مقدار &CurrentCtx,
                                  Mustache_Output_جریان &OS) {
  منطقي IsLambda = Ctx.SectionLambdas.شامل_است(Accessor_مقدار[0]);
  ثابت json::مقدار *ContextPtr = پیدا_کن_Context();
  إذا (isContextFalsey(ContextPtr) && !IsLambda) {
    render_فرزند(CurrentCtx, OS);
  }
}

فراغ ASTNode::render(ثابت llvm::json::مقدار &داده_2, Mustache_Output_جریان &OS) {
  إذا (Ty != ریشه_2 && Ty != متن_2 && Accessor_مقدار.خالی())
    أرجع;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  والد_Context = &داده_2;

  بدل (Ty) {
  حالة ریشه_2:
    render_ریشه(داده_2, OS);
    أرجع;
  حالة متن_2:
    render_متن(OS);
    أرجع;
  حالة Partial:
    renderPartial(داده_2, OS);
    أرجع;
  حالة Variable:
    renderVariable(داده_2, OS);
    أرجع;
  حالة UnescapeVariable:
    renderUnescapeVariable(داده_2, OS);
    أرجع;
  حالة Section:
    renderSection(داده_2, OS);
    أرجع;
  حالة InvertSection:
    renderInvertSection(داده_2, OS);
    أرجع;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ثابت json::مقدار *ASTNode::پیدا_کن_Context() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  إذا (Accessor_مقدار.خالی())
    أرجع اشاره_تهی;
  إذا (Accessor_مقدار[0] == ".")
    أرجع والد_Context;

  ثابت json::Object *CurrentContext = والد_Context->getAsObject();
  StringRef CurrentAccessor = Accessor_مقدار[0];
  ASTNode *Current_والد = والد_2;

  طالما (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    إذا (Current_والد->Ty != ریشه_2) {
      CurrentContext = Current_والد->والد_Context->getAsObject();
      Current_والد = Current_والد->والد_2;
      استمر;
    }
    أرجع اشاره_تهی;
  }
  ثابت json::مقدار *Context = اشاره_تهی;
  لكل (خودکار [Idx, Acc] : enumerate(Accessor_مقدار)) {
    ثابت json::مقدار *Current_مقدار = CurrentContext->get(Acc);
    إذا (!Current_مقدار)
      أرجع اشاره_تهی;
    إذا (Idx < Accessor_مقدار.اندازه_2() - 1) {
      CurrentContext = Current_مقدار->getAsObject();
      إذا (!CurrentContext)
        أرجع اشاره_تهی;
    } وإلا {
      Context = Current_مقدار;
    }
  }
  أرجع Context;
}

فراغ ASTNode::render_فرزند(ثابت json::مقدار &Contexts,
                          Mustache_Output_جریان &OS) {
  لكل (ASTNode &فرزند : Children)
    فرزند.render(Contexts, OS);
}

فراغ ASTNode::renderPartial(ثابت json::مقدار &Contexts,
                            Mustache_Output_جریان &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  جمع_کن_Indentation_String_جریان IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

فراغ ASTNode::renderLambdas(ثابت llvm::json::مقدار &Contexts,
                            Mustache_Output_جریان &OS, Lambda &L) {
  json::مقدار Lambda_نتیجه = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  مقصد_Mustache_String(Lambda_نتیجه, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_گره = P.parse();

  Escape_String_جریان ES(OS, Ctx.Escapes);
  إذا (Ty == Variable) {
    Lambda_گره->render(Contexts, ES);
    أرجع;
  }
  Lambda_گره->render(Contexts, OS);
}

فراغ ASTNode::renderSectionLambdas(ثابت llvm::json::مقدار &Contexts,
                                   Mustache_Output_جریان &OS, SectionLambda &L) {
  json::مقدار Return = L(RawBody.str());
  إذا (isFalsey(Return))
    أرجع;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  مقصد_Mustache_String(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_گره = P.parse();
  Lambda_گره->render(Contexts, OS);
}

فراغ Template::render(ثابت llvm::json::مقدار &داده_2, llvm::raw_ostream &OS) {
  Raw_Mustache_Output_جریان MOS(OS);
  درخت->render(داده_2, MOS);
}

فراغ Template::ثبات_Partial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_درخت = P.parse();
  Ctx.Partials.درج(std::make_pair(Name, Partial_درخت));
}

فراغ Template::ثبات_Lambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

فراغ Template::ثبات_Lambda(std::string Name, SectionLambda L) {
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
  Other.درخت = اشاره_تهی;
}

Template::~Template() = افتراضي;

} // namespace llvm::mustache

#لغو_تعریف DEBUG_TYPE

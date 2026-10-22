//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#अन्तर्भाव "llvm/Support/Mustache.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <sstream>

#परिभाषय DEBUG_TYPE "mustache"

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::mustache;

नामाकाश {

उपयुज्य Accessor = ArrayRef<StringRef>;

स्थिर तार्किक isFalsey(ध्रुव json::मानम् &V) {
  प्रत्यागम V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().मानम्_2()) ||
         (V.getAsArray() && V.getAsArray()->रिक्तानि());
}

स्थिर तार्किक isContextFalsey(ध्रुव json::मानम् *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  यदि (!V)
    प्रत्यागम सत्य;
  प्रत्यागम isFalsey(*V);
}

स्थिर शून्य विभज_And_Trim(StringRef Str, Small_सदिशम्_Impl<StringRef> &Tokens) {
  size_t CurrentPos = 0;
  यावत् (CurrentPos < Str.आकार_2()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    यदि (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.आकार_2();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.खण्डिका(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t Start = Part.find_first_not_सम्बन्धि(" \t\r\n");
    यदि (Start != StringRef::npos) {
      size_t अन्तः_2 = Part.find_चरमः_not_सम्बन्धि(" \t\r\n");
      Tokens.प्रक्षिप_पृष्ठभागः(Part.खण्डिका(Start, अन्तः_2 + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

स्थिर Accessor विभज_Mustache_String(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  Small_सदिशम्<StringRef> Tokens;
  यदि (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    Tokens.प्रक्षिप_पृष्ठभागः(".");
  } अन्यथा {
    विभज_And_Trim(Str, Tokens);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.आकार_2());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  प्रत्यागम ArrayRef<StringRef>(ArenaTokens, Tokens.आकार_2());
}
} // namespace

नामाकाश llvm::mustache {

वर्ग Mustache_Output_प्रवाहः : सार्वजनिक raw_ostream {
सार्वजनिक:
  Mustache_Output_प्रवाहः() = स्वभाव;
  ~Mustache_Output_प्रवाहः() override = स्वभाव;

  आभास शून्य suspendIndentation() {}
  आभास शून्य resumeIndentation() {}

गुप्त:
  शून्य anchor() override;
};

शून्य Mustache_Output_प्रवाहः::anchor() {}

वर्ग Raw_Mustache_Output_प्रवाहः : सार्वजनिक Mustache_Output_प्रवाहः {
सार्वजनिक:
  Raw_Mustache_Output_प्रवाहः(raw_ostream &OS) : OS(OS) { समुच्चयः_Unbuffered(); }

गुप्त:
  raw_ostream &OS;

  शून्य write_impl(ध्रुव वर्ण *Ptr, size_t आकार) override {
    OS.लिख(Ptr, आकार);
  }
  uint64_t current_pos() ध्रुव override { प्रत्यागम OS.tell(); }
};

वर्ग Token {
सार्वजनिक:
  गणना वर्ग Type {
    Text,
    Variable,
    Partial,
    SectionOpen,
    SectionClose,
    InvertSectionOpen,
    UnescapeVariable,
    Comment,
    समुच्चयः_Delimiter,
  };

  Token(StringRef Str)
      : TokenType(Type::Text), RawBody(Str), TokenBody(RawBody),
        Accessor_मानम्({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, वर्ण Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    यदि (TokenType == Type::Comment)
      प्रत्यागम;
    StringRef AccessorStr(एतत्->TokenBody);
    यदि (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_मानम् = विभज_Mustache_String(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ध्रुव { प्रत्यागम Accessor_मानम्; }

  Type getType() ध्रुव { प्रत्यागम TokenType; }

  शून्य समुच्चयः_Indentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() ध्रुव { प्रत्यागम Indentation; }

  स्थिर Type getTokenType(वर्ण Identifier) {
    विकल्प (Identifier) {
    पक्ष '#':
      प्रत्यागम Type::SectionOpen;
    पक्ष '/':
      प्रत्यागम Type::SectionClose;
    पक्ष '^':
      प्रत्यागम Type::InvertSectionOpen;
    पक्ष '!':
      प्रत्यागम Type::Comment;
    पक्ष '>':
      प्रत्यागम Type::Partial;
    पक्ष '&':
      प्रत्यागम Type::UnescapeVariable;
    पक्ष '=':
      प्रत्यागम Type::समुच्चयः_Delimiter;
    स्वभाव:
      प्रत्यागम Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_मानम्;
  size_t Indentation;
};

उपयुज्य EscapeMap = DenseMap<वर्ण, std::string>;

वर्ग ASTNode : सार्वजनिक ilist_ग्रन्थिः<ASTNode> {
सार्वजनिक:
  गणना Type {
    मूलम्_2,
    Text,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::मूलम्_2), Parent(शून्यसूचकः), ParentContext(शून्यसूचकः) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *Parent)
      : Ctx(Ctx), Ty(Type::Text), Body(Body), Parent(Parent),
        ParentContext(शून्यसूचकः) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *Parent)
      : Ctx(Ctx), Ty(Ty), Parent(Parent), Accessor_मानम्(Accessor),
        ParentContext(शून्यसूचकः) {}

  शून्य सङ्कलय_Child(AstPtr Child) { Children.प्रक्षिप_पृष्ठभागः(Child); };

  शून्य समुच्चयः_Raw_Body(StringRef NewBody) { RawBody = NewBody; };

  शून्य समुच्चयः_Indentation(size_t NewIndentation) { Indentation = NewIndentation; };

  शून्य render(ध्रुव llvm::json::मानम् &दत्त_2, Mustache_Output_प्रवाहः &OS);

गुप्त:
  शून्य renderLambdas(ध्रुव llvm::json::मानम् &Contexts,
                     Mustache_Output_प्रवाहः &OS, Lambda &L);

  शून्य renderSectionLambdas(ध्रुव llvm::json::मानम् &Contexts,
                            Mustache_Output_प्रवाहः &OS, SectionLambda &L);

  शून्य renderPartial(ध्रुव llvm::json::मानम् &Contexts,
                     Mustache_Output_प्रवाहः &OS, ASTNode *Partial);

  शून्य renderChild(ध्रुव llvm::json::मानम् &Context, Mustache_Output_प्रवाहः &OS);

  ध्रुव llvm::json::मानम् *findContext();

  शून्य render_मूलम्(ध्रुव json::मानम् &CurrentCtx, Mustache_Output_प्रवाहः &OS);
  शून्य renderText(Mustache_Output_प्रवाहः &OS);
  शून्य renderPartial(ध्रुव json::मानम् &CurrentCtx, Mustache_Output_प्रवाहः &OS);
  शून्य renderVariable(ध्रुव json::मानम् &CurrentCtx, Mustache_Output_प्रवाहः &OS);
  शून्य renderUnescapeVariable(ध्रुव json::मानम् &CurrentCtx,
                              Mustache_Output_प्रवाहः &OS);
  शून्य renderSection(ध्रुव json::मानम् &CurrentCtx, Mustache_Output_प्रवाहः &OS);
  शून्य renderInvertSection(ध्रुव json::मानम् &CurrentCtx,
                           Mustache_Output_प्रवाहः &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  ध्रुव ArrayRef<StringRef> Accessor_मानम्;
  ध्रुव llvm::json::मानम् *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
स्थिर AstPtr create_मूलम्_ग्रन्थिः(MustacheContext &Ctx) {
  प्रत्यागम नूतन (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

स्थिर AstPtr create_ग्रन्थिः(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  प्रत्यागम नूतन (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

स्थिर AstPtr create_Text_ग्रन्थिः(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  प्रत्यागम नूतन (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
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
स्थिर तार्किक hasTextBehind(size_t Idx, ध्रुव ArrayRef<Token> &Tokens) {
  यदि (Idx == 0)
    प्रत्यागम सत्य;

  size_t पूर्व_Idx = Idx - 1;
  यदि (Tokens[पूर्व_Idx].getType() != Token::Type::Text)
    प्रत्यागम सत्य;

  ध्रुव Token &पूर्व_Token = Tokens[पूर्व_Idx];
  StringRef TokenBody = StringRef(पूर्व_Token.RawBody).rtrim(" \r\t\v");
  प्रत्यागम !TokenBody.अन्ताः_with("\n") && !(TokenBody.रिक्तानि() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
स्थिर तार्किक hasTextAhead(size_t Idx, ध्रुव ArrayRef<Token> &Tokens) {
  यदि (Idx >= Tokens.आकार_2() - 1)
    प्रत्यागम सत्य;

  size_t अग्र_Idx = Idx + 1;
  यदि (Tokens[अग्र_Idx].getType() != Token::Type::Text)
    प्रत्यागम सत्य;

  ध्रुव Token &अग्र_Token = Tokens[अग्र_Idx];
  StringRef TokenBody = StringRef(अग्र_Token.RawBody).ltrim(" ");
  प्रत्यागम !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

स्थिर तार्किक requires_Clean_ऊर्ध्वम्(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  प्रत्यागम T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::समुच्चयः_Delimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
स्थिर शून्य stripTokenAhead(Small_सदिशम्_Impl<Token> &Tokens, size_t Idx) {
  Token &अग्र_Token = Tokens[Idx + 1];
  StringRef अग्र_Token_Body = अग्र_Token.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  यदि (अग्र_Token_Body.starts_with("\r\n"))
    अग्र_Token.TokenBody = अग्र_Token_Body.substr(2);
  अन्यथा यदि (अग्र_Token_Body.starts_with("\n"))
    अग्र_Token.TokenBody = अग्र_Token_Body.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
शून्य stripTokenBefore(Small_सदिशम्_Impl<Token> &Tokens, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &पूर्व_Token = Tokens[Idx - 1];
  StringRef पूर्व_Token_Body = पूर्व_Token.TokenBody;
  StringRef Unindented = पूर्व_Token_Body.rtrim(" \r\t\v");
  size_t Indentation = पूर्व_Token_Body.आकार_2() - Unindented.आकार_2();
  पूर्व_Token.TokenBody = Unindented;
  CurrentToken.समुच्चयः_Indentation(Indentation);
}

संरचना चिह्नकम् {
  गणना वर्ग विधा_2 {
    None,
    प्रसामान्यम्, // {{...}}
    Triple, // {{{...}}}
  };

  विधा_2 चिह्नकम्_विधा = विधा_2::None;
  StringRef Content;   // The content between the delimiters.
  StringRef पूर्णम्_मेलनम्; // The entire tag, including delimiters.
  size_t StartPosition = StringRef::npos;
};

[[maybe_अप्रयुक्तम्]] स्थिर ध्रुव वर्ण *चिह्नकम्_विधा_गन्तव्यम्_String(चिह्नकम्::विधा_2 K) {
  विकल्प (K) {
  पक्ष चिह्नकम्::विधा_2::None:
    प्रत्यागम "None";
  पक्ष चिह्नकम्::विधा_2::प्रसामान्यम्:
    प्रत्यागम "Normal";
  पक्ष चिह्नकम्::विधा_2::Triple:
    प्रत्यागम "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_अप्रयुक्तम्]] स्थिर ध्रुव वर्ण *json_विधा_गन्तव्यम्_String(json::मानम्::विधा_2 K) {
  विकल्प (K) {
  पक्ष json::मानम्::विधा_2::Null:
    प्रत्यागम "JSON_KIND_NULL";
  पक्ष json::मानम्::विधा_2::Boolean:
    प्रत्यागम "JSON_KIND_BOOLEAN";
  पक्ष json::मानम्::विधा_2::सङ्ख्या:
    प्रत्यागम "JSON_KIND_NUMBER";
  पक्ष json::मानम्::विधा_2::String:
    प्रत्यागम "JSON_KIND_STRING";
  पक्ष json::मानम्::विधा_2::Array:
    प्रत्यागम "JSON_KIND_ARRAY";
  पक्ष json::मानम्::विधा_2::Object:
    प्रत्यागम "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
स्थिर Small_सदिशम्<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  Small_सदिशम्<Token> Tokens;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t TextStart = 0;

  ध्रुव StringLiteral TripleOpen("{{{");
  ध्रुव StringLiteral TripleClose("}}}");

  यावत् (Cursor < Template.आकार_2()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef चिह्नकम्_Open, चिह्नकम्_Close;
    चिह्नकम्::विधा_2 विधा_2;

    // Determine which tag we've encountered.
    यदि (TemplateSuffix.starts_with(TripleOpen)) {
      विधा_2 = चिह्नकम्::विधा_2::Triple;
      चिह्नकम्_Open = TripleOpen;
      चिह्नकम्_Close = TripleClose;
    } अन्यथा यदि (TemplateSuffix.starts_with(Open)) {
      विधा_2 = चिह्नकम्::विधा_2::प्रसामान्यम्;
      चिह्नकम्_Open = Open;
      चिह्नकम्_Close = Close;
    } अन्यथा {
      // Not at a tag, continue scanning.
      ++Cursor;
      अनुवर्त;
    }

    // Found a tag, first add the preceding text.
    यदि (Cursor > TextStart)
      Tokens.emplace_पृष्ठभागः(Template.खण्डिका(TextStart, Cursor));

    // Find the closing tag.
    size_t अन्तः_Pos = Template.find(चिह्नकम्_Close, Cursor + चिह्नकम्_Open.आकार_2());
    यदि (अन्तः_Pos == StringRef::npos) {
      // No closing tag, the rest is text.
      Tokens.emplace_पृष्ठभागः(Template.substr(Cursor));
      TextStart = Cursor = Template.आकार_2();
      भङ्ग;
    }

    // Extract tag content and full match.
    size_t ContentStart = Cursor + चिह्नकम्_Open.आकार_2();
    StringRef Content = Template.substr(ContentStart, अन्तः_Pos - ContentStart);
    StringRef पूर्णम्_मेलनम् =
        Template.substr(Cursor, (अन्तः_Pos + चिह्नकम्_Close.आकार_2()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << पूर्णम्_मेलनम् << ", Content: " << Content
                      << ", Kind: " << चिह्नकम्_विधा_गन्तव्यम्_String(विधा_2) << "\n");
    यदि (विधा_2 == चिह्नकम्::विधा_2::Triple) {
      Tokens.emplace_पृष्ठभागः(पूर्णम्_मेलनम्, Ctx.Saver.save("&" + Content), '&', Ctx);
    } अन्यथा { // Normal Tag
      StringRef Interpolated = Content;
      यदि (!Interpolated.trim().starts_with("=")) {
        वर्ण Front = Interpolated.रिक्तानि() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_पृष्ठभागः(पूर्णम्_मेलनम्, Interpolated, Front, Ctx);
      } अन्यथा { // Set Delimiter
        Tokens.emplace_पृष्ठभागः(पूर्णम्_मेलनम्, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](वर्ण C) { प्रत्यागम C == '='; });
        DelimSpec = DelimSpec.trim();

        स्वचालित [NewOpen, NewClose] = DelimSpec.विभज(' ');
        LLVM_DEBUG(dbgs() << "[Set Delimiter] NewOpen: " << NewOpen
                          << ", NewClose: " << NewClose << "\n");
        Open = NewOpen;
        Close = NewClose;
      }
    }

    // Move past the tag for the next iteration.
    Cursor += पूर्णम्_मेलनम्.आकार_2();
    TextStart = Cursor;
  }

  // Add any remaining text after the last tag.
  यदि (TextStart < Template.आकार_2())
    Tokens.emplace_पृष्ठभागः(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  size_t चरमः_Idx = Tokens.आकार_2() - 1;
  प्रति (size_t Idx = 0, अन्तः_2 = Tokens.आकार_2(); Idx < अन्तः_2; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    यदि (!requires_Clean_ऊर्ध्वम्(CurrentType))
      अनुवर्त;

    तार्किक HasTextBehind = hasTextBehind(Idx, Tokens);
    तार्किक HasTextAhead = hasTextAhead(Idx, Tokens);

    यदि ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    यदि ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == चरमः_Idx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  प्रत्यागम Tokens;
}

// Custom stream to escape strings.
वर्ग Escape_String_प्रवाहः : सार्वजनिक Mustache_Output_प्रवाहः {
सार्वजनिक:
  स्पष्ट Escape_String_प्रवाहः(llvm::raw_ostream &Wrapped_प्रवाहः,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.कुञ्चिकाः().begin(), Escape.कुञ्चिकाः().अन्तः()),
        Wrapped_प्रवाहः(Wrapped_प्रवाहः) {
    समुच्चयः_Unbuffered();
  }

रक्षित:
  शून्य write_impl(ध्रुव वर्ण *Ptr, size_t आकार) override {
    StringRef दत्त_2(Ptr, आकार);
    size_t Start = 0;
    यावत् (Start < आकार) {
      // Find the next character that needs to be escaped.
      size_t अग्र_2 = दत्त_2.find_first_सम्बन्धि(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      यदि (अग्र_2 == StringRef::npos) {
        Wrapped_प्रवाहः << दत्त_2.substr(Start);
        प्रत्यागम;
      }

      // Write the chunk of text before the escapable character.
      यदि (अग्र_2 > Start)
        Wrapped_प्रवाहः << दत्त_2.substr(Start, अग्र_2 - Start);

      // Look up and write the escaped version of the character.
      Wrapped_प्रवाहः << Escape[दत्त_2[अग्र_2]];
      Start = अग्र_2 + 1;
    }
  }

  uint64_t current_pos() ध्रुव override { प्रत्यागम Wrapped_प्रवाहः.tell(); }

गुप्त:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &Wrapped_प्रवाहः;
};

// Custom stream to add indentation used to for rendering partials.
वर्ग सङ्कलय_Indentation_String_प्रवाहः : सार्वजनिक Mustache_Output_प्रवाहः {
सार्वजनिक:
  स्पष्ट सङ्कलय_Indentation_String_प्रवाहः(raw_ostream &Wrapped_प्रवाहः,
                                      size_t Indentation)
      : Indentation(Indentation), Wrapped_प्रवाहः(Wrapped_प्रवाहः),
        NeedsIndent(सत्य), IsSuspended(असत्य) {
    समुच्चयः_Unbuffered();
  }

  शून्य suspendIndentation() override { IsSuspended = सत्य; }
  शून्य resumeIndentation() override { IsSuspended = असत्य; }

रक्षित:
  शून्य write_impl(ध्रुव वर्ण *Ptr, size_t आकार) override {
    llvm::StringRef दत्त_2(Ptr, आकार);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    प्रति (वर्ण C : दत्त_2) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      यदि (NeedsIndent && C != '\n') {
        Wrapped_प्रवाहः << Indent;
        NeedsIndent = असत्य;
      }
      Wrapped_प्रवाहः << C;
      यदि (C == '\n' && !IsSuspended)
        NeedsIndent = सत्य;
    }
  }

  uint64_t current_pos() ध्रुव override { प्रत्यागम Wrapped_प्रवाहः.tell(); }

गुप्त:
  size_t Indentation;
  raw_ostream &Wrapped_प्रवाहः;
  तार्किक NeedsIndent;
  तार्किक IsSuspended;
};

वर्ग Parser {
सार्वजनिक:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

गुप्त:
  शून्य parseMustache(ASTNode *Parent);
  शून्य parseSection(ASTNode *Parent, ASTNode::Type Ty, ध्रुव Accessor &A);

  MustacheContext &Ctx;
  Small_सदिशम्<Token> Tokens;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

शून्य Parser::parseSection(ASTNode *Parent, ASTNode::Type Ty,
                          ध्रुव Accessor &A) {
  AstPtr Current_ग्रन्थिः = create_ग्रन्थिः(Ctx, Ty, A, Parent);
  size_t Start = CurrentPtr;
  parseMustache(Current_ग्रन्थिः);
  ध्रुव size_t अन्तः_2 = CurrentPtr - 1;

  size_t Raw_Body_आकार = 0;
  प्रति (size_t I = Start; I < अन्तः_2; ++I)
    Raw_Body_आकार += Tokens[I].RawBody.आकार_2();

  SmallString<128> RawBody;
  RawBody.reserve(Raw_Body_आकार);
  प्रति (std::size_t I = Start; I < अन्तः_2; ++I)
    RawBody += Tokens[I].RawBody;

  Current_ग्रन्थिः->समुच्चयः_Raw_Body(Ctx.Saver.save(StringRef(RawBody)));
  Parent->सङ्कलय_Child(Current_ग्रन्थिः);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr मूलम्_ग्रन्थिः = create_मूलम्_ग्रन्थिः(Ctx);
  parseMustache(मूलम्_ग्रन्थिः);
  प्रत्यागम मूलम्_ग्रन्थिः;
}

शून्य Parser::parseMustache(ASTNode *Parent) {

  यावत् (CurrentPtr < Tokens.आकार_2()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_ग्रन्थिः;

    विकल्प (CurrentToken.getType()) {
    पक्ष Token::Type::Text: {
      Current_ग्रन्थिः = create_Text_ग्रन्थिः(Ctx, CurrentToken.TokenBody, Parent);
      Parent->सङ्कलय_Child(Current_ग्रन्थिः);
      भङ्ग;
    }
    पक्ष Token::Type::Variable: {
      Current_ग्रन्थिः = create_ग्रन्थिः(Ctx, ASTNode::Variable, A, Parent);
      Parent->सङ्कलय_Child(Current_ग्रन्थिः);
      भङ्ग;
    }
    पक्ष Token::Type::UnescapeVariable: {
      Current_ग्रन्थिः = create_ग्रन्थिः(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->सङ्कलय_Child(Current_ग्रन्थिः);
      भङ्ग;
    }
    पक्ष Token::Type::Partial: {
      Current_ग्रन्थिः = create_ग्रन्थिः(Ctx, ASTNode::Partial, A, Parent);
      Current_ग्रन्थिः->समुच्चयः_Indentation(CurrentToken.getIndentation());
      Parent->सङ्कलय_Child(Current_ग्रन्थिः);
      भङ्ग;
    }
    पक्ष Token::Type::SectionOpen: {
      parseSection(Parent, ASTNode::Section, A);
      भङ्ग;
    }
    पक्ष Token::Type::InvertSectionOpen: {
      parseSection(Parent, ASTNode::InvertSection, A);
      भङ्ग;
    }
    पक्ष Token::Type::Comment:
    पक्ष Token::Type::समुच्चयः_Delimiter:
      भङ्ग;
    पक्ष Token::Type::SectionClose:
      प्रत्यागम;
    }
  }
}
स्थिर शून्य गन्तव्यम्_Mustache_String(ध्रुव json::मानम् &दत्त_2, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << json_विधा_गन्तव्यम्_String(दत्त_2.विधा()) << ", Data: " << दत्त_2
                    << "\n");
  विकल्प (दत्त_2.विधा()) {
  पक्ष json::मानम्::Null:
    प्रत्यागम;
  पक्ष json::मानम्::सङ्ख्या: {
    स्वचालित अङ्कः = *दत्त_2.get_As_सङ्ख्या();
    std::ostringstream SS;
    SS << अङ्कः;
    OS << SS.str();
    प्रत्यागम;
  }
  पक्ष json::मानम्::String: {
    OS << *दत्त_2.getAsString();
    प्रत्यागम;
  }

  पक्ष json::मानम्::Array: {
    स्वचालित Arr = *दत्त_2.getAsArray();
    यदि (Arr.रिक्तानि())
      प्रत्यागम;
    [[fallthrough]];
  }
  पक्ष json::मानम्::Object:
  पक्ष json::मानम्::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.मानम्_2(दत्त_2);
    भङ्ग;
  }
  }
}

शून्य ASTNode::render_मूलम्(ध्रुव json::मानम् &CurrentCtx,
                         Mustache_Output_प्रवाहः &OS) {
  renderChild(CurrentCtx, OS);
}

शून्य ASTNode::renderText(Mustache_Output_प्रवाहः &OS) { OS << Body; }

शून्य ASTNode::renderPartial(ध्रुव json::मानम् &CurrentCtx,
                            Mustache_Output_प्रवाहः &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_मानम्[0]
                    << ", Indentation:" << Indentation << "\n");
  स्वचालित Partial = Ctx.Partials.find(Accessor_मानम्[0]);
  यदि (Partial != Ctx.Partials.अन्तः())
    renderPartial(CurrentCtx, OS, Partial->get_मानम्());
}

शून्य ASTNode::renderVariable(ध्रुव json::मानम् &CurrentCtx,
                             Mustache_Output_प्रवाहः &OS) {
  स्वचालित Lambda = Ctx.Lambdas.find(Accessor_मानम्[0]);
  यदि (Lambda != Ctx.Lambdas.अन्तः()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_मानम्());
  } अन्यथा यदि (ध्रुव json::मानम् *ContextPtr = findContext()) {
    Escape_String_प्रवाहः ES(OS, Ctx.Escapes);
    गन्तव्यम्_Mustache_String(*ContextPtr, ES);
  }
}

शून्य ASTNode::renderUnescapeVariable(ध्रुव json::मानम् &CurrentCtx,
                                     Mustache_Output_प्रवाहः &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_मानम्[0]
                    << "\n");
  स्वचालित Lambda = Ctx.Lambdas.find(Accessor_मानम्[0]);
  यदि (Lambda != Ctx.Lambdas.अन्तः()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_मानम्());
  } अन्यथा यदि (ध्रुव json::मानम् *ContextPtr = findContext()) {
    OS.suspendIndentation();
    गन्तव्यम्_Mustache_String(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

शून्य ASTNode::renderSection(ध्रुव json::मानम् &CurrentCtx,
                            Mustache_Output_प्रवाहः &OS) {
  स्वचालित SectionLambda = Ctx.SectionLambdas.find(Accessor_मानम्[0]);
  यदि (SectionLambda != Ctx.SectionLambdas.अन्तः()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_मानम्());
    प्रत्यागम;
  }

  ध्रुव json::मानम् *ContextPtr = findContext();
  यदि (isContextFalsey(ContextPtr))
    प्रत्यागम;

  यदि (ध्रुव json::Array *Arr = ContextPtr->getAsArray()) {
    प्रति (ध्रुव json::मानम् &V : *Arr)
      renderChild(V, OS);
    प्रत्यागम;
  }
  renderChild(*ContextPtr, OS);
}

शून्य ASTNode::renderInvertSection(ध्रुव json::मानम् &CurrentCtx,
                                  Mustache_Output_प्रवाहः &OS) {
  तार्किक IsLambda = Ctx.SectionLambdas.अन्तर्भवति(Accessor_मानम्[0]);
  ध्रुव json::मानम् *ContextPtr = findContext();
  यदि (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

शून्य ASTNode::render(ध्रुव llvm::json::मानम् &दत्त_2, Mustache_Output_प्रवाहः &OS) {
  यदि (Ty != मूलम्_2 && Ty != Text && Accessor_मानम्.रिक्तानि())
    प्रत्यागम;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &दत्त_2;

  विकल्प (Ty) {
  पक्ष मूलम्_2:
    render_मूलम्(दत्त_2, OS);
    प्रत्यागम;
  पक्ष Text:
    renderText(OS);
    प्रत्यागम;
  पक्ष Partial:
    renderPartial(दत्त_2, OS);
    प्रत्यागम;
  पक्ष Variable:
    renderVariable(दत्त_2, OS);
    प्रत्यागम;
  पक्ष UnescapeVariable:
    renderUnescapeVariable(दत्त_2, OS);
    प्रत्यागम;
  पक्ष Section:
    renderSection(दत्त_2, OS);
    प्रत्यागम;
  पक्ष InvertSection:
    renderInvertSection(दत्त_2, OS);
    प्रत्यागम;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ध्रुव json::मानम् *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  यदि (Accessor_मानम्.रिक्तानि())
    प्रत्यागम शून्यसूचकः;
  यदि (Accessor_मानम्[0] == ".")
    प्रत्यागम ParentContext;

  ध्रुव json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = Accessor_मानम्[0];
  ASTNode *CurrentParent = Parent;

  यावत् (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    यदि (CurrentParent->Ty != मूलम्_2) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      अनुवर्त;
    }
    प्रत्यागम शून्यसूचकः;
  }
  ध्रुव json::मानम् *Context = शून्यसूचकः;
  प्रति (स्वचालित [Idx, Acc] : enumerate(Accessor_मानम्)) {
    ध्रुव json::मानम् *Current_मानम् = CurrentContext->get(Acc);
    यदि (!Current_मानम्)
      प्रत्यागम शून्यसूचकः;
    यदि (Idx < Accessor_मानम्.आकार_2() - 1) {
      CurrentContext = Current_मानम्->getAsObject();
      यदि (!CurrentContext)
        प्रत्यागम शून्यसूचकः;
    } अन्यथा {
      Context = Current_मानम्;
    }
  }
  प्रत्यागम Context;
}

शून्य ASTNode::renderChild(ध्रुव json::मानम् &Contexts,
                          Mustache_Output_प्रवाहः &OS) {
  प्रति (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

शून्य ASTNode::renderPartial(ध्रुव json::मानम् &Contexts,
                            Mustache_Output_प्रवाहः &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  सङ्कलय_Indentation_String_प्रवाहः IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

शून्य ASTNode::renderLambdas(ध्रुव llvm::json::मानम् &Contexts,
                            Mustache_Output_प्रवाहः &OS, Lambda &L) {
  json::मानम् Lambda_परिणाम = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  गन्तव्यम्_Mustache_String(Lambda_परिणाम, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ग्रन्थिः = P.parse();

  Escape_String_प्रवाहः ES(OS, Ctx.Escapes);
  यदि (Ty == Variable) {
    Lambda_ग्रन्थिः->render(Contexts, ES);
    प्रत्यागम;
  }
  Lambda_ग्रन्थिः->render(Contexts, OS);
}

शून्य ASTNode::renderSectionLambdas(ध्रुव llvm::json::मानम् &Contexts,
                                   Mustache_Output_प्रवाहः &OS, SectionLambda &L) {
  json::मानम् Return = L(RawBody.str());
  यदि (isFalsey(Return))
    प्रत्यागम;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  गन्तव्यम्_Mustache_String(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ग्रन्थिः = P.parse();
  Lambda_ग्रन्थिः->render(Contexts, OS);
}

शून्य Template::render(ध्रुव llvm::json::मानम् &दत्त_2, llvm::raw_ostream &OS) {
  Raw_Mustache_Output_प्रवाहः MOS(OS);
  वृक्षः->render(दत्त_2, MOS);
}

शून्य Template::पञ्जिकाकोशः_Partial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_वृक्षः = P.parse();
  Ctx.Partials.निवेशय(std::make_pair(Name, Partial_वृक्षः));
}

शून्य Template::पञ्जिकाकोशः_Lambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

शून्य Template::पञ्जिकाकोशः_Lambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

शून्य Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  वृक्षः = P.parse();
  // The default behavior is to escape html entities.
  ध्रुव EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), वृक्षः(Other.वृक्षः) {
  Other.वृक्षः = शून्यसूचकः;
}

Template::~Template() = स्वभाव;

} // namespace llvm::mustache

#अपरिभाषय DEBUG_TYPE

/*===--- ConvertUTF.c - Universal Character Names conversions ---------------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===------------------------------------------------------------------------=*/
/*
 * Copyright © 1991-2015 Unicode, Inc. All rights reserved.
 * Distributed under the Terms of Use in
 * http://www.unicode.org/copyright.html.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of the Unicode data files and any associated documentation
 * (the "Data Files") or Unicode software and any associated documentation
 * (the "Software") to deal in the Data Files or Software
 * without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, and/or sell copies of
 * the Data Files or Software, and to permit persons to whom the Data Files
 * or Software are furnished to do so, provided that
 * (a) this copyright and permission notice appear with all copies
 * of the Data Files or Software,
 * (b) this copyright and permission notice appear in associated
 * documentation, and
 * (c) there is clear notice in each modified Data File or in the Software
 * as well as in the documentation associated with the Data File(s) or
 * Software that the data or software has been modified.
 *
 * THE DATA FILES AND SOFTWARE ARE PROVIDED "AS IS", WITHOUT WARRANTY OF
 * ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS
 * NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL
 * DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THE DATA FILES OR SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder
 * shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in these Data Files or Software without prior
 * written authorization of the copyright holder.
 */

/* ---------------------------------------------------------------------

    Conversions between UTF32, UTF-16, and UTF-8. Source code file.
    Author: Mark E. Davis, 1994.
    Rev History: Rick McGowan, fixes & updates May 2001.
    Sept 2001: fixed const & error conditions per
        mods suggested by S. Parent & A. Lillich.
    June 2002: Tim Dodd added detection and handling of incomplete
        source sequences, enhanced error detection, added casts
        to eliminate compiler warnings.
    July 2003: slight mods to back out aggressive FFFE detection.
    Jan 2004: updated switches in from-UTF8 conversions.
    Oct 2004: updated to use UNI_MAX_LEGAL_UTF32 in UTF-32 conversions.

    See the header file "ConvertUTF.h" for complete documentation.

------------------------------------------------------------------------ */

#समावेश "llvm/Support/ConvertUTF.h"
#अगर_घोषित CVTUTF_DEBUG
#समावेश <stdio.h>
#पूर्ण_अगर
#समावेश <assert.h>

/*
 * This code extensively uses fall-through switches.
 * Keep the compiler from warning about that.
 */
#अगर defined(__clang__) && defined(__has_warning)
# अगर __has_warning("-Wimplicit-fallthrough")
#  define ConvertUTF_DISABLE_WARNINGS \
    _Pragma("clang diagnostic push")  \
    _Pragma("clang diagnostic ignored \"-Wimplicit-fallthrough\"")
#  define ConvertUTF_RESTORE_WARNINGS \
    _Pragma("clang diagnostic pop")
# endif
#या_अगर defined(__GNUC__)
# define ConvertUTF_DISABLE_WARNINGS \
   _Pragma("GCC diagnostic push")    \
   _Pragma("GCC diagnostic ignored \"-Wimplicit-fallthrough\"")
# define ConvertUTF_RESTORE_WARNINGS \
   _Pragma("GCC diagnostic pop")
#पूर्ण_अगर
#अगर_अघोषित ConvertUTF_DISABLE_WARNINGS
# define ConvertUTF_DISABLE_WARNINGS
#पूर्ण_अगर
#अगर_अघोषित ConvertUTF_RESTORE_WARNINGS
# define ConvertUTF_RESTORE_WARNINGS
#पूर्ण_अगर

ConvertUTF_DISABLE_WARNINGS

namespace llvm {

अटल स्थिर पूर्णांक halfShift  = 10; /* used for shifting by 10 bits */

अटल स्थिर UTF32 halfBase = 0x0010000UL;
अटल स्थिर UTF32 halfMask = 0x3FFUL;

#घोषणा UNI_SUR_ऊपरी_START  (UTF32)0xD800
#घोषणा UNI_SUR_ऊपरी_END    (UTF32)0xDBFF
#घोषणा UNI_SUR_निचला_START   (UTF32)0xDC00
#घोषणा UNI_SUR_निचला_END     (UTF32)0xDFFF

/* --------------------------------------------------------------------- */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
अटल स्थिर अक्षर trailingBytesForUTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/*
 * Magic values subtracted from a buffer value during UTF8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence.
 */
अटल स्थिर UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
                     0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
अटल स्थिर UTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */


/* --------------------------------------------------------------------- */

Conversion_परिणाम ConvertUTF32toUTF16 (
        स्थिर UTF32** sourceStart, स्थिर UTF32* sourceEnd,
        UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags) {
    Conversion_परिणाम परिणाम_2 = conversionOK;
    स्थिर UTF32* source = *sourceStart;
    UTF16* target = *targetStart;
    जबतक (source < sourceEnd) {
        UTF32 ch;
        अगर (target >= targetEnd) {
            परिणाम_2 = targetExhausted; अवरोध;
        }
        ch = *source++;
        अगर (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
            अगर (ch >= UNI_SUR_ऊपरी_START && ch <= UNI_SUR_निचला_END) {
                अगर (flags == strictConversion) {
                    --source; /* return to the illegal value itself */
                    परिणाम_2 = sourceIllegal;
                    अवरोध;
                } अन्यथा {
                    *target++ = UNI_REPLACEMENT_CHAR;
                }
            } अन्यथा {
                *target++ = (UTF16)ch; /* normal case */
            }
        } अन्यथा अगर (ch > UNI_MAX_LEGAL_UTF32) {
            अगर (flags == strictConversion) {
                परिणाम_2 = sourceIllegal;
            } अन्यथा {
                *target++ = UNI_REPLACEMENT_CHAR;
            }
        } अन्यथा {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            अगर (target + 1 >= targetEnd) {
                --source; /* Back up source pointer! */
                परिणाम_2 = targetExhausted; अवरोध;
            }
            ch -= halfBase;
            *target++ = (UTF16)((ch >> halfShift) + UNI_SUR_ऊपरी_START);
            *target++ = (UTF16)((ch & halfMask) + UNI_SUR_निचला_START);
        }
    }
    *sourceStart = source;
    *targetStart = target;
    वापस परिणाम_2;
}

/* --------------------------------------------------------------------- */

Conversion_परिणाम ConvertUTF16toUTF32 (
        स्थिर UTF16** sourceStart, स्थिर UTF16* sourceEnd,
        UTF32** targetStart, UTF32* targetEnd, ConversionFlags flags) {
    Conversion_परिणाम परिणाम_2 = conversionOK;
    स्थिर UTF16* source = *sourceStart;
    UTF32* target = *targetStart;
    UTF32 ch, ch2;
    जबतक (source < sourceEnd) {
        स्थिर UTF16* oldSource = source; /*  In case we have to back up because of target overflow. */
        ch = *source++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        अगर (ch >= UNI_SUR_ऊपरी_START && ch <= UNI_SUR_ऊपरी_END) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            अगर (source < sourceEnd) {
                ch2 = *source;
                /* If it's a low surrogate, convert to UTF32. */
                अगर (ch2 >= UNI_SUR_निचला_START && ch2 <= UNI_SUR_निचला_END) {
                    ch = ((ch - UNI_SUR_ऊपरी_START) << halfShift)
                        + (ch2 - UNI_SUR_निचला_START) + halfBase;
                    ++source;
                } अन्यथा अगर (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --source; /* return to the illegal value itself */
                    परिणाम_2 = sourceIllegal;
                    अवरोध;
                }
            } अन्यथा { /* We don't have the 16 bits following the high surrogate. */
                --source; /* return to the high surrogate */
                परिणाम_2 = sourceExhausted;
                अवरोध;
            }
        } अन्यथा अगर (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            अगर (ch >= UNI_SUR_निचला_START && ch <= UNI_SUR_निचला_END) {
                --source; /* return to the illegal value itself */
                परिणाम_2 = sourceIllegal;
                अवरोध;
            }
        }
        अगर (target >= targetEnd) {
            source = oldSource; /* Back up source pointer! */
            परिणाम_2 = targetExhausted; अवरोध;
        }
        *target++ = ch;
    }
    *sourceStart = source;
    *targetStart = target;
#अगर_घोषित CVTUTF_DEBUG
अगर (परिणाम_2 == sourceIllegal) {
    ख_लिखो(मानक_त्रुटि, "ConvertUTF16toUTF32 illegal seq 0x%04x,%04x\n", ch, ch2);
    ख_साफ(मानक_त्रुटि);
}
#पूर्ण_अगर
    वापस परिणाम_2;
}
Conversion_परिणाम ConvertUTF16toUTF8 (
        स्थिर UTF16** sourceStart, स्थिर UTF16* sourceEnd,
        UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags) {
    Conversion_परिणाम परिणाम_2 = conversionOK;
    स्थिर UTF16* source = *sourceStart;
    UTF8* target = *targetStart;
    जबतक (source < sourceEnd) {
        UTF32 ch;
        अचिन्हित लघु bytesToWrite = 0;
        स्थिर UTF32 byteMask = 0xBF;
        स्थिर UTF32 byteMark = 0x80;
        स्थिर UTF16* oldSource = source; /* In case we have to back up because of target overflow. */
        ch = *source++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        अगर (ch >= UNI_SUR_ऊपरी_START && ch <= UNI_SUR_ऊपरी_END) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            अगर (source < sourceEnd) {
                UTF32 ch2 = *source;
                /* If it's a low surrogate, convert to UTF32. */
                अगर (ch2 >= UNI_SUR_निचला_START && ch2 <= UNI_SUR_निचला_END) {
                    ch = ((ch - UNI_SUR_ऊपरी_START) << halfShift)
                        + (ch2 - UNI_SUR_निचला_START) + halfBase;
                    ++source;
                } अन्यथा अगर (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --source; /* return to the illegal value itself */
                    परिणाम_2 = sourceIllegal;
                    अवरोध;
                }
            } अन्यथा { /* We don't have the 16 bits following the high surrogate. */
                --source; /* return to the high surrogate */
                परिणाम_2 = sourceExhausted;
                अवरोध;
            }
        } अन्यथा अगर (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            अगर (ch >= UNI_SUR_निचला_START && ch <= UNI_SUR_निचला_END) {
                --source; /* return to the illegal value itself */
                परिणाम_2 = sourceIllegal;
                अवरोध;
            }
        }
        /* Figure out how many bytes the result will require */
        अगर (ch < (UTF32)0x80) {      bytesToWrite = 1;
        } अन्यथा अगर (ch < (UTF32)0x800) {     bytesToWrite = 2;
        } अन्यथा अगर (ch < (UTF32)0x10000) {   bytesToWrite = 3;
        } अन्यथा अगर (ch < (UTF32)0x110000) {  bytesToWrite = 4;
        } अन्यथा {                            bytesToWrite = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
        }

        target += bytesToWrite;
        अगर (target > targetEnd) {
            source = oldSource; /* Back up source pointer! */
            target -= bytesToWrite; परिणाम_2 = targetExhausted; अवरोध;
        }
        चयन (bytesToWrite) { /* note: everything falls through. */
            हाल 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            हाल 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            हाल 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            हाल 1: *--target =  (UTF8)(ch | firstByteMark[bytesToWrite]);
        }
        target += bytesToWrite;
    }
    *sourceStart = source;
    *targetStart = target;
    वापस परिणाम_2;
}

/* --------------------------------------------------------------------- */

Conversion_परिणाम ConvertUTF32toUTF8 (
        स्थिर UTF32** sourceStart, स्थिर UTF32* sourceEnd,
        UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags) {
    Conversion_परिणाम परिणाम_2 = conversionOK;
    स्थिर UTF32* source = *sourceStart;
    UTF8* target = *targetStart;
    जबतक (source < sourceEnd) {
        UTF32 ch;
        अचिन्हित लघु bytesToWrite = 0;
        स्थिर UTF32 byteMask = 0xBF;
        स्थिर UTF32 byteMark = 0x80;
        ch = *source++;
        अगर (flags == strictConversion ) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            अगर (ch >= UNI_SUR_ऊपरी_START && ch <= UNI_SUR_निचला_END) {
                --source; /* return to the illegal value itself */
                परिणाम_2 = sourceIllegal;
                अवरोध;
            }
        }
        /*
         * Figure out how many bytes the result will require. Turn any
         * illegally large UTF32 things (> Plane 17) into replacement chars.
         */
        अगर (ch < (UTF32)0x80) {      bytesToWrite = 1;
        } अन्यथा अगर (ch < (UTF32)0x800) {     bytesToWrite = 2;
        } अन्यथा अगर (ch < (UTF32)0x10000) {   bytesToWrite = 3;
        } अन्यथा अगर (ch <= UNI_MAX_LEGAL_UTF32) {  bytesToWrite = 4;
        } अन्यथा {                            bytesToWrite = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
                                            परिणाम_2 = sourceIllegal;
        }

        target += bytesToWrite;
        अगर (target > targetEnd) {
            --source; /* Back up source pointer! */
            target -= bytesToWrite; परिणाम_2 = targetExhausted; अवरोध;
        }
        चयन (bytesToWrite) { /* note: everything falls through. */
            हाल 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            हाल 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            हाल 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            हाल 1: *--target = (UTF8) (ch | firstByteMark[bytesToWrite]);
        }
        target += bytesToWrite;
    }
    *sourceStart = source;
    *targetStart = target;
    वापस परिणाम_2;
}

/* --------------------------------------------------------------------- */

/*
 * Utility routine to tell whether a sequence of bytes is legal UTF-8.
 * This must be called with the length pre-determined by the first byte.
 * If not calling this from ConvertUTF8to*, then the length can be set by:
 *  length = trailingBytesForUTF8[*source]+1;
 * and the sequence is illegal right away if there aren't that many bytes
 * available.
 * If presented with a length > 4, this returns false.  The Unicode
 * definition of UTF-8 goes up to 4-byte sequences.
 */

अटल Boolean isLegalUTF8(स्थिर UTF8 *source, पूर्णांक लंबाई_2) {
    UTF8 a;
    स्थिर UTF8 *srcptr = source+लंबाई_2;
    चयन (लंबाई_2) {
    शेष: वापस false;
        /* Everything else falls through when "true"... */
    हाल 4: अगर ((a = (*--srcptr)) < 0x80 || a > 0xBF) वापस false;
    हाल 3: अगर ((a = (*--srcptr)) < 0x80 || a > 0xBF) वापस false;
    हाल 2: अगर ((a = (*--srcptr)) < 0x80 || a > 0xBF) वापस false;

        चयन (*source) {
            /* no fall-through in this inner switch */
            हाल 0xE0: अगर (a < 0xA0) वापस false; अवरोध;
            हाल 0xED: अगर (a > 0x9F) वापस false; अवरोध;
            हाल 0xF0: अगर (a < 0x90) वापस false; अवरोध;
            हाल 0xF4: अगर (a > 0x8F) वापस false; अवरोध;
            शेष:   अगर (a < 0x80) वापस false;
        }

    हाल 1: अगर (*source >= 0x80 && *source < 0xC2) वापस false;
    }
    अगर (*source > 0xF4) वापस false;
    वापस true;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 sequence is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8Sequence(स्थिर UTF8 *source, स्थिर UTF8 *sourceEnd) {
    पूर्णांक लंबाई_2 = trailingBytesForUTF8[*source]+1;
    अगर (लंबाई_2 > sourceEnd - source) {
        वापस false;
    }
    वापस isLegalUTF8(source, लंबाई_2);
}

/*
 * Exported function to return the size of the first utf-8 code unit sequence,
 * Or 0 if the sequence is not valid;
 */
अचिन्हित get_UTF8_Sequence_आकार(स्थिर UTF8 *source, स्थिर UTF8 *sourceEnd) {
  पूर्णांक लंबाई_2 = trailingBytesForUTF8[*source] + 1;
  वापस (लंबाई_2 <= sourceEnd - source && isLegalUTF8(source, लंबाई_2)) ? लंबाई_2
                                                                       : 0;
}

/* --------------------------------------------------------------------- */

अचिन्हित findMaximalSubpartOfIllFormedUTF8Sequence(स्थिर UTF8 *source,
                                                   स्थिर UTF8 *sourceEnd) {
  UTF8 b1, b2, b3;

  निश्चित(!isLegalUTF8Sequence(source, sourceEnd));

  /*
   * Unicode 6.3.0, D93b:
   *
   *   Maximal subpart of an ill-formed subsequence: The longest code unit
   *   subsequence starting at an unconvertible offset that is either:
   *   a. the initial subsequence of a well-formed code unit sequence, or
   *   b. a subsequence of length one.
   */

  अगर (source == sourceEnd)
    वापस 0;

  /*
   * Perform case analysis.  See Unicode 6.3.0, Table 3-7. Well-Formed UTF-8
   * Byte Sequences.
   */

  b1 = *source;
  ++source;
  अगर (b1 >= 0xC2 && b1 <= 0xDF) {
    /*
     * First byte is valid, but we know that this code unit sequence is
     * invalid, so the maximal subpart has to end after the first byte.
     */
    वापस 1;
  }

  अगर (source == sourceEnd)
    वापस 1;

  b2 = *source;
  ++source;

  अगर (b1 == 0xE0) {
    वापस (b2 >= 0xA0 && b2 <= 0xBF) ? 2 : 1;
  }
  अगर (b1 >= 0xE1 && b1 <= 0xEC) {
    वापस (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  अगर (b1 == 0xED) {
    वापस (b2 >= 0x80 && b2 <= 0x9F) ? 2 : 1;
  }
  अगर (b1 >= 0xEE && b1 <= 0xEF) {
    वापस (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  अगर (b1 == 0xF0) {
    अगर (b2 >= 0x90 && b2 <= 0xBF) {
      अगर (source == sourceEnd)
        वापस 2;

      b3 = *source;
      वापस (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    वापस 1;
  }
  अगर (b1 >= 0xF1 && b1 <= 0xF3) {
    अगर (b2 >= 0x80 && b2 <= 0xBF) {
      अगर (source == sourceEnd)
        वापस 2;

      b3 = *source;
      वापस (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    वापस 1;
  }
  अगर (b1 == 0xF4) {
    अगर (b2 >= 0x80 && b2 <= 0x8F) {
      अगर (source == sourceEnd)
        वापस 2;

      b3 = *source;
      वापस (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    वापस 1;
  }

  निश्चित((b1 >= 0x80 && b1 <= 0xC1) || b1 >= 0xF5);
  /*
   * There are no valid sequences that start with these bytes.  Maximal subpart
   * is defined to have length 1 in these cases.
   */
  वापस 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return the total number of bytes in a codepoint
 * represented in UTF-8, given the value of the first byte.
 */
अचिन्हित get_अंक_Bytes_For_UTF8(UTF8 first) {
  वापस trailingBytesForUTF8[first] + 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 string is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8String(स्थिर UTF8 **source, स्थिर UTF8 *sourceEnd) {
    जबतक (*source != sourceEnd) {
        पूर्णांक लंबाई_2 = trailingBytesForUTF8[**source] + 1;
        अगर (लंबाई_2 > sourceEnd - *source || !isLegalUTF8(*source, लंबाई_2))
            वापस false;
        *source += लंबाई_2;
    }
    वापस true;
}

/* --------------------------------------------------------------------- */

Conversion_परिणाम ConvertUTF8toUTF16 (
        स्थिर UTF8** sourceStart, स्थिर UTF8* sourceEnd,
        UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags) {
    Conversion_परिणाम परिणाम_2 = conversionOK;
    स्थिर UTF8* source = *sourceStart;
    UTF16* target = *targetStart;
    जबतक (source < sourceEnd) {
        UTF32 ch = 0;
        अचिन्हित लघु extraBytesToRead = trailingBytesForUTF8[*source];
        अगर (extraBytesToRead >= sourceEnd - source) {
            परिणाम_2 = sourceExhausted; अवरोध;
        }
        /* Do this check whether lenient or strict */
        अगर (!isLegalUTF8(source, extraBytesToRead+1)) {
            परिणाम_2 = sourceIllegal;
            अवरोध;
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        चयन (extraBytesToRead) {
            हाल 5: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
            हाल 4: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
            हाल 3: ch += *source++; ch <<= 6;
            हाल 2: ch += *source++; ch <<= 6;
            हाल 1: ch += *source++; ch <<= 6;
            हाल 0: ch += *source++;
        }
        ch -= offsetsFromUTF8[extraBytesToRead];

        अगर (target >= targetEnd) {
            source -= (extraBytesToRead+1); /* Back up source pointer! */
            परिणाम_2 = targetExhausted; अवरोध;
        }
        अगर (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32 */
            अगर (ch >= UNI_SUR_ऊपरी_START && ch <= UNI_SUR_निचला_END) {
                अगर (flags == strictConversion) {
                    source -= (extraBytesToRead+1); /* return to the illegal value itself */
                    परिणाम_2 = sourceIllegal;
                    अवरोध;
                } अन्यथा {
                    *target++ = UNI_REPLACEMENT_CHAR;
                }
            } अन्यथा {
                *target++ = (UTF16)ch; /* normal case */
            }
        } अन्यथा अगर (ch > UNI_MAX_UTF16) {
            अगर (flags == strictConversion) {
                परिणाम_2 = sourceIllegal;
                source -= (extraBytesToRead+1); /* return to the start */
                अवरोध; /* Bail out; shouldn't continue */
            } अन्यथा {
                *target++ = UNI_REPLACEMENT_CHAR;
            }
        } अन्यथा {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            अगर (target + 1 >= targetEnd) {
                source -= (extraBytesToRead+1); /* Back up source pointer! */
                परिणाम_2 = targetExhausted; अवरोध;
            }
            ch -= halfBase;
            *target++ = (UTF16)((ch >> halfShift) + UNI_SUR_ऊपरी_START);
            *target++ = (UTF16)((ch & halfMask) + UNI_SUR_निचला_START);
        }
    }
    *sourceStart = source;
    *targetStart = target;
    वापस परिणाम_2;
}

/* --------------------------------------------------------------------- */

अटल Conversion_परिणाम ConvertUTF8toUTF32Impl(
        स्थिर UTF8** sourceStart, स्थिर UTF8* sourceEnd,
        UTF32** targetStart, UTF32* targetEnd, ConversionFlags flags,
        Boolean निवेश_Is_Partial) {
    Conversion_परिणाम परिणाम_2 = conversionOK;
    स्थिर UTF8* source = *sourceStart;
    UTF32* target = *targetStart;
    जबतक (source < sourceEnd) {
        UTF32 ch = 0;
        अचिन्हित लघु extraBytesToRead = trailingBytesForUTF8[*source];
        अगर (extraBytesToRead >= sourceEnd - source) {
            अगर (flags == strictConversion || निवेश_Is_Partial) {
                परिणाम_2 = sourceExhausted;
                अवरोध;
            } अन्यथा {
                परिणाम_2 = sourceIllegal;

                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                source += findMaximalSubpartOfIllFormedUTF8Sequence(source,
                                                                    sourceEnd);
                *target++ = UNI_REPLACEMENT_CHAR;
                जारी;
            }
        }
        अगर (target >= targetEnd) {
            परिणाम_2 = targetExhausted; अवरोध;
        }

        /* Do this check whether lenient or strict */
        अगर (!isLegalUTF8(source, extraBytesToRead+1)) {
            परिणाम_2 = sourceIllegal;
            अगर (flags == strictConversion) {
                /* Abort conversion. */
                अवरोध;
            } अन्यथा {
                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                source += findMaximalSubpartOfIllFormedUTF8Sequence(source,
                                                                    sourceEnd);
                *target++ = UNI_REPLACEMENT_CHAR;
                जारी;
            }
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        चयन (extraBytesToRead) {
            हाल 5: ch += *source++; ch <<= 6;
            हाल 4: ch += *source++; ch <<= 6;
            हाल 3: ch += *source++; ch <<= 6;
            हाल 2: ch += *source++; ch <<= 6;
            हाल 1: ch += *source++; ch <<= 6;
            हाल 0: ch += *source++;
        }
        ch -= offsetsFromUTF8[extraBytesToRead];

        अगर (ch <= UNI_MAX_LEGAL_UTF32) {
            /*
             * UTF-16 surrogate values are illegal in UTF-32, and anything
             * over Plane 17 (> 0x10FFFF) is illegal.
             */
            अगर (ch >= UNI_SUR_ऊपरी_START && ch <= UNI_SUR_निचला_END) {
                अगर (flags == strictConversion) {
                    source -= (extraBytesToRead+1); /* return to the illegal value itself */
                    परिणाम_2 = sourceIllegal;
                    अवरोध;
                } अन्यथा {
                    *target++ = UNI_REPLACEMENT_CHAR;
                }
            } अन्यथा {
                *target++ = ch;
            }
        } अन्यथा { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
            परिणाम_2 = sourceIllegal;
            *target++ = UNI_REPLACEMENT_CHAR;
        }
    }
    *sourceStart = source;
    *targetStart = target;
    वापस परिणाम_2;
}

Conversion_परिणाम ConvertUTF8toUTF32Partial(स्थिर UTF8 **sourceStart,
                                           स्थिर UTF8 *sourceEnd,
                                           UTF32 **targetStart,
                                           UTF32 *targetEnd,
                                           ConversionFlags flags) {
  वापस ConvertUTF8toUTF32Impl(sourceStart, sourceEnd, targetStart, targetEnd,
                                flags, /*InputIsPartial=*/true);
}

Conversion_परिणाम ConvertUTF8toUTF32(स्थिर UTF8 **sourceStart,
                                    स्थिर UTF8 *sourceEnd, UTF32 **targetStart,
                                    UTF32 *targetEnd, ConversionFlags flags) {
  वापस ConvertUTF8toUTF32Impl(sourceStart, sourceEnd, targetStart, targetEnd,
                                flags, /*InputIsPartial=*/false);
}

/* ---------------------------------------------------------------------

    Note A.
    The fall-through switches in UTF-8 reading code save a
    temp variable, some decrements & conditionals.  The switches
    are equivalent to the following loop:
        {
            int tmpBytesToRead = extraBytesToRead+1;
            do {
                ch += *source++;
                --tmpBytesToRead;
                if (tmpBytesToRead) ch <<= 6;
            } while (tmpBytesToRead > 0);
        }
    In UTF-8 writing code, the switches on "bytesToWrite" are
    similarly unrolled loops.

   --------------------------------------------------------------------- */

} // namespace llvm

ConvertUTF_RESTORE_WARNINGS

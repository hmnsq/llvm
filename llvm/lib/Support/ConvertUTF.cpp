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

#अन्तर्भाव "llvm/Support/ConvertUTF.h"
#ifdef CVTUTF_DEBUG
#अन्तर्भाव <stdio.h>
#endif
#अन्तर्भाव <assert.h>

/*
 * This code extensively uses fall-through switches.
 * Keep the compiler from warning about that.
 */
#जइ defined(__clang__) && defined(__has_warning)
# जइ __has_warning("-Wimplicit-fallthrough")
#  define ConvertUTF_DISABLE_WARNINGS \
    _Pragma("clang diagnostic push")  \
    _Pragma("clang diagnostic ignored \"-Wimplicit-fallthrough\"")
#  define ConvertUTF_RESTORE_WARNINGS \
    _Pragma("clang diagnostic pop")
# endif
#नोचेद्यदि defined(__GNUC__)
# define ConvertUTF_DISABLE_WARNINGS \
   _Pragma("GCC diagnostic push")    \
   _Pragma("GCC diagnostic ignored \"-Wimplicit-fallthrough\"")
# define ConvertUTF_RESTORE_WARNINGS \
   _Pragma("GCC diagnostic pop")
#endif
#ifndef ConvertUTF_DISABLE_WARNINGS
# define ConvertUTF_DISABLE_WARNINGS
#endif
#ifndef ConvertUTF_RESTORE_WARNINGS
# define ConvertUTF_RESTORE_WARNINGS
#endif

ConvertUTF_DISABLE_WARNINGS

नामाकाश llvm {

स्थिर ध्रुव पूर्णाङ्क halfShift  = 10; /* used for shifting by 10 bits */

स्थिर ध्रुव UTF32 halfBase = 0x0010000UL;
स्थिर ध्रुव UTF32 halfMask = 0x3FFUL;

#define UNI_SUR_उच्चः_START  (UTF32)0xD800
#define UNI_SUR_उच्चः_END    (UTF32)0xDBFF
#define UNI_SUR_नीचः_START   (UTF32)0xDC00
#define UNI_SUR_नीचः_END     (UTF32)0xDFFF

/* --------------------------------------------------------------------- */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
स्थिर ध्रुव वर्ण trailingBytesForUTF8[256] = {
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
स्थिर ध्रुव UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
                     0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
स्थिर ध्रुव UTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */


/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF32toUTF16 (
        ध्रुव UTF32** sourceStart, ध्रुव UTF32* sourceEnd,
        UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags) {
    ConversionResult result = conversionOK;
    ध्रुव UTF32* source = *sourceStart;
    UTF16* target = *targetStart;
    जाव (source < sourceEnd) {
        UTF32 ch;
        जइ (target >= targetEnd) {
            result = targetExhausted; भङ्ग;
        }
        ch = *source++;
        जइ (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
            जइ (ch >= UNI_SUR_उच्चः_START && ch <= UNI_SUR_नीचः_END) {
                जइ (flags == strictConversion) {
                    --source; /* return to the illegal value itself */
                    result = sourceIllegal;
                    भङ्ग;
                } अण्णहा {
                    *target++ = UNI_REPLACEMENT_CHAR;
                }
            } अण्णहा {
                *target++ = (UTF16)ch; /* normal case */
            }
        } अण्णहा जइ (ch > UNI_MAX_LEGAL_UTF32) {
            जइ (flags == strictConversion) {
                result = sourceIllegal;
            } अण्णहा {
                *target++ = UNI_REPLACEMENT_CHAR;
            }
        } अण्णहा {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            जइ (target + 1 >= targetEnd) {
                --source; /* Back up source pointer! */
                result = targetExhausted; भङ्ग;
            }
            ch -= halfBase;
            *target++ = (UTF16)((ch >> halfShift) + UNI_SUR_उच्चः_START);
            *target++ = (UTF16)((ch & halfMask) + UNI_SUR_नीचः_START);
        }
    }
    *sourceStart = source;
    *targetStart = target;
    प्रत्यागम result;
}

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF16toUTF32 (
        ध्रुव UTF16** sourceStart, ध्रुव UTF16* sourceEnd,
        UTF32** targetStart, UTF32* targetEnd, ConversionFlags flags) {
    ConversionResult result = conversionOK;
    ध्रुव UTF16* source = *sourceStart;
    UTF32* target = *targetStart;
    UTF32 ch, ch2;
    जाव (source < sourceEnd) {
        ध्रुव UTF16* oldSource = source; /*  In case we have to back up because of target overflow. */
        ch = *source++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        जइ (ch >= UNI_SUR_उच्चः_START && ch <= UNI_SUR_उच्चः_END) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            जइ (source < sourceEnd) {
                ch2 = *source;
                /* If it's a low surrogate, convert to UTF32. */
                जइ (ch2 >= UNI_SUR_नीचः_START && ch2 <= UNI_SUR_नीचः_END) {
                    ch = ((ch - UNI_SUR_उच्चः_START) << halfShift)
                        + (ch2 - UNI_SUR_नीचः_START) + halfBase;
                    ++source;
                } अण्णहा जइ (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --source; /* return to the illegal value itself */
                    result = sourceIllegal;
                    भङ्ग;
                }
            } अण्णहा { /* We don't have the 16 bits following the high surrogate. */
                --source; /* return to the high surrogate */
                result = sourceExhausted;
                भङ्ग;
            }
        } अण्णहा जइ (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            जइ (ch >= UNI_SUR_नीचः_START && ch <= UNI_SUR_नीचः_END) {
                --source; /* return to the illegal value itself */
                result = sourceIllegal;
                भङ्ग;
            }
        }
        जइ (target >= targetEnd) {
            source = oldSource; /* Back up source pointer! */
            result = targetExhausted; भङ्ग;
        }
        *target++ = ch;
    }
    *sourceStart = source;
    *targetStart = target;
#ifdef CVTUTF_DEBUG
जइ (result == sourceIllegal) {
    fprintf(stderr, "ConvertUTF16toUTF32 illegal seq 0x%04x,%04x\n", ch, ch2);
    fflush(stderr);
}
#endif
    प्रत्यागम result;
}
ConversionResult ConvertUTF16toUTF8 (
        ध्रुव UTF16** sourceStart, ध्रुव UTF16* sourceEnd,
        UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags) {
    ConversionResult result = conversionOK;
    ध्रुव UTF16* source = *sourceStart;
    UTF8* target = *targetStart;
    जाव (source < sourceEnd) {
        UTF32 ch;
        unsigned short bytesToWrite = 0;
        ध्रुव UTF32 byteMask = 0xBF;
        ध्रुव UTF32 byteMark = 0x80;
        ध्रुव UTF16* oldSource = source; /* In case we have to back up because of target overflow. */
        ch = *source++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        जइ (ch >= UNI_SUR_उच्चः_START && ch <= UNI_SUR_उच्चः_END) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            जइ (source < sourceEnd) {
                UTF32 ch2 = *source;
                /* If it's a low surrogate, convert to UTF32. */
                जइ (ch2 >= UNI_SUR_नीचः_START && ch2 <= UNI_SUR_नीचः_END) {
                    ch = ((ch - UNI_SUR_उच्चः_START) << halfShift)
                        + (ch2 - UNI_SUR_नीचः_START) + halfBase;
                    ++source;
                } अण्णहा जइ (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --source; /* return to the illegal value itself */
                    result = sourceIllegal;
                    भङ्ग;
                }
            } अण्णहा { /* We don't have the 16 bits following the high surrogate. */
                --source; /* return to the high surrogate */
                result = sourceExhausted;
                भङ्ग;
            }
        } अण्णहा जइ (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            जइ (ch >= UNI_SUR_नीचः_START && ch <= UNI_SUR_नीचः_END) {
                --source; /* return to the illegal value itself */
                result = sourceIllegal;
                भङ्ग;
            }
        }
        /* Figure out how many bytes the result will require */
        जइ (ch < (UTF32)0x80) {      bytesToWrite = 1;
        } अण्णहा जइ (ch < (UTF32)0x800) {     bytesToWrite = 2;
        } अण्णहा जइ (ch < (UTF32)0x10000) {   bytesToWrite = 3;
        } अण्णहा जइ (ch < (UTF32)0x110000) {  bytesToWrite = 4;
        } अण्णहा {                            bytesToWrite = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
        }

        target += bytesToWrite;
        जइ (target > targetEnd) {
            source = oldSource; /* Back up source pointer! */
            target -= bytesToWrite; result = targetExhausted; भङ्ग;
        }
        विकल्प (bytesToWrite) { /* note: everything falls through. */
            पक्ष 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            पक्ष 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            पक्ष 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            पक्ष 1: *--target =  (UTF8)(ch | firstByteMark[bytesToWrite]);
        }
        target += bytesToWrite;
    }
    *sourceStart = source;
    *targetStart = target;
    प्रत्यागम result;
}

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF32toUTF8 (
        ध्रुव UTF32** sourceStart, ध्रुव UTF32* sourceEnd,
        UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags) {
    ConversionResult result = conversionOK;
    ध्रुव UTF32* source = *sourceStart;
    UTF8* target = *targetStart;
    जाव (source < sourceEnd) {
        UTF32 ch;
        unsigned short bytesToWrite = 0;
        ध्रुव UTF32 byteMask = 0xBF;
        ध्रुव UTF32 byteMark = 0x80;
        ch = *source++;
        जइ (flags == strictConversion ) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            जइ (ch >= UNI_SUR_उच्चः_START && ch <= UNI_SUR_नीचः_END) {
                --source; /* return to the illegal value itself */
                result = sourceIllegal;
                भङ्ग;
            }
        }
        /*
         * Figure out how many bytes the result will require. Turn any
         * illegally large UTF32 things (> Plane 17) into replacement chars.
         */
        जइ (ch < (UTF32)0x80) {      bytesToWrite = 1;
        } अण्णहा जइ (ch < (UTF32)0x800) {     bytesToWrite = 2;
        } अण्णहा जइ (ch < (UTF32)0x10000) {   bytesToWrite = 3;
        } अण्णहा जइ (ch <= UNI_MAX_LEGAL_UTF32) {  bytesToWrite = 4;
        } अण्णहा {                            bytesToWrite = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
                                            result = sourceIllegal;
        }

        target += bytesToWrite;
        जइ (target > targetEnd) {
            --source; /* Back up source pointer! */
            target -= bytesToWrite; result = targetExhausted; भङ्ग;
        }
        विकल्प (bytesToWrite) { /* note: everything falls through. */
            पक्ष 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            पक्ष 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            पक्ष 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            पक्ष 1: *--target = (UTF8) (ch | firstByteMark[bytesToWrite]);
        }
        target += bytesToWrite;
    }
    *sourceStart = source;
    *targetStart = target;
    प्रत्यागम result;
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

स्थिर Boolean isLegalUTF8(ध्रुव UTF8 *source, पूर्णाङ्क length) {
    UTF8 a;
    ध्रुव UTF8 *srcptr = source+length;
    विकल्प (length) {
    स्वभाव: प्रत्यागम मिच्छ;
        /* Everything else falls through when "true"... */
    पक्ष 4: जइ ((a = (*--srcptr)) < 0x80 || a > 0xBF) प्रत्यागम मिच्छ;
    पक्ष 3: जइ ((a = (*--srcptr)) < 0x80 || a > 0xBF) प्रत्यागम मिच्छ;
    पक्ष 2: जइ ((a = (*--srcptr)) < 0x80 || a > 0xBF) प्रत्यागम मिच्छ;

        विकल्प (*source) {
            /* no fall-through in this inner switch */
            पक्ष 0xE0: जइ (a < 0xA0) प्रत्यागम मिच्छ; भङ्ग;
            पक्ष 0xED: जइ (a > 0x9F) प्रत्यागम मिच्छ; भङ्ग;
            पक्ष 0xF0: जइ (a < 0x90) प्रत्यागम मिच्छ; भङ्ग;
            पक्ष 0xF4: जइ (a > 0x8F) प्रत्यागम मिच्छ; भङ्ग;
            स्वभाव:   जइ (a < 0x80) प्रत्यागम मिच्छ;
        }

    पक्ष 1: जइ (*source >= 0x80 && *source < 0xC2) प्रत्यागम मिच्छ;
    }
    जइ (*source > 0xF4) प्रत्यागम मिच्छ;
    प्रत्यागम सच्च;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 sequence is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8Sequence(ध्रुव UTF8 *source, ध्रुव UTF8 *sourceEnd) {
    पूर्णाङ्क length = trailingBytesForUTF8[*source]+1;
    जइ (length > sourceEnd - source) {
        प्रत्यागम मिच्छ;
    }
    प्रत्यागम isLegalUTF8(source, length);
}

/*
 * Exported function to return the size of the first utf-8 code unit sequence,
 * Or 0 if the sequence is not valid;
 */
unsigned getUTF8SequenceSize(ध्रुव UTF8 *source, ध्रुव UTF8 *sourceEnd) {
  पूर्णाङ्क length = trailingBytesForUTF8[*source] + 1;
  प्रत्यागम (length <= sourceEnd - source && isLegalUTF8(source, length)) ? length
                                                                       : 0;
}

/* --------------------------------------------------------------------- */

unsigned findMaximalSubpartOfIllFormedUTF8Sequence(ध्रुव UTF8 *source,
                                                   ध्रुव UTF8 *sourceEnd) {
  UTF8 b1, b2, b3;

  प्रतिज्ञा(!isLegalUTF8Sequence(source, sourceEnd));

  /*
   * Unicode 6.3.0, D93b:
   *
   *   Maximal subpart of an ill-formed subsequence: The longest code unit
   *   subsequence starting at an unconvertible offset that is either:
   *   a. the initial subsequence of a well-formed code unit sequence, or
   *   b. a subsequence of length one.
   */

  जइ (source == sourceEnd)
    प्रत्यागम 0;

  /*
   * Perform case analysis.  See Unicode 6.3.0, Table 3-7. Well-Formed UTF-8
   * Byte Sequences.
   */

  b1 = *source;
  ++source;
  जइ (b1 >= 0xC2 && b1 <= 0xDF) {
    /*
     * First byte is valid, but we know that this code unit sequence is
     * invalid, so the maximal subpart has to end after the first byte.
     */
    प्रत्यागम 1;
  }

  जइ (source == sourceEnd)
    प्रत्यागम 1;

  b2 = *source;
  ++source;

  जइ (b1 == 0xE0) {
    प्रत्यागम (b2 >= 0xA0 && b2 <= 0xBF) ? 2 : 1;
  }
  जइ (b1 >= 0xE1 && b1 <= 0xEC) {
    प्रत्यागम (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  जइ (b1 == 0xED) {
    प्रत्यागम (b2 >= 0x80 && b2 <= 0x9F) ? 2 : 1;
  }
  जइ (b1 >= 0xEE && b1 <= 0xEF) {
    प्रत्यागम (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  जइ (b1 == 0xF0) {
    जइ (b2 >= 0x90 && b2 <= 0xBF) {
      जइ (source == sourceEnd)
        प्रत्यागम 2;

      b3 = *source;
      प्रत्यागम (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    प्रत्यागम 1;
  }
  जइ (b1 >= 0xF1 && b1 <= 0xF3) {
    जइ (b2 >= 0x80 && b2 <= 0xBF) {
      जइ (source == sourceEnd)
        प्रत्यागम 2;

      b3 = *source;
      प्रत्यागम (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    प्रत्यागम 1;
  }
  जइ (b1 == 0xF4) {
    जइ (b2 >= 0x80 && b2 <= 0x8F) {
      जइ (source == sourceEnd)
        प्रत्यागम 2;

      b3 = *source;
      प्रत्यागम (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    प्रत्यागम 1;
  }

  प्रतिज्ञा((b1 >= 0x80 && b1 <= 0xC1) || b1 >= 0xF5);
  /*
   * There are no valid sequences that start with these bytes.  Maximal subpart
   * is defined to have length 1 in these cases.
   */
  प्रत्यागम 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return the total number of bytes in a codepoint
 * represented in UTF-8, given the value of the first byte.
 */
unsigned get_अङ्कः_Bytes_For_UTF8(UTF8 first) {
  प्रत्यागम trailingBytesForUTF8[first] + 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 string is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8String(ध्रुव UTF8 **source, ध्रुव UTF8 *sourceEnd) {
    जाव (*source != sourceEnd) {
        पूर्णाङ्क length = trailingBytesForUTF8[**source] + 1;
        जइ (length > sourceEnd - *source || !isLegalUTF8(*source, length))
            प्रत्यागम मिच्छ;
        *source += length;
    }
    प्रत्यागम सच्च;
}

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF8toUTF16 (
        ध्रुव UTF8** sourceStart, ध्रुव UTF8* sourceEnd,
        UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags) {
    ConversionResult result = conversionOK;
    ध्रुव UTF8* source = *sourceStart;
    UTF16* target = *targetStart;
    जाव (source < sourceEnd) {
        UTF32 ch = 0;
        unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
        जइ (extraBytesToRead >= sourceEnd - source) {
            result = sourceExhausted; भङ्ग;
        }
        /* Do this check whether lenient or strict */
        जइ (!isLegalUTF8(source, extraBytesToRead+1)) {
            result = sourceIllegal;
            भङ्ग;
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        विकल्प (extraBytesToRead) {
            पक्ष 5: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
            पक्ष 4: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
            पक्ष 3: ch += *source++; ch <<= 6;
            पक्ष 2: ch += *source++; ch <<= 6;
            पक्ष 1: ch += *source++; ch <<= 6;
            पक्ष 0: ch += *source++;
        }
        ch -= offsetsFromUTF8[extraBytesToRead];

        जइ (target >= targetEnd) {
            source -= (extraBytesToRead+1); /* Back up source pointer! */
            result = targetExhausted; भङ्ग;
        }
        जइ (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32 */
            जइ (ch >= UNI_SUR_उच्चः_START && ch <= UNI_SUR_नीचः_END) {
                जइ (flags == strictConversion) {
                    source -= (extraBytesToRead+1); /* return to the illegal value itself */
                    result = sourceIllegal;
                    भङ्ग;
                } अण्णहा {
                    *target++ = UNI_REPLACEMENT_CHAR;
                }
            } अण्णहा {
                *target++ = (UTF16)ch; /* normal case */
            }
        } अण्णहा जइ (ch > UNI_MAX_UTF16) {
            जइ (flags == strictConversion) {
                result = sourceIllegal;
                source -= (extraBytesToRead+1); /* return to the start */
                भङ्ग; /* Bail out; shouldn't continue */
            } अण्णहा {
                *target++ = UNI_REPLACEMENT_CHAR;
            }
        } अण्णहा {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            जइ (target + 1 >= targetEnd) {
                source -= (extraBytesToRead+1); /* Back up source pointer! */
                result = targetExhausted; भङ्ग;
            }
            ch -= halfBase;
            *target++ = (UTF16)((ch >> halfShift) + UNI_SUR_उच्चः_START);
            *target++ = (UTF16)((ch & halfMask) + UNI_SUR_नीचः_START);
        }
    }
    *sourceStart = source;
    *targetStart = target;
    प्रत्यागम result;
}

/* --------------------------------------------------------------------- */

स्थिर ConversionResult ConvertUTF8toUTF32Impl(
        ध्रुव UTF8** sourceStart, ध्रुव UTF8* sourceEnd,
        UTF32** targetStart, UTF32* targetEnd, ConversionFlags flags,
        Boolean निवेशः_Is_Partial) {
    ConversionResult result = conversionOK;
    ध्रुव UTF8* source = *sourceStart;
    UTF32* target = *targetStart;
    जाव (source < sourceEnd) {
        UTF32 ch = 0;
        unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
        जइ (extraBytesToRead >= sourceEnd - source) {
            जइ (flags == strictConversion || निवेशः_Is_Partial) {
                result = sourceExhausted;
                भङ्ग;
            } अण्णहा {
                result = sourceIllegal;

                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                source += findMaximalSubpartOfIllFormedUTF8Sequence(source,
                                                                    sourceEnd);
                *target++ = UNI_REPLACEMENT_CHAR;
                अनुवर्त;
            }
        }
        जइ (target >= targetEnd) {
            result = targetExhausted; भङ्ग;
        }

        /* Do this check whether lenient or strict */
        जइ (!isLegalUTF8(source, extraBytesToRead+1)) {
            result = sourceIllegal;
            जइ (flags == strictConversion) {
                /* Abort conversion. */
                भङ्ग;
            } अण्णहा {
                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                source += findMaximalSubpartOfIllFormedUTF8Sequence(source,
                                                                    sourceEnd);
                *target++ = UNI_REPLACEMENT_CHAR;
                अनुवर्त;
            }
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        विकल्प (extraBytesToRead) {
            पक्ष 5: ch += *source++; ch <<= 6;
            पक्ष 4: ch += *source++; ch <<= 6;
            पक्ष 3: ch += *source++; ch <<= 6;
            पक्ष 2: ch += *source++; ch <<= 6;
            पक्ष 1: ch += *source++; ch <<= 6;
            पक्ष 0: ch += *source++;
        }
        ch -= offsetsFromUTF8[extraBytesToRead];

        जइ (ch <= UNI_MAX_LEGAL_UTF32) {
            /*
             * UTF-16 surrogate values are illegal in UTF-32, and anything
             * over Plane 17 (> 0x10FFFF) is illegal.
             */
            जइ (ch >= UNI_SUR_उच्चः_START && ch <= UNI_SUR_नीचः_END) {
                जइ (flags == strictConversion) {
                    source -= (extraBytesToRead+1); /* return to the illegal value itself */
                    result = sourceIllegal;
                    भङ्ग;
                } अण्णहा {
                    *target++ = UNI_REPLACEMENT_CHAR;
                }
            } अण्णहा {
                *target++ = ch;
            }
        } अण्णहा { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
            result = sourceIllegal;
            *target++ = UNI_REPLACEMENT_CHAR;
        }
    }
    *sourceStart = source;
    *targetStart = target;
    प्रत्यागम result;
}

ConversionResult ConvertUTF8toUTF32Partial(ध्रुव UTF8 **sourceStart,
                                           ध्रुव UTF8 *sourceEnd,
                                           UTF32 **targetStart,
                                           UTF32 *targetEnd,
                                           ConversionFlags flags) {
  प्रत्यागम ConvertUTF8toUTF32Impl(sourceStart, sourceEnd, targetStart, targetEnd,
                                flags, /*InputIsPartial=*/सच्च);
}

ConversionResult ConvertUTF8toUTF32(ध्रुव UTF8 **sourceStart,
                                    ध्रुव UTF8 *sourceEnd, UTF32 **targetStart,
                                    UTF32 *targetEnd, ConversionFlags flags) {
  प्रत्यागम ConvertUTF8toUTF32Impl(sourceStart, sourceEnd, targetStart, targetEnd,
                                flags, /*InputIsPartial=*/मिच्छ);
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

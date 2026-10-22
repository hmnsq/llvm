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

#അന്തര്ഭാവ "llvm/Support/ConvertUTF.h"
#ifdef CVTUTF_DEBUG
#അന്തര്ഭാവ <stdio.h>
#endif
#അന്തര്ഭാവ <assert.h>

/*
 * This code extensively uses fall-through switches.
 * Keep the compiler from warning about that.
 */
#എങ്കിൽ defined(__clang__) && defined(__has_warning)
# എങ്കിൽ __has_warning("-Wimplicit-fallthrough")
#  define ConvertUTF_DISABLE_WARNINGS \
    _Pragma("clang diagnostic push")  \
    _Pragma("clang diagnostic ignored \"-Wimplicit-fallthrough\"")
#  define ConvertUTF_RESTORE_WARNINGS \
    _Pragma("clang diagnostic pop")
# endif
#അല്ലെങ്കിലെങ്കിൽ defined(__GNUC__)
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

നാമാകാശ llvm {

സ്ഥിതം സ്ഥിരം പൂർണ്ണസംഖ്യ halfShift  = 10; /* used for shifting by 10 bits */

സ്ഥിതം സ്ഥിരം UTF32 halfBase = 0x0010000UL;
സ്ഥിതം സ്ഥിരം UTF32 halfMask = 0x3FFUL;

#define UNI_SUR_മുകളിലെ_START  (UTF32)0xD800
#define UNI_SUR_മുകളിലെ_END    (UTF32)0xDBFF
#define UNI_SUR_താഴെയുള്ള_START   (UTF32)0xDC00
#define UNI_SUR_താഴെയുള്ള_END     (UTF32)0xDFFF

/* --------------------------------------------------------------------- */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
സ്ഥിതം സ്ഥിരം അക്ഷരം trailingBytesForUTF8[256] = {
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
സ്ഥിതം സ്ഥിരം UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
                     0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
സ്ഥിതം സ്ഥിരം UTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

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
        സ്ഥിരം UTF32** sourceStart, സ്ഥിരം UTF32* sourceEnd,
        UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags) {
    ConversionResult result = conversionOK;
    സ്ഥിരം UTF32* source = *sourceStart;
    UTF16* target = *targetStart;
    ആകുമ്പോൾ (source < sourceEnd) {
        UTF32 ch;
        എങ്കിൽ (target >= targetEnd) {
            result = targetExhausted; നിർത്തുക;
        }
        ch = *source++;
        എങ്കിൽ (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
            എങ്കിൽ (ch >= UNI_SUR_മുകളിലെ_START && ch <= UNI_SUR_താഴെയുള്ള_END) {
                എങ്കിൽ (flags == strictConversion) {
                    --source; /* return to the illegal value itself */
                    result = sourceIllegal;
                    നിർത്തുക;
                } അല്ലെങ്കിൽ {
                    *target++ = UNI_REPLACEMENT_CHAR;
                }
            } അല്ലെങ്കിൽ {
                *target++ = (UTF16)ch; /* normal case */
            }
        } അല്ലെങ്കിൽ എങ്കിൽ (ch > UNI_MAX_LEGAL_UTF32) {
            എങ്കിൽ (flags == strictConversion) {
                result = sourceIllegal;
            } അല്ലെങ്കിൽ {
                *target++ = UNI_REPLACEMENT_CHAR;
            }
        } അല്ലെങ്കിൽ {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            എങ്കിൽ (target + 1 >= targetEnd) {
                --source; /* Back up source pointer! */
                result = targetExhausted; നിർത്തുക;
            }
            ch -= halfBase;
            *target++ = (UTF16)((ch >> halfShift) + UNI_SUR_മുകളിലെ_START);
            *target++ = (UTF16)((ch & halfMask) + UNI_SUR_താഴെയുള്ള_START);
        }
    }
    *sourceStart = source;
    *targetStart = target;
    തിരികെ result;
}

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF16toUTF32 (
        സ്ഥിരം UTF16** sourceStart, സ്ഥിരം UTF16* sourceEnd,
        UTF32** targetStart, UTF32* targetEnd, ConversionFlags flags) {
    ConversionResult result = conversionOK;
    സ്ഥിരം UTF16* source = *sourceStart;
    UTF32* target = *targetStart;
    UTF32 ch, ch2;
    ആകുമ്പോൾ (source < sourceEnd) {
        സ്ഥിരം UTF16* oldSource = source; /*  In case we have to back up because of target overflow. */
        ch = *source++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        എങ്കിൽ (ch >= UNI_SUR_മുകളിലെ_START && ch <= UNI_SUR_മുകളിലെ_END) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            എങ്കിൽ (source < sourceEnd) {
                ch2 = *source;
                /* If it's a low surrogate, convert to UTF32. */
                എങ്കിൽ (ch2 >= UNI_SUR_താഴെയുള്ള_START && ch2 <= UNI_SUR_താഴെയുള്ള_END) {
                    ch = ((ch - UNI_SUR_മുകളിലെ_START) << halfShift)
                        + (ch2 - UNI_SUR_താഴെയുള്ള_START) + halfBase;
                    ++source;
                } അല്ലെങ്കിൽ എങ്കിൽ (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --source; /* return to the illegal value itself */
                    result = sourceIllegal;
                    നിർത്തുക;
                }
            } അല്ലെങ്കിൽ { /* We don't have the 16 bits following the high surrogate. */
                --source; /* return to the high surrogate */
                result = sourceExhausted;
                നിർത്തുക;
            }
        } അല്ലെങ്കിൽ എങ്കിൽ (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            എങ്കിൽ (ch >= UNI_SUR_താഴെയുള്ള_START && ch <= UNI_SUR_താഴെയുള്ള_END) {
                --source; /* return to the illegal value itself */
                result = sourceIllegal;
                നിർത്തുക;
            }
        }
        എങ്കിൽ (target >= targetEnd) {
            source = oldSource; /* Back up source pointer! */
            result = targetExhausted; നിർത്തുക;
        }
        *target++ = ch;
    }
    *sourceStart = source;
    *targetStart = target;
#ifdef CVTUTF_DEBUG
എങ്കിൽ (result == sourceIllegal) {
    fprintf(stderr, "ConvertUTF16toUTF32 illegal seq 0x%04x,%04x\n", ch, ch2);
    fflush(stderr);
}
#endif
    തിരികെ result;
}
ConversionResult ConvertUTF16toUTF8 (
        സ്ഥിരം UTF16** sourceStart, സ്ഥിരം UTF16* sourceEnd,
        UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags) {
    ConversionResult result = conversionOK;
    സ്ഥിരം UTF16* source = *sourceStart;
    UTF8* target = *targetStart;
    ആകുമ്പോൾ (source < sourceEnd) {
        UTF32 ch;
        unsigned short bytesToWrite = 0;
        സ്ഥിരം UTF32 byteMask = 0xBF;
        സ്ഥിരം UTF32 byteMark = 0x80;
        സ്ഥിരം UTF16* oldSource = source; /* In case we have to back up because of target overflow. */
        ch = *source++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        എങ്കിൽ (ch >= UNI_SUR_മുകളിലെ_START && ch <= UNI_SUR_മുകളിലെ_END) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            എങ്കിൽ (source < sourceEnd) {
                UTF32 ch2 = *source;
                /* If it's a low surrogate, convert to UTF32. */
                എങ്കിൽ (ch2 >= UNI_SUR_താഴെയുള്ള_START && ch2 <= UNI_SUR_താഴെയുള്ള_END) {
                    ch = ((ch - UNI_SUR_മുകളിലെ_START) << halfShift)
                        + (ch2 - UNI_SUR_താഴെയുള്ള_START) + halfBase;
                    ++source;
                } അല്ലെങ്കിൽ എങ്കിൽ (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --source; /* return to the illegal value itself */
                    result = sourceIllegal;
                    നിർത്തുക;
                }
            } അല്ലെങ്കിൽ { /* We don't have the 16 bits following the high surrogate. */
                --source; /* return to the high surrogate */
                result = sourceExhausted;
                നിർത്തുക;
            }
        } അല്ലെങ്കിൽ എങ്കിൽ (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            എങ്കിൽ (ch >= UNI_SUR_താഴെയുള്ള_START && ch <= UNI_SUR_താഴെയുള്ള_END) {
                --source; /* return to the illegal value itself */
                result = sourceIllegal;
                നിർത്തുക;
            }
        }
        /* Figure out how many bytes the result will require */
        എങ്കിൽ (ch < (UTF32)0x80) {      bytesToWrite = 1;
        } അല്ലെങ്കിൽ എങ്കിൽ (ch < (UTF32)0x800) {     bytesToWrite = 2;
        } അല്ലെങ്കിൽ എങ്കിൽ (ch < (UTF32)0x10000) {   bytesToWrite = 3;
        } അല്ലെങ്കിൽ എങ്കിൽ (ch < (UTF32)0x110000) {  bytesToWrite = 4;
        } അല്ലെങ്കിൽ {                            bytesToWrite = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
        }

        target += bytesToWrite;
        എങ്കിൽ (target > targetEnd) {
            source = oldSource; /* Back up source pointer! */
            target -= bytesToWrite; result = targetExhausted; നിർത്തുക;
        }
        തിരഞ്ഞെടുപ്പ് (bytesToWrite) { /* note: everything falls through. */
            അവസ്ഥ 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            അവസ്ഥ 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            അവസ്ഥ 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            അവസ്ഥ 1: *--target =  (UTF8)(ch | firstByteMark[bytesToWrite]);
        }
        target += bytesToWrite;
    }
    *sourceStart = source;
    *targetStart = target;
    തിരികെ result;
}

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF32toUTF8 (
        സ്ഥിരം UTF32** sourceStart, സ്ഥിരം UTF32* sourceEnd,
        UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags) {
    ConversionResult result = conversionOK;
    സ്ഥിരം UTF32* source = *sourceStart;
    UTF8* target = *targetStart;
    ആകുമ്പോൾ (source < sourceEnd) {
        UTF32 ch;
        unsigned short bytesToWrite = 0;
        സ്ഥിരം UTF32 byteMask = 0xBF;
        സ്ഥിരം UTF32 byteMark = 0x80;
        ch = *source++;
        എങ്കിൽ (flags == strictConversion ) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            എങ്കിൽ (ch >= UNI_SUR_മുകളിലെ_START && ch <= UNI_SUR_താഴെയുള്ള_END) {
                --source; /* return to the illegal value itself */
                result = sourceIllegal;
                നിർത്തുക;
            }
        }
        /*
         * Figure out how many bytes the result will require. Turn any
         * illegally large UTF32 things (> Plane 17) into replacement chars.
         */
        എങ്കിൽ (ch < (UTF32)0x80) {      bytesToWrite = 1;
        } അല്ലെങ്കിൽ എങ്കിൽ (ch < (UTF32)0x800) {     bytesToWrite = 2;
        } അല്ലെങ്കിൽ എങ്കിൽ (ch < (UTF32)0x10000) {   bytesToWrite = 3;
        } അല്ലെങ്കിൽ എങ്കിൽ (ch <= UNI_MAX_LEGAL_UTF32) {  bytesToWrite = 4;
        } അല്ലെങ്കിൽ {                            bytesToWrite = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
                                            result = sourceIllegal;
        }

        target += bytesToWrite;
        എങ്കിൽ (target > targetEnd) {
            --source; /* Back up source pointer! */
            target -= bytesToWrite; result = targetExhausted; നിർത്തുക;
        }
        തിരഞ്ഞെടുപ്പ് (bytesToWrite) { /* note: everything falls through. */
            അവസ്ഥ 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            അവസ്ഥ 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            അവസ്ഥ 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            അവസ്ഥ 1: *--target = (UTF8) (ch | firstByteMark[bytesToWrite]);
        }
        target += bytesToWrite;
    }
    *sourceStart = source;
    *targetStart = target;
    തിരികെ result;
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

സ്ഥിതം Boolean isLegalUTF8(സ്ഥിരം UTF8 *source, പൂർണ്ണസംഖ്യ length) {
    UTF8 a;
    സ്ഥിരം UTF8 *srcptr = source+length;
    തിരഞ്ഞെടുപ്പ് (length) {
    സ്ഥിരസ്ഥിതി: തിരികെ അസത്യം;
        /* Everything else falls through when "true"... */
    അവസ്ഥ 4: എങ്കിൽ ((a = (*--srcptr)) < 0x80 || a > 0xBF) തിരികെ അസത്യം;
    അവസ്ഥ 3: എങ്കിൽ ((a = (*--srcptr)) < 0x80 || a > 0xBF) തിരികെ അസത്യം;
    അവസ്ഥ 2: എങ്കിൽ ((a = (*--srcptr)) < 0x80 || a > 0xBF) തിരികെ അസത്യം;

        തിരഞ്ഞെടുപ്പ് (*source) {
            /* no fall-through in this inner switch */
            അവസ്ഥ 0xE0: എങ്കിൽ (a < 0xA0) തിരികെ അസത്യം; നിർത്തുക;
            അവസ്ഥ 0xED: എങ്കിൽ (a > 0x9F) തിരികെ അസത്യം; നിർത്തുക;
            അവസ്ഥ 0xF0: എങ്കിൽ (a < 0x90) തിരികെ അസത്യം; നിർത്തുക;
            അവസ്ഥ 0xF4: എങ്കിൽ (a > 0x8F) തിരികെ അസത്യം; നിർത്തുക;
            സ്ഥിരസ്ഥിതി:   എങ്കിൽ (a < 0x80) തിരികെ അസത്യം;
        }

    അവസ്ഥ 1: എങ്കിൽ (*source >= 0x80 && *source < 0xC2) തിരികെ അസത്യം;
    }
    എങ്കിൽ (*source > 0xF4) തിരികെ അസത്യം;
    തിരികെ സത്യം;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 sequence is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8Sequence(സ്ഥിരം UTF8 *source, സ്ഥിരം UTF8 *sourceEnd) {
    പൂർണ്ണസംഖ്യ length = trailingBytesForUTF8[*source]+1;
    എങ്കിൽ (length > sourceEnd - source) {
        തിരികെ അസത്യം;
    }
    തിരികെ isLegalUTF8(source, length);
}

/*
 * Exported function to return the size of the first utf-8 code unit sequence,
 * Or 0 if the sequence is not valid;
 */
unsigned getUTF8SequenceSize(സ്ഥിരം UTF8 *source, സ്ഥിരം UTF8 *sourceEnd) {
  പൂർണ്ണസംഖ്യ length = trailingBytesForUTF8[*source] + 1;
  തിരികെ (length <= sourceEnd - source && isLegalUTF8(source, length)) ? length
                                                                       : 0;
}

/* --------------------------------------------------------------------- */

unsigned findMaximalSubpartOfIllFormedUTF8Sequence(സ്ഥിരം UTF8 *source,
                                                   സ്ഥിരം UTF8 *sourceEnd) {
  UTF8 b1, b2, b3;

  പ്രതിജ്ഞാ(!isLegalUTF8Sequence(source, sourceEnd));

  /*
   * Unicode 6.3.0, D93b:
   *
   *   Maximal subpart of an ill-formed subsequence: The longest code unit
   *   subsequence starting at an unconvertible offset that is either:
   *   a. the initial subsequence of a well-formed code unit sequence, or
   *   b. a subsequence of length one.
   */

  എങ്കിൽ (source == sourceEnd)
    തിരികെ 0;

  /*
   * Perform case analysis.  See Unicode 6.3.0, Table 3-7. Well-Formed UTF-8
   * Byte Sequences.
   */

  b1 = *source;
  ++source;
  എങ്കിൽ (b1 >= 0xC2 && b1 <= 0xDF) {
    /*
     * First byte is valid, but we know that this code unit sequence is
     * invalid, so the maximal subpart has to end after the first byte.
     */
    തിരികെ 1;
  }

  എങ്കിൽ (source == sourceEnd)
    തിരികെ 1;

  b2 = *source;
  ++source;

  എങ്കിൽ (b1 == 0xE0) {
    തിരികെ (b2 >= 0xA0 && b2 <= 0xBF) ? 2 : 1;
  }
  എങ്കിൽ (b1 >= 0xE1 && b1 <= 0xEC) {
    തിരികെ (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  എങ്കിൽ (b1 == 0xED) {
    തിരികെ (b2 >= 0x80 && b2 <= 0x9F) ? 2 : 1;
  }
  എങ്കിൽ (b1 >= 0xEE && b1 <= 0xEF) {
    തിരികെ (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  എങ്കിൽ (b1 == 0xF0) {
    എങ്കിൽ (b2 >= 0x90 && b2 <= 0xBF) {
      എങ്കിൽ (source == sourceEnd)
        തിരികെ 2;

      b3 = *source;
      തിരികെ (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    തിരികെ 1;
  }
  എങ്കിൽ (b1 >= 0xF1 && b1 <= 0xF3) {
    എങ്കിൽ (b2 >= 0x80 && b2 <= 0xBF) {
      എങ്കിൽ (source == sourceEnd)
        തിരികെ 2;

      b3 = *source;
      തിരികെ (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    തിരികെ 1;
  }
  എങ്കിൽ (b1 == 0xF4) {
    എങ്കിൽ (b2 >= 0x80 && b2 <= 0x8F) {
      എങ്കിൽ (source == sourceEnd)
        തിരികെ 2;

      b3 = *source;
      തിരികെ (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    തിരികെ 1;
  }

  പ്രതിജ്ഞാ((b1 >= 0x80 && b1 <= 0xC1) || b1 >= 0xF5);
  /*
   * There are no valid sequences that start with these bytes.  Maximal subpart
   * is defined to have length 1 in these cases.
   */
  തിരികെ 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return the total number of bytes in a codepoint
 * represented in UTF-8, given the value of the first byte.
 */
unsigned get_അക്കം_Bytes_For_UTF8(UTF8 first) {
  തിരികെ trailingBytesForUTF8[first] + 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 string is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8String(സ്ഥിരം UTF8 **source, സ്ഥിരം UTF8 *sourceEnd) {
    ആകുമ്പോൾ (*source != sourceEnd) {
        പൂർണ്ണസംഖ്യ length = trailingBytesForUTF8[**source] + 1;
        എങ്കിൽ (length > sourceEnd - *source || !isLegalUTF8(*source, length))
            തിരികെ അസത്യം;
        *source += length;
    }
    തിരികെ സത്യം;
}

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF8toUTF16 (
        സ്ഥിരം UTF8** sourceStart, സ്ഥിരം UTF8* sourceEnd,
        UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags) {
    ConversionResult result = conversionOK;
    സ്ഥിരം UTF8* source = *sourceStart;
    UTF16* target = *targetStart;
    ആകുമ്പോൾ (source < sourceEnd) {
        UTF32 ch = 0;
        unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
        എങ്കിൽ (extraBytesToRead >= sourceEnd - source) {
            result = sourceExhausted; നിർത്തുക;
        }
        /* Do this check whether lenient or strict */
        എങ്കിൽ (!isLegalUTF8(source, extraBytesToRead+1)) {
            result = sourceIllegal;
            നിർത്തുക;
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        തിരഞ്ഞെടുപ്പ് (extraBytesToRead) {
            അവസ്ഥ 5: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
            അവസ്ഥ 4: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
            അവസ്ഥ 3: ch += *source++; ch <<= 6;
            അവസ്ഥ 2: ch += *source++; ch <<= 6;
            അവസ്ഥ 1: ch += *source++; ch <<= 6;
            അവസ്ഥ 0: ch += *source++;
        }
        ch -= offsetsFromUTF8[extraBytesToRead];

        എങ്കിൽ (target >= targetEnd) {
            source -= (extraBytesToRead+1); /* Back up source pointer! */
            result = targetExhausted; നിർത്തുക;
        }
        എങ്കിൽ (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32 */
            എങ്കിൽ (ch >= UNI_SUR_മുകളിലെ_START && ch <= UNI_SUR_താഴെയുള്ള_END) {
                എങ്കിൽ (flags == strictConversion) {
                    source -= (extraBytesToRead+1); /* return to the illegal value itself */
                    result = sourceIllegal;
                    നിർത്തുക;
                } അല്ലെങ്കിൽ {
                    *target++ = UNI_REPLACEMENT_CHAR;
                }
            } അല്ലെങ്കിൽ {
                *target++ = (UTF16)ch; /* normal case */
            }
        } അല്ലെങ്കിൽ എങ്കിൽ (ch > UNI_MAX_UTF16) {
            എങ്കിൽ (flags == strictConversion) {
                result = sourceIllegal;
                source -= (extraBytesToRead+1); /* return to the start */
                നിർത്തുക; /* Bail out; shouldn't continue */
            } അല്ലെങ്കിൽ {
                *target++ = UNI_REPLACEMENT_CHAR;
            }
        } അല്ലെങ്കിൽ {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            എങ്കിൽ (target + 1 >= targetEnd) {
                source -= (extraBytesToRead+1); /* Back up source pointer! */
                result = targetExhausted; നിർത്തുക;
            }
            ch -= halfBase;
            *target++ = (UTF16)((ch >> halfShift) + UNI_SUR_മുകളിലെ_START);
            *target++ = (UTF16)((ch & halfMask) + UNI_SUR_താഴെയുള്ള_START);
        }
    }
    *sourceStart = source;
    *targetStart = target;
    തിരികെ result;
}

/* --------------------------------------------------------------------- */

സ്ഥിതം ConversionResult ConvertUTF8toUTF32Impl(
        സ്ഥിരം UTF8** sourceStart, സ്ഥിരം UTF8* sourceEnd,
        UTF32** targetStart, UTF32* targetEnd, ConversionFlags flags,
        Boolean നിവേശം_Is_Partial) {
    ConversionResult result = conversionOK;
    സ്ഥിരം UTF8* source = *sourceStart;
    UTF32* target = *targetStart;
    ആകുമ്പോൾ (source < sourceEnd) {
        UTF32 ch = 0;
        unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
        എങ്കിൽ (extraBytesToRead >= sourceEnd - source) {
            എങ്കിൽ (flags == strictConversion || നിവേശം_Is_Partial) {
                result = sourceExhausted;
                നിർത്തുക;
            } അല്ലെങ്കിൽ {
                result = sourceIllegal;

                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                source += findMaximalSubpartOfIllFormedUTF8Sequence(source,
                                                                    sourceEnd);
                *target++ = UNI_REPLACEMENT_CHAR;
                തുടരുക;
            }
        }
        എങ്കിൽ (target >= targetEnd) {
            result = targetExhausted; നിർത്തുക;
        }

        /* Do this check whether lenient or strict */
        എങ്കിൽ (!isLegalUTF8(source, extraBytesToRead+1)) {
            result = sourceIllegal;
            എങ്കിൽ (flags == strictConversion) {
                /* Abort conversion. */
                നിർത്തുക;
            } അല്ലെങ്കിൽ {
                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                source += findMaximalSubpartOfIllFormedUTF8Sequence(source,
                                                                    sourceEnd);
                *target++ = UNI_REPLACEMENT_CHAR;
                തുടരുക;
            }
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        തിരഞ്ഞെടുപ്പ് (extraBytesToRead) {
            അവസ്ഥ 5: ch += *source++; ch <<= 6;
            അവസ്ഥ 4: ch += *source++; ch <<= 6;
            അവസ്ഥ 3: ch += *source++; ch <<= 6;
            അവസ്ഥ 2: ch += *source++; ch <<= 6;
            അവസ്ഥ 1: ch += *source++; ch <<= 6;
            അവസ്ഥ 0: ch += *source++;
        }
        ch -= offsetsFromUTF8[extraBytesToRead];

        എങ്കിൽ (ch <= UNI_MAX_LEGAL_UTF32) {
            /*
             * UTF-16 surrogate values are illegal in UTF-32, and anything
             * over Plane 17 (> 0x10FFFF) is illegal.
             */
            എങ്കിൽ (ch >= UNI_SUR_മുകളിലെ_START && ch <= UNI_SUR_താഴെയുള്ള_END) {
                എങ്കിൽ (flags == strictConversion) {
                    source -= (extraBytesToRead+1); /* return to the illegal value itself */
                    result = sourceIllegal;
                    നിർത്തുക;
                } അല്ലെങ്കിൽ {
                    *target++ = UNI_REPLACEMENT_CHAR;
                }
            } അല്ലെങ്കിൽ {
                *target++ = ch;
            }
        } അല്ലെങ്കിൽ { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
            result = sourceIllegal;
            *target++ = UNI_REPLACEMENT_CHAR;
        }
    }
    *sourceStart = source;
    *targetStart = target;
    തിരികെ result;
}

ConversionResult ConvertUTF8toUTF32Partial(സ്ഥിരം UTF8 **sourceStart,
                                           സ്ഥിരം UTF8 *sourceEnd,
                                           UTF32 **targetStart,
                                           UTF32 *targetEnd,
                                           ConversionFlags flags) {
  തിരികെ ConvertUTF8toUTF32Impl(sourceStart, sourceEnd, targetStart, targetEnd,
                                flags, /*InputIsPartial=*/സത്യം);
}

ConversionResult ConvertUTF8toUTF32(സ്ഥിരം UTF8 **sourceStart,
                                    സ്ഥിരം UTF8 *sourceEnd, UTF32 **targetStart,
                                    UTF32 *targetEnd, ConversionFlags flags) {
  തിരികെ ConvertUTF8toUTF32Impl(sourceStart, sourceEnd, targetStart, targetEnd,
                                flags, /*InputIsPartial=*/അസത്യം);
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

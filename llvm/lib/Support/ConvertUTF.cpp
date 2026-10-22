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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ConvertUTF.h"
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CVTUTF_DEBUG
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>

/*
 * This code extensively uses fall-through switches.
 * Keep the compiler from warning about that.
 */
#ਜੇ defined(__clang__) && defined(__has_warning)
# ਜੇ __has_warning("-Wimplicit-fallthrough")
#  define ConvertUTF_DISABLE_WARNINGS \
    _Pragma("clang diagnostic push")  \
    _Pragma("clang diagnostic ignored \"-Wimplicit-fallthrough\"")
#  define ConvertUTF_RESTORE_WARNINGS \
    _Pragma("clang diagnostic pop")
# endif
#ਜਾਂਜੇ defined(__GNUC__)
# define ConvertUTF_DISABLE_WARNINGS \
   _Pragma("GCC diagnostic push")    \
   _Pragma("GCC diagnostic ignored \"-Wimplicit-fallthrough\"")
# define ConvertUTF_RESTORE_WARNINGS \
   _Pragma("GCC diagnostic pop")
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ConvertUTF_DISABLE_WARNINGS
# define ConvertUTF_DISABLE_WARNINGS
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ConvertUTF_RESTORE_WARNINGS
# define ConvertUTF_RESTORE_WARNINGS
#ਜੇ_ਅੰਤ

ConvertUTF_DISABLE_WARNINGS

ਨਾਮਾਕਾਸ਼ llvm {

ਸਥਿਰ ਅਚਲ ਪੂਰਨ half_ਖਿਸਕਾਓ  = 10; /* used for shifting by 10 bits */

ਸਥਿਰ ਅਚਲ UTF32 half_ਅਧਾਰ = 0x0010000UL;
ਸਥਿਰ ਅਚਲ UTF32 half_ਨਕਾਬ = 0x3FFUL;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNI_SUR_ਉੱਚਾ_ਸ਼ੁਰੂ  (UTF32)0xD800
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNI_SUR_ਉੱਚਾ_ਸਿਰਾ    (UTF32)0xDBFF
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNI_SUR_ਨੀਵਾਂ_ਸ਼ੁਰੂ   (UTF32)0xDC00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNI_SUR_ਨੀਵਾਂ_ਸਿਰਾ     (UTF32)0xDFFF

/* --------------------------------------------------------------------- */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ trailing_ਬਾਈਟਾਂ_For_UTF8[256] = {
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
ਸਥਿਰ ਅਚਲ UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
                     0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
ਸਥਿਰ ਅਚਲ UTF8 ਪਹਿਲਾ_Byte_Mark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */


/* --------------------------------------------------------------------- */

Conversion_ਨਤੀਜਾ ConvertUTF32toUTF16 (
        ਅਚਲ UTF32** ਸਰੋਤ_ਸ਼ੁਰੂ, ਅਚਲ UTF32* ਸਰੋਤ_ਸਿਰਾ,
        UTF16** ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ, UTF16* ਨਿਸ਼ਾਨਾ_ਸਿਰਾ, ConversionFlags flags) {
    Conversion_ਨਤੀਜਾ ਨਤੀਜਾ_2 = conversionOK;
    ਅਚਲ UTF32* ਸਰੋਤ_2 = *ਸਰੋਤ_ਸ਼ੁਰੂ;
    UTF16* ਨਿਸ਼ਾਨਾ = *ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ;
    ਜਦੋਂ (ਸਰੋਤ_2 < ਸਰੋਤ_ਸਿਰਾ) {
        UTF32 ch;
        ਜੇ (ਨਿਸ਼ਾਨਾ >= ਨਿਸ਼ਾਨਾ_ਸਿਰਾ) {
            ਨਤੀਜਾ_2 = ਨਿਸ਼ਾਨਾ_Exhausted; ਰੋਕੋ;
        }
        ch = *ਸਰੋਤ_2++;
        ਜੇ (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
            ਜੇ (ch >= UNI_SUR_ਉੱਚਾ_ਸ਼ੁਰੂ && ch <= UNI_SUR_ਨੀਵਾਂ_ਸਿਰਾ) {
                ਜੇ (flags == strictConversion) {
                    --ਸਰੋਤ_2; /* return to the illegal value itself */
                    ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;
                    ਰੋਕੋ;
                } ਨਹੀਂਤਾਂ {
                    *ਨਿਸ਼ਾਨਾ++ = UNI_REPLACEMENT_CHAR;
                }
            } ਨਹੀਂਤਾਂ {
                *ਨਿਸ਼ਾਨਾ++ = (UTF16)ch; /* normal case */
            }
        } ਨਹੀਂਤਾਂ ਜੇ (ch > UNI_MAX_LEGAL_UTF32) {
            ਜੇ (flags == strictConversion) {
                ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;
            } ਨਹੀਂਤਾਂ {
                *ਨਿਸ਼ਾਨਾ++ = UNI_REPLACEMENT_CHAR;
            }
        } ਨਹੀਂਤਾਂ {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            ਜੇ (ਨਿਸ਼ਾਨਾ + 1 >= ਨਿਸ਼ਾਨਾ_ਸਿਰਾ) {
                --ਸਰੋਤ_2; /* Back up source pointer! */
                ਨਤੀਜਾ_2 = ਨਿਸ਼ਾਨਾ_Exhausted; ਰੋਕੋ;
            }
            ch -= half_ਅਧਾਰ;
            *ਨਿਸ਼ਾਨਾ++ = (UTF16)((ch >> half_ਖਿਸਕਾਓ) + UNI_SUR_ਉੱਚਾ_ਸ਼ੁਰੂ);
            *ਨਿਸ਼ਾਨਾ++ = (UTF16)((ch & half_ਨਕਾਬ) + UNI_SUR_ਨੀਵਾਂ_ਸ਼ੁਰੂ);
        }
    }
    *ਸਰੋਤ_ਸ਼ੁਰੂ = ਸਰੋਤ_2;
    *ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ = ਨਿਸ਼ਾਨਾ;
    ਵਾਪਸ ਨਤੀਜਾ_2;
}

/* --------------------------------------------------------------------- */

Conversion_ਨਤੀਜਾ ConvertUTF16toUTF32 (
        ਅਚਲ UTF16** ਸਰੋਤ_ਸ਼ੁਰੂ, ਅਚਲ UTF16* ਸਰੋਤ_ਸਿਰਾ,
        UTF32** ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ, UTF32* ਨਿਸ਼ਾਨਾ_ਸਿਰਾ, ConversionFlags flags) {
    Conversion_ਨਤੀਜਾ ਨਤੀਜਾ_2 = conversionOK;
    ਅਚਲ UTF16* ਸਰੋਤ_2 = *ਸਰੋਤ_ਸ਼ੁਰੂ;
    UTF32* ਨਿਸ਼ਾਨਾ = *ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ;
    UTF32 ch, ch2;
    ਜਦੋਂ (ਸਰੋਤ_2 < ਸਰੋਤ_ਸਿਰਾ) {
        ਅਚਲ UTF16* old_ਸਰੋਤ = ਸਰੋਤ_2; /*  In case we have to back up because of target overflow. */
        ch = *ਸਰੋਤ_2++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        ਜੇ (ch >= UNI_SUR_ਉੱਚਾ_ਸ਼ੁਰੂ && ch <= UNI_SUR_ਉੱਚਾ_ਸਿਰਾ) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            ਜੇ (ਸਰੋਤ_2 < ਸਰੋਤ_ਸਿਰਾ) {
                ch2 = *ਸਰੋਤ_2;
                /* If it's a low surrogate, convert to UTF32. */
                ਜੇ (ch2 >= UNI_SUR_ਨੀਵਾਂ_ਸ਼ੁਰੂ && ch2 <= UNI_SUR_ਨੀਵਾਂ_ਸਿਰਾ) {
                    ch = ((ch - UNI_SUR_ਉੱਚਾ_ਸ਼ੁਰੂ) << half_ਖਿਸਕਾਓ)
                        + (ch2 - UNI_SUR_ਨੀਵਾਂ_ਸ਼ੁਰੂ) + half_ਅਧਾਰ;
                    ++ਸਰੋਤ_2;
                } ਨਹੀਂਤਾਂ ਜੇ (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --ਸਰੋਤ_2; /* return to the illegal value itself */
                    ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;
                    ਰੋਕੋ;
                }
            } ਨਹੀਂਤਾਂ { /* We don't have the 16 bits following the high surrogate. */
                --ਸਰੋਤ_2; /* return to the high surrogate */
                ਨਤੀਜਾ_2 = ਸਰੋਤ_Exhausted;
                ਰੋਕੋ;
            }
        } ਨਹੀਂਤਾਂ ਜੇ (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            ਜੇ (ch >= UNI_SUR_ਨੀਵਾਂ_ਸ਼ੁਰੂ && ch <= UNI_SUR_ਨੀਵਾਂ_ਸਿਰਾ) {
                --ਸਰੋਤ_2; /* return to the illegal value itself */
                ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;
                ਰੋਕੋ;
            }
        }
        ਜੇ (ਨਿਸ਼ਾਨਾ >= ਨਿਸ਼ਾਨਾ_ਸਿਰਾ) {
            ਸਰੋਤ_2 = old_ਸਰੋਤ; /* Back up source pointer! */
            ਨਤੀਜਾ_2 = ਨਿਸ਼ਾਨਾ_Exhausted; ਰੋਕੋ;
        }
        *ਨਿਸ਼ਾਨਾ++ = ch;
    }
    *ਸਰੋਤ_ਸ਼ੁਰੂ = ਸਰੋਤ_2;
    *ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ = ਨਿਸ਼ਾਨਾ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CVTUTF_DEBUG
ਜੇ (ਨਤੀਜਾ_2 == ਸਰੋਤ_Illegal) {
    fprintf(stderr, "ConvertUTF16toUTF32 illegal seq 0x%04x,%04x\n", ch, ch2);
    fflush(stderr);
}
#ਜੇ_ਅੰਤ
    ਵਾਪਸ ਨਤੀਜਾ_2;
}
Conversion_ਨਤੀਜਾ ConvertUTF16toUTF8 (
        ਅਚਲ UTF16** ਸਰੋਤ_ਸ਼ੁਰੂ, ਅਚਲ UTF16* ਸਰੋਤ_ਸਿਰਾ,
        UTF8** ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ, UTF8* ਨਿਸ਼ਾਨਾ_ਸਿਰਾ, ConversionFlags flags) {
    Conversion_ਨਤੀਜਾ ਨਤੀਜਾ_2 = conversionOK;
    ਅਚਲ UTF16* ਸਰੋਤ_2 = *ਸਰੋਤ_ਸ਼ੁਰੂ;
    UTF8* ਨਿਸ਼ਾਨਾ = *ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ;
    ਜਦੋਂ (ਸਰੋਤ_2 < ਸਰੋਤ_ਸਿਰਾ) {
        UTF32 ch;
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write = 0;
        ਅਚਲ UTF32 byte_ਨਕਾਬ = 0xBF;
        ਅਚਲ UTF32 byteMark = 0x80;
        ਅਚਲ UTF16* old_ਸਰੋਤ = ਸਰੋਤ_2; /* In case we have to back up because of target overflow. */
        ch = *ਸਰੋਤ_2++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        ਜੇ (ch >= UNI_SUR_ਉੱਚਾ_ਸ਼ੁਰੂ && ch <= UNI_SUR_ਉੱਚਾ_ਸਿਰਾ) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            ਜੇ (ਸਰੋਤ_2 < ਸਰੋਤ_ਸਿਰਾ) {
                UTF32 ch2 = *ਸਰੋਤ_2;
                /* If it's a low surrogate, convert to UTF32. */
                ਜੇ (ch2 >= UNI_SUR_ਨੀਵਾਂ_ਸ਼ੁਰੂ && ch2 <= UNI_SUR_ਨੀਵਾਂ_ਸਿਰਾ) {
                    ch = ((ch - UNI_SUR_ਉੱਚਾ_ਸ਼ੁਰੂ) << half_ਖਿਸਕਾਓ)
                        + (ch2 - UNI_SUR_ਨੀਵਾਂ_ਸ਼ੁਰੂ) + half_ਅਧਾਰ;
                    ++ਸਰੋਤ_2;
                } ਨਹੀਂਤਾਂ ਜੇ (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --ਸਰੋਤ_2; /* return to the illegal value itself */
                    ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;
                    ਰੋਕੋ;
                }
            } ਨਹੀਂਤਾਂ { /* We don't have the 16 bits following the high surrogate. */
                --ਸਰੋਤ_2; /* return to the high surrogate */
                ਨਤੀਜਾ_2 = ਸਰੋਤ_Exhausted;
                ਰੋਕੋ;
            }
        } ਨਹੀਂਤਾਂ ਜੇ (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            ਜੇ (ch >= UNI_SUR_ਨੀਵਾਂ_ਸ਼ੁਰੂ && ch <= UNI_SUR_ਨੀਵਾਂ_ਸਿਰਾ) {
                --ਸਰੋਤ_2; /* return to the illegal value itself */
                ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;
                ਰੋਕੋ;
            }
        }
        /* Figure out how many bytes the result will require */
        ਜੇ (ch < (UTF32)0x80) {      ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write = 1;
        } ਨਹੀਂਤਾਂ ਜੇ (ch < (UTF32)0x800) {     ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write = 2;
        } ਨਹੀਂਤਾਂ ਜੇ (ch < (UTF32)0x10000) {   ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write = 3;
        } ਨਹੀਂਤਾਂ ਜੇ (ch < (UTF32)0x110000) {  ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write = 4;
        } ਨਹੀਂਤਾਂ {                            ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
        }

        ਨਿਸ਼ਾਨਾ += ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write;
        ਜੇ (ਨਿਸ਼ਾਨਾ > ਨਿਸ਼ਾਨਾ_ਸਿਰਾ) {
            ਸਰੋਤ_2 = old_ਸਰੋਤ; /* Back up source pointer! */
            ਨਿਸ਼ਾਨਾ -= ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write; ਨਤੀਜਾ_2 = ਨਿਸ਼ਾਨਾ_Exhausted; ਰੋਕੋ;
        }
        ਚੋਣ (ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write) { /* note: everything falls through. */
            ਹਾਲਤ 4: *--ਨਿਸ਼ਾਨਾ = (UTF8)((ch | byteMark) & byte_ਨਕਾਬ); ch >>= 6;
            ਹਾਲਤ 3: *--ਨਿਸ਼ਾਨਾ = (UTF8)((ch | byteMark) & byte_ਨਕਾਬ); ch >>= 6;
            ਹਾਲਤ 2: *--ਨਿਸ਼ਾਨਾ = (UTF8)((ch | byteMark) & byte_ਨਕਾਬ); ch >>= 6;
            ਹਾਲਤ 1: *--ਨਿਸ਼ਾਨਾ =  (UTF8)(ch | ਪਹਿਲਾ_Byte_Mark[ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write]);
        }
        ਨਿਸ਼ਾਨਾ += ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write;
    }
    *ਸਰੋਤ_ਸ਼ੁਰੂ = ਸਰੋਤ_2;
    *ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ = ਨਿਸ਼ਾਨਾ;
    ਵਾਪਸ ਨਤੀਜਾ_2;
}

/* --------------------------------------------------------------------- */

Conversion_ਨਤੀਜਾ ConvertUTF32toUTF8 (
        ਅਚਲ UTF32** ਸਰੋਤ_ਸ਼ੁਰੂ, ਅਚਲ UTF32* ਸਰੋਤ_ਸਿਰਾ,
        UTF8** ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ, UTF8* ਨਿਸ਼ਾਨਾ_ਸਿਰਾ, ConversionFlags flags) {
    Conversion_ਨਤੀਜਾ ਨਤੀਜਾ_2 = conversionOK;
    ਅਚਲ UTF32* ਸਰੋਤ_2 = *ਸਰੋਤ_ਸ਼ੁਰੂ;
    UTF8* ਨਿਸ਼ਾਨਾ = *ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ;
    ਜਦੋਂ (ਸਰੋਤ_2 < ਸਰੋਤ_ਸਿਰਾ) {
        UTF32 ch;
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write = 0;
        ਅਚਲ UTF32 byte_ਨਕਾਬ = 0xBF;
        ਅਚਲ UTF32 byteMark = 0x80;
        ch = *ਸਰੋਤ_2++;
        ਜੇ (flags == strictConversion ) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            ਜੇ (ch >= UNI_SUR_ਉੱਚਾ_ਸ਼ੁਰੂ && ch <= UNI_SUR_ਨੀਵਾਂ_ਸਿਰਾ) {
                --ਸਰੋਤ_2; /* return to the illegal value itself */
                ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;
                ਰੋਕੋ;
            }
        }
        /*
         * Figure out how many bytes the result will require. Turn any
         * illegally large UTF32 things (> Plane 17) into replacement chars.
         */
        ਜੇ (ch < (UTF32)0x80) {      ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write = 1;
        } ਨਹੀਂਤਾਂ ਜੇ (ch < (UTF32)0x800) {     ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write = 2;
        } ਨਹੀਂਤਾਂ ਜੇ (ch < (UTF32)0x10000) {   ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write = 3;
        } ਨਹੀਂਤਾਂ ਜੇ (ch <= UNI_MAX_LEGAL_UTF32) {  ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write = 4;
        } ਨਹੀਂਤਾਂ {                            ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
                                            ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;
        }

        ਨਿਸ਼ਾਨਾ += ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write;
        ਜੇ (ਨਿਸ਼ਾਨਾ > ਨਿਸ਼ਾਨਾ_ਸਿਰਾ) {
            --ਸਰੋਤ_2; /* Back up source pointer! */
            ਨਿਸ਼ਾਨਾ -= ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write; ਨਤੀਜਾ_2 = ਨਿਸ਼ਾਨਾ_Exhausted; ਰੋਕੋ;
        }
        ਚੋਣ (ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write) { /* note: everything falls through. */
            ਹਾਲਤ 4: *--ਨਿਸ਼ਾਨਾ = (UTF8)((ch | byteMark) & byte_ਨਕਾਬ); ch >>= 6;
            ਹਾਲਤ 3: *--ਨਿਸ਼ਾਨਾ = (UTF8)((ch | byteMark) & byte_ਨਕਾਬ); ch >>= 6;
            ਹਾਲਤ 2: *--ਨਿਸ਼ਾਨਾ = (UTF8)((ch | byteMark) & byte_ਨਕਾਬ); ch >>= 6;
            ਹਾਲਤ 1: *--ਨਿਸ਼ਾਨਾ = (UTF8) (ch | ਪਹਿਲਾ_Byte_Mark[ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write]);
        }
        ਨਿਸ਼ਾਨਾ += ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write;
    }
    *ਸਰੋਤ_ਸ਼ੁਰੂ = ਸਰੋਤ_2;
    *ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ = ਨਿਸ਼ਾਨਾ;
    ਵਾਪਸ ਨਤੀਜਾ_2;
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

ਸਥਿਰ Boolean isLegalUTF8(ਅਚਲ UTF8 *ਸਰੋਤ_2, ਪੂਰਨ ਲੰਮਾਈ_2) {
    UTF8 a;
    ਅਚਲ UTF8 *srcptr = ਸਰੋਤ_2+ਲੰਮਾਈ_2;
    ਚੋਣ (ਲੰਮਾਈ_2) {
    ਮੂਲ: ਵਾਪਸ ਝੂਠ;
        /* Everything else falls through when "true"... */
    ਹਾਲਤ 4: ਜੇ ((a = (*--srcptr)) < 0x80 || a > 0xBF) ਵਾਪਸ ਝੂਠ;
    ਹਾਲਤ 3: ਜੇ ((a = (*--srcptr)) < 0x80 || a > 0xBF) ਵਾਪਸ ਝੂਠ;
    ਹਾਲਤ 2: ਜੇ ((a = (*--srcptr)) < 0x80 || a > 0xBF) ਵਾਪਸ ਝੂਠ;

        ਚੋਣ (*ਸਰੋਤ_2) {
            /* no fall-through in this inner switch */
            ਹਾਲਤ 0xE0: ਜੇ (a < 0xA0) ਵਾਪਸ ਝੂਠ; ਰੋਕੋ;
            ਹਾਲਤ 0xED: ਜੇ (a > 0x9F) ਵਾਪਸ ਝੂਠ; ਰੋਕੋ;
            ਹਾਲਤ 0xF0: ਜੇ (a < 0x90) ਵਾਪਸ ਝੂਠ; ਰੋਕੋ;
            ਹਾਲਤ 0xF4: ਜੇ (a > 0x8F) ਵਾਪਸ ਝੂਠ; ਰੋਕੋ;
            ਮੂਲ:   ਜੇ (a < 0x80) ਵਾਪਸ ਝੂਠ;
        }

    ਹਾਲਤ 1: ਜੇ (*ਸਰੋਤ_2 >= 0x80 && *ਸਰੋਤ_2 < 0xC2) ਵਾਪਸ ਝੂਠ;
    }
    ਜੇ (*ਸਰੋਤ_2 > 0xF4) ਵਾਪਸ ਝੂਠ;
    ਵਾਪਸ ਸੱਚ;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 sequence is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8Sequence(ਅਚਲ UTF8 *ਸਰੋਤ_2, ਅਚਲ UTF8 *ਸਰੋਤ_ਸਿਰਾ) {
    ਪੂਰਨ ਲੰਮਾਈ_2 = trailing_ਬਾਈਟਾਂ_For_UTF8[*ਸਰੋਤ_2]+1;
    ਜੇ (ਲੰਮਾਈ_2 > ਸਰੋਤ_ਸਿਰਾ - ਸਰੋਤ_2) {
        ਵਾਪਸ ਝੂਠ;
    }
    ਵਾਪਸ isLegalUTF8(ਸਰੋਤ_2, ਲੰਮਾਈ_2);
}

/*
 * Exported function to return the size of the first utf-8 code unit sequence,
 * Or 0 if the sequence is not valid;
 */
ਬਿਨਾਨਿਸ਼ਾਨੀ get_UTF8_Sequence_ਮਾਪ(ਅਚਲ UTF8 *ਸਰੋਤ_2, ਅਚਲ UTF8 *ਸਰੋਤ_ਸਿਰਾ) {
  ਪੂਰਨ ਲੰਮਾਈ_2 = trailing_ਬਾਈਟਾਂ_For_UTF8[*ਸਰੋਤ_2] + 1;
  ਵਾਪਸ (ਲੰਮਾਈ_2 <= ਸਰੋਤ_ਸਿਰਾ - ਸਰੋਤ_2 && isLegalUTF8(ਸਰੋਤ_2, ਲੰਮਾਈ_2)) ? ਲੰਮਾਈ_2
                                                                       : 0;
}

/* --------------------------------------------------------------------- */

ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੱਭੋ_Maximal_Subpart_ਦਾ_Ill_Formed_UTF8_Sequence(ਅਚਲ UTF8 *ਸਰੋਤ_2,
                                                   ਅਚਲ UTF8 *ਸਰੋਤ_ਸਿਰਾ) {
  UTF8 b1, b2, b3;

  ਪ੍ਰਤਿਜ੍ਞਾ(!isLegalUTF8Sequence(ਸਰੋਤ_2, ਸਰੋਤ_ਸਿਰਾ));

  /*
   * Unicode 6.3.0, D93b:
   *
   *   Maximal subpart of an ill-formed subsequence: The longest code unit
   *   subsequence starting at an unconvertible offset that is either:
   *   a. the initial subsequence of a well-formed code unit sequence, or
   *   b. a subsequence of length one.
   */

  ਜੇ (ਸਰੋਤ_2 == ਸਰੋਤ_ਸਿਰਾ)
    ਵਾਪਸ 0;

  /*
   * Perform case analysis.  See Unicode 6.3.0, Table 3-7. Well-Formed UTF-8
   * Byte Sequences.
   */

  b1 = *ਸਰੋਤ_2;
  ++ਸਰੋਤ_2;
  ਜੇ (b1 >= 0xC2 && b1 <= 0xDF) {
    /*
     * First byte is valid, but we know that this code unit sequence is
     * invalid, so the maximal subpart has to end after the first byte.
     */
    ਵਾਪਸ 1;
  }

  ਜੇ (ਸਰੋਤ_2 == ਸਰੋਤ_ਸਿਰਾ)
    ਵਾਪਸ 1;

  b2 = *ਸਰੋਤ_2;
  ++ਸਰੋਤ_2;

  ਜੇ (b1 == 0xE0) {
    ਵਾਪਸ (b2 >= 0xA0 && b2 <= 0xBF) ? 2 : 1;
  }
  ਜੇ (b1 >= 0xE1 && b1 <= 0xEC) {
    ਵਾਪਸ (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  ਜੇ (b1 == 0xED) {
    ਵਾਪਸ (b2 >= 0x80 && b2 <= 0x9F) ? 2 : 1;
  }
  ਜੇ (b1 >= 0xEE && b1 <= 0xEF) {
    ਵਾਪਸ (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  ਜੇ (b1 == 0xF0) {
    ਜੇ (b2 >= 0x90 && b2 <= 0xBF) {
      ਜੇ (ਸਰੋਤ_2 == ਸਰੋਤ_ਸਿਰਾ)
        ਵਾਪਸ 2;

      b3 = *ਸਰੋਤ_2;
      ਵਾਪਸ (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    ਵਾਪਸ 1;
  }
  ਜੇ (b1 >= 0xF1 && b1 <= 0xF3) {
    ਜੇ (b2 >= 0x80 && b2 <= 0xBF) {
      ਜੇ (ਸਰੋਤ_2 == ਸਰੋਤ_ਸਿਰਾ)
        ਵਾਪਸ 2;

      b3 = *ਸਰੋਤ_2;
      ਵਾਪਸ (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    ਵਾਪਸ 1;
  }
  ਜੇ (b1 == 0xF4) {
    ਜੇ (b2 >= 0x80 && b2 <= 0x8F) {
      ਜੇ (ਸਰੋਤ_2 == ਸਰੋਤ_ਸਿਰਾ)
        ਵਾਪਸ 2;

      b3 = *ਸਰੋਤ_2;
      ਵਾਪਸ (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    ਵਾਪਸ 1;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ((b1 >= 0x80 && b1 <= 0xC1) || b1 >= 0xF5);
  /*
   * There are no valid sequences that start with these bytes.  Maximal subpart
   * is defined to have length 1 in these cases.
   */
  ਵਾਪਸ 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return the total number of bytes in a codepoint
 * represented in UTF-8, given the value of the first byte.
 */
ਬਿਨਾਨਿਸ਼ਾਨੀ get_ਅੰਕ_ਬਾਈਟਾਂ_For_UTF8(UTF8 ਪਹਿਲਾ_2) {
  ਵਾਪਸ trailing_ਬਾਈਟਾਂ_For_UTF8[ਪਹਿਲਾ_2] + 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 string is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8String(ਅਚਲ UTF8 **ਸਰੋਤ_2, ਅਚਲ UTF8 *ਸਰੋਤ_ਸਿਰਾ) {
    ਜਦੋਂ (*ਸਰੋਤ_2 != ਸਰੋਤ_ਸਿਰਾ) {
        ਪੂਰਨ ਲੰਮਾਈ_2 = trailing_ਬਾਈਟਾਂ_For_UTF8[**ਸਰੋਤ_2] + 1;
        ਜੇ (ਲੰਮਾਈ_2 > ਸਰੋਤ_ਸਿਰਾ - *ਸਰੋਤ_2 || !isLegalUTF8(*ਸਰੋਤ_2, ਲੰਮਾਈ_2))
            ਵਾਪਸ ਝੂਠ;
        *ਸਰੋਤ_2 += ਲੰਮਾਈ_2;
    }
    ਵਾਪਸ ਸੱਚ;
}

/* --------------------------------------------------------------------- */

Conversion_ਨਤੀਜਾ ConvertUTF8toUTF16 (
        ਅਚਲ UTF8** ਸਰੋਤ_ਸ਼ੁਰੂ, ਅਚਲ UTF8* ਸਰੋਤ_ਸਿਰਾ,
        UTF16** ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ, UTF16* ਨਿਸ਼ਾਨਾ_ਸਿਰਾ, ConversionFlags flags) {
    Conversion_ਨਤੀਜਾ ਨਤੀਜਾ_2 = conversionOK;
    ਅਚਲ UTF8* ਸਰੋਤ_2 = *ਸਰੋਤ_ਸ਼ੁਰੂ;
    UTF16* ਨਿਸ਼ਾਨਾ = *ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ;
    ਜਦੋਂ (ਸਰੋਤ_2 < ਸਰੋਤ_ਸਿਰਾ) {
        UTF32 ch = 0;
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ extra_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Read = trailing_ਬਾਈਟਾਂ_For_UTF8[*ਸਰੋਤ_2];
        ਜੇ (extra_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Read >= ਸਰੋਤ_ਸਿਰਾ - ਸਰੋਤ_2) {
            ਨਤੀਜਾ_2 = ਸਰੋਤ_Exhausted; ਰੋਕੋ;
        }
        /* Do this check whether lenient or strict */
        ਜੇ (!isLegalUTF8(ਸਰੋਤ_2, extra_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Read+1)) {
            ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;
            ਰੋਕੋ;
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        ਚੋਣ (extra_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Read) {
            ਹਾਲਤ 5: ch += *ਸਰੋਤ_2++; ch <<= 6; /* remember, illegal UTF-8 */
            ਹਾਲਤ 4: ch += *ਸਰੋਤ_2++; ch <<= 6; /* remember, illegal UTF-8 */
            ਹਾਲਤ 3: ch += *ਸਰੋਤ_2++; ch <<= 6;
            ਹਾਲਤ 2: ch += *ਸਰੋਤ_2++; ch <<= 6;
            ਹਾਲਤ 1: ch += *ਸਰੋਤ_2++; ch <<= 6;
            ਹਾਲਤ 0: ch += *ਸਰੋਤ_2++;
        }
        ch -= offsetsFromUTF8[extra_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Read];

        ਜੇ (ਨਿਸ਼ਾਨਾ >= ਨਿਸ਼ਾਨਾ_ਸਿਰਾ) {
            ਸਰੋਤ_2 -= (extra_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Read+1); /* Back up source pointer! */
            ਨਤੀਜਾ_2 = ਨਿਸ਼ਾਨਾ_Exhausted; ਰੋਕੋ;
        }
        ਜੇ (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32 */
            ਜੇ (ch >= UNI_SUR_ਉੱਚਾ_ਸ਼ੁਰੂ && ch <= UNI_SUR_ਨੀਵਾਂ_ਸਿਰਾ) {
                ਜੇ (flags == strictConversion) {
                    ਸਰੋਤ_2 -= (extra_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Read+1); /* return to the illegal value itself */
                    ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;
                    ਰੋਕੋ;
                } ਨਹੀਂਤਾਂ {
                    *ਨਿਸ਼ਾਨਾ++ = UNI_REPLACEMENT_CHAR;
                }
            } ਨਹੀਂਤਾਂ {
                *ਨਿਸ਼ਾਨਾ++ = (UTF16)ch; /* normal case */
            }
        } ਨਹੀਂਤਾਂ ਜੇ (ch > UNI_MAX_UTF16) {
            ਜੇ (flags == strictConversion) {
                ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;
                ਸਰੋਤ_2 -= (extra_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Read+1); /* return to the start */
                ਰੋਕੋ; /* Bail out; shouldn't continue */
            } ਨਹੀਂਤਾਂ {
                *ਨਿਸ਼ਾਨਾ++ = UNI_REPLACEMENT_CHAR;
            }
        } ਨਹੀਂਤਾਂ {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            ਜੇ (ਨਿਸ਼ਾਨਾ + 1 >= ਨਿਸ਼ਾਨਾ_ਸਿਰਾ) {
                ਸਰੋਤ_2 -= (extra_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Read+1); /* Back up source pointer! */
                ਨਤੀਜਾ_2 = ਨਿਸ਼ਾਨਾ_Exhausted; ਰੋਕੋ;
            }
            ch -= half_ਅਧਾਰ;
            *ਨਿਸ਼ਾਨਾ++ = (UTF16)((ch >> half_ਖਿਸਕਾਓ) + UNI_SUR_ਉੱਚਾ_ਸ਼ੁਰੂ);
            *ਨਿਸ਼ਾਨਾ++ = (UTF16)((ch & half_ਨਕਾਬ) + UNI_SUR_ਨੀਵਾਂ_ਸ਼ੁਰੂ);
        }
    }
    *ਸਰੋਤ_ਸ਼ੁਰੂ = ਸਰੋਤ_2;
    *ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ = ਨਿਸ਼ਾਨਾ;
    ਵਾਪਸ ਨਤੀਜਾ_2;
}

/* --------------------------------------------------------------------- */

ਸਥਿਰ Conversion_ਨਤੀਜਾ ConvertUTF8toUTF32Impl(
        ਅਚਲ UTF8** ਸਰੋਤ_ਸ਼ੁਰੂ, ਅਚਲ UTF8* ਸਰੋਤ_ਸਿਰਾ,
        UTF32** ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ, UTF32* ਨਿਸ਼ਾਨਾ_ਸਿਰਾ, ConversionFlags flags,
        Boolean ਨਿਵੇਸ਼_Is_Partial) {
    Conversion_ਨਤੀਜਾ ਨਤੀਜਾ_2 = conversionOK;
    ਅਚਲ UTF8* ਸਰੋਤ_2 = *ਸਰੋਤ_ਸ਼ੁਰੂ;
    UTF32* ਨਿਸ਼ਾਨਾ = *ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ;
    ਜਦੋਂ (ਸਰੋਤ_2 < ਸਰੋਤ_ਸਿਰਾ) {
        UTF32 ch = 0;
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ extra_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Read = trailing_ਬਾਈਟਾਂ_For_UTF8[*ਸਰੋਤ_2];
        ਜੇ (extra_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Read >= ਸਰੋਤ_ਸਿਰਾ - ਸਰੋਤ_2) {
            ਜੇ (flags == strictConversion || ਨਿਵੇਸ਼_Is_Partial) {
                ਨਤੀਜਾ_2 = ਸਰੋਤ_Exhausted;
                ਰੋਕੋ;
            } ਨਹੀਂਤਾਂ {
                ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;

                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                ਸਰੋਤ_2 += ਲੱਭੋ_Maximal_Subpart_ਦਾ_Ill_Formed_UTF8_Sequence(ਸਰੋਤ_2,
                                                                    ਸਰੋਤ_ਸਿਰਾ);
                *ਨਿਸ਼ਾਨਾ++ = UNI_REPLACEMENT_CHAR;
                ਜਾਰੀ;
            }
        }
        ਜੇ (ਨਿਸ਼ਾਨਾ >= ਨਿਸ਼ਾਨਾ_ਸਿਰਾ) {
            ਨਤੀਜਾ_2 = ਨਿਸ਼ਾਨਾ_Exhausted; ਰੋਕੋ;
        }

        /* Do this check whether lenient or strict */
        ਜੇ (!isLegalUTF8(ਸਰੋਤ_2, extra_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Read+1)) {
            ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;
            ਜੇ (flags == strictConversion) {
                /* Abort conversion. */
                ਰੋਕੋ;
            } ਨਹੀਂਤਾਂ {
                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                ਸਰੋਤ_2 += ਲੱਭੋ_Maximal_Subpart_ਦਾ_Ill_Formed_UTF8_Sequence(ਸਰੋਤ_2,
                                                                    ਸਰੋਤ_ਸਿਰਾ);
                *ਨਿਸ਼ਾਨਾ++ = UNI_REPLACEMENT_CHAR;
                ਜਾਰੀ;
            }
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        ਚੋਣ (extra_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Read) {
            ਹਾਲਤ 5: ch += *ਸਰੋਤ_2++; ch <<= 6;
            ਹਾਲਤ 4: ch += *ਸਰੋਤ_2++; ch <<= 6;
            ਹਾਲਤ 3: ch += *ਸਰੋਤ_2++; ch <<= 6;
            ਹਾਲਤ 2: ch += *ਸਰੋਤ_2++; ch <<= 6;
            ਹਾਲਤ 1: ch += *ਸਰੋਤ_2++; ch <<= 6;
            ਹਾਲਤ 0: ch += *ਸਰੋਤ_2++;
        }
        ch -= offsetsFromUTF8[extra_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Read];

        ਜੇ (ch <= UNI_MAX_LEGAL_UTF32) {
            /*
             * UTF-16 surrogate values are illegal in UTF-32, and anything
             * over Plane 17 (> 0x10FFFF) is illegal.
             */
            ਜੇ (ch >= UNI_SUR_ਉੱਚਾ_ਸ਼ੁਰੂ && ch <= UNI_SUR_ਨੀਵਾਂ_ਸਿਰਾ) {
                ਜੇ (flags == strictConversion) {
                    ਸਰੋਤ_2 -= (extra_ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Read+1); /* return to the illegal value itself */
                    ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;
                    ਰੋਕੋ;
                } ਨਹੀਂਤਾਂ {
                    *ਨਿਸ਼ਾਨਾ++ = UNI_REPLACEMENT_CHAR;
                }
            } ਨਹੀਂਤਾਂ {
                *ਨਿਸ਼ਾਨਾ++ = ch;
            }
        } ਨਹੀਂਤਾਂ { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
            ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;
            *ਨਿਸ਼ਾਨਾ++ = UNI_REPLACEMENT_CHAR;
        }
    }
    *ਸਰੋਤ_ਸ਼ੁਰੂ = ਸਰੋਤ_2;
    *ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ = ਨਿਸ਼ਾਨਾ;
    ਵਾਪਸ ਨਤੀਜਾ_2;
}

Conversion_ਨਤੀਜਾ ConvertUTF8toUTF32Partial(ਅਚਲ UTF8 **ਸਰੋਤ_ਸ਼ੁਰੂ,
                                           ਅਚਲ UTF8 *ਸਰੋਤ_ਸਿਰਾ,
                                           UTF32 **ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ,
                                           UTF32 *ਨਿਸ਼ਾਨਾ_ਸਿਰਾ,
                                           ConversionFlags flags) {
  ਵਾਪਸ ConvertUTF8toUTF32Impl(ਸਰੋਤ_ਸ਼ੁਰੂ, ਸਰੋਤ_ਸਿਰਾ, ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ, ਨਿਸ਼ਾਨਾ_ਸਿਰਾ,
                                flags, /*InputIsPartial=*/ਸੱਚ);
}

Conversion_ਨਤੀਜਾ ConvertUTF8toUTF32(ਅਚਲ UTF8 **ਸਰੋਤ_ਸ਼ੁਰੂ,
                                    ਅਚਲ UTF8 *ਸਰੋਤ_ਸਿਰਾ, UTF32 **ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ,
                                    UTF32 *ਨਿਸ਼ਾਨਾ_ਸਿਰਾ, ConversionFlags flags) {
  ਵਾਪਸ ConvertUTF8toUTF32Impl(ਸਰੋਤ_ਸ਼ੁਰੂ, ਸਰੋਤ_ਸਿਰਾ, ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ, ਨਿਸ਼ਾਨਾ_ਸਿਰਾ,
                                flags, /*InputIsPartial=*/ਝੂਠ);
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

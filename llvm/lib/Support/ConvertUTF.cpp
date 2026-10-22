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

#شامل "llvm/Support/ConvertUTF.h"
#اگر_تعریف_شدہ CVTUTF_DEBUG
#شامل <stdio.h>
#اگر_ختم
#شامل <assert.h>

/*
 * This code extensively uses fall-through switches.
 * Keep the compiler from warning about that.
 */
#اگر defined(__clang__) && defined(__has_warning)
# اگر __has_warning("-Wimplicit-fallthrough")
#  define ConvertUTF_DISABLE_WARNINGS \
    _Pragma("clang diagnostic push")  \
    _Pragma("clang diagnostic ignored \"-Wimplicit-fallthrough\"")
#  define ConvertUTF_RESTORE_WARNINGS \
    _Pragma("clang diagnostic pop")
# endif
#وگرنہ_اگر defined(__GNUC__)
# define ConvertUTF_DISABLE_WARNINGS \
   _Pragma("GCC diagnostic push")    \
   _Pragma("GCC diagnostic ignored \"-Wimplicit-fallthrough\"")
# define ConvertUTF_RESTORE_WARNINGS \
   _Pragma("GCC diagnostic pop")
#اگر_ختم
#اگر_غیر_تعریف_شدہ ConvertUTF_DISABLE_WARNINGS
# define ConvertUTF_DISABLE_WARNINGS
#اگر_ختم
#اگر_غیر_تعریف_شدہ ConvertUTF_RESTORE_WARNINGS
# define ConvertUTF_RESTORE_WARNINGS
#اگر_ختم

ConvertUTF_DISABLE_WARNINGS

نامخلا llvm {

ساکن ثابت صحیح half_سرکاؤ  = 10; /* used for shifting by 10 bits */

ساکن ثابت UTF32 half_بنیاد = 0x0010000UL;
ساکن ثابت UTF32 half_نقاب = 0x3FFUL;

#تعریف_کرو UNI_SUR_بلند_آغاز  (UTF32)0xD800
#تعریف_کرو UNI_SUR_بلند_انتہا    (UTF32)0xDBFF
#تعریف_کرو UNI_SUR_کم_آغاز   (UTF32)0xDC00
#تعریف_کرو UNI_SUR_کم_انتہا     (UTF32)0xDFFF

/* --------------------------------------------------------------------- */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
ساکن ثابت حرف trailing_بائٹیں_For_UTF8[256] = {
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
ساکن ثابت UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
                     0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
ساکن ثابت UTF8 پہلا_Byte_Mark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */


/* --------------------------------------------------------------------- */

Conversion_نتیجہ ConvertUTF32toUTF16 (
        ثابت UTF32** ماخذ_آغاز, ثابت UTF32* ماخذ_انتہا,
        UTF16** ہدف_آغاز, UTF16* ہدف_انتہا, ConversionFlags flags) {
    Conversion_نتیجہ نتیجہ_2 = conversionOK;
    ثابت UTF32* ماخذ_2 = *ماخذ_آغاز;
    UTF16* ہدف = *ہدف_آغاز;
    جب (ماخذ_2 < ماخذ_انتہا) {
        UTF32 ch;
        اگر (ہدف >= ہدف_انتہا) {
            نتیجہ_2 = ہدف_Exhausted; توڑو;
        }
        ch = *ماخذ_2++;
        اگر (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
            اگر (ch >= UNI_SUR_بلند_آغاز && ch <= UNI_SUR_کم_انتہا) {
                اگر (flags == strictConversion) {
                    --ماخذ_2; /* return to the illegal value itself */
                    نتیجہ_2 = ماخذ_Illegal;
                    توڑو;
                } ورنہ {
                    *ہدف++ = UNI_REPLACEMENT_CHAR;
                }
            } ورنہ {
                *ہدف++ = (UTF16)ch; /* normal case */
            }
        } ورنہ اگر (ch > UNI_MAX_LEGAL_UTF32) {
            اگر (flags == strictConversion) {
                نتیجہ_2 = ماخذ_Illegal;
            } ورنہ {
                *ہدف++ = UNI_REPLACEMENT_CHAR;
            }
        } ورنہ {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            اگر (ہدف + 1 >= ہدف_انتہا) {
                --ماخذ_2; /* Back up source pointer! */
                نتیجہ_2 = ہدف_Exhausted; توڑو;
            }
            ch -= half_بنیاد;
            *ہدف++ = (UTF16)((ch >> half_سرکاؤ) + UNI_SUR_بلند_آغاز);
            *ہدف++ = (UTF16)((ch & half_نقاب) + UNI_SUR_کم_آغاز);
        }
    }
    *ماخذ_آغاز = ماخذ_2;
    *ہدف_آغاز = ہدف;
    واپس نتیجہ_2;
}

/* --------------------------------------------------------------------- */

Conversion_نتیجہ ConvertUTF16toUTF32 (
        ثابت UTF16** ماخذ_آغاز, ثابت UTF16* ماخذ_انتہا,
        UTF32** ہدف_آغاز, UTF32* ہدف_انتہا, ConversionFlags flags) {
    Conversion_نتیجہ نتیجہ_2 = conversionOK;
    ثابت UTF16* ماخذ_2 = *ماخذ_آغاز;
    UTF32* ہدف = *ہدف_آغاز;
    UTF32 ch, ch2;
    جب (ماخذ_2 < ماخذ_انتہا) {
        ثابت UTF16* old_ماخذ = ماخذ_2; /*  In case we have to back up because of target overflow. */
        ch = *ماخذ_2++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        اگر (ch >= UNI_SUR_بلند_آغاز && ch <= UNI_SUR_بلند_انتہا) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            اگر (ماخذ_2 < ماخذ_انتہا) {
                ch2 = *ماخذ_2;
                /* If it's a low surrogate, convert to UTF32. */
                اگر (ch2 >= UNI_SUR_کم_آغاز && ch2 <= UNI_SUR_کم_انتہا) {
                    ch = ((ch - UNI_SUR_بلند_آغاز) << half_سرکاؤ)
                        + (ch2 - UNI_SUR_کم_آغاز) + half_بنیاد;
                    ++ماخذ_2;
                } ورنہ اگر (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --ماخذ_2; /* return to the illegal value itself */
                    نتیجہ_2 = ماخذ_Illegal;
                    توڑو;
                }
            } ورنہ { /* We don't have the 16 bits following the high surrogate. */
                --ماخذ_2; /* return to the high surrogate */
                نتیجہ_2 = ماخذ_Exhausted;
                توڑو;
            }
        } ورنہ اگر (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            اگر (ch >= UNI_SUR_کم_آغاز && ch <= UNI_SUR_کم_انتہا) {
                --ماخذ_2; /* return to the illegal value itself */
                نتیجہ_2 = ماخذ_Illegal;
                توڑو;
            }
        }
        اگر (ہدف >= ہدف_انتہا) {
            ماخذ_2 = old_ماخذ; /* Back up source pointer! */
            نتیجہ_2 = ہدف_Exhausted; توڑو;
        }
        *ہدف++ = ch;
    }
    *ماخذ_آغاز = ماخذ_2;
    *ہدف_آغاز = ہدف;
#اگر_تعریف_شدہ CVTUTF_DEBUG
اگر (نتیجہ_2 == ماخذ_Illegal) {
    fprintf(stderr, "ConvertUTF16toUTF32 illegal seq 0x%04x,%04x\n", ch, ch2);
    fflush(stderr);
}
#اگر_ختم
    واپس نتیجہ_2;
}
Conversion_نتیجہ ConvertUTF16toUTF8 (
        ثابت UTF16** ماخذ_آغاز, ثابت UTF16* ماخذ_انتہا,
        UTF8** ہدف_آغاز, UTF8* ہدف_انتہا, ConversionFlags flags) {
    Conversion_نتیجہ نتیجہ_2 = conversionOK;
    ثابت UTF16* ماخذ_2 = *ماخذ_آغاز;
    UTF8* ہدف = *ہدف_آغاز;
    جب (ماخذ_2 < ماخذ_انتہا) {
        UTF32 ch;
        غیرعلامتی مختصر بائٹیں_مقصد_Write = 0;
        ثابت UTF32 byte_نقاب = 0xBF;
        ثابت UTF32 byteMark = 0x80;
        ثابت UTF16* old_ماخذ = ماخذ_2; /* In case we have to back up because of target overflow. */
        ch = *ماخذ_2++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        اگر (ch >= UNI_SUR_بلند_آغاز && ch <= UNI_SUR_بلند_انتہا) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            اگر (ماخذ_2 < ماخذ_انتہا) {
                UTF32 ch2 = *ماخذ_2;
                /* If it's a low surrogate, convert to UTF32. */
                اگر (ch2 >= UNI_SUR_کم_آغاز && ch2 <= UNI_SUR_کم_انتہا) {
                    ch = ((ch - UNI_SUR_بلند_آغاز) << half_سرکاؤ)
                        + (ch2 - UNI_SUR_کم_آغاز) + half_بنیاد;
                    ++ماخذ_2;
                } ورنہ اگر (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --ماخذ_2; /* return to the illegal value itself */
                    نتیجہ_2 = ماخذ_Illegal;
                    توڑو;
                }
            } ورنہ { /* We don't have the 16 bits following the high surrogate. */
                --ماخذ_2; /* return to the high surrogate */
                نتیجہ_2 = ماخذ_Exhausted;
                توڑو;
            }
        } ورنہ اگر (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            اگر (ch >= UNI_SUR_کم_آغاز && ch <= UNI_SUR_کم_انتہا) {
                --ماخذ_2; /* return to the illegal value itself */
                نتیجہ_2 = ماخذ_Illegal;
                توڑو;
            }
        }
        /* Figure out how many bytes the result will require */
        اگر (ch < (UTF32)0x80) {      بائٹیں_مقصد_Write = 1;
        } ورنہ اگر (ch < (UTF32)0x800) {     بائٹیں_مقصد_Write = 2;
        } ورنہ اگر (ch < (UTF32)0x10000) {   بائٹیں_مقصد_Write = 3;
        } ورنہ اگر (ch < (UTF32)0x110000) {  بائٹیں_مقصد_Write = 4;
        } ورنہ {                            بائٹیں_مقصد_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
        }

        ہدف += بائٹیں_مقصد_Write;
        اگر (ہدف > ہدف_انتہا) {
            ماخذ_2 = old_ماخذ; /* Back up source pointer! */
            ہدف -= بائٹیں_مقصد_Write; نتیجہ_2 = ہدف_Exhausted; توڑو;
        }
        انتخاب (بائٹیں_مقصد_Write) { /* note: everything falls through. */
            حالت 4: *--ہدف = (UTF8)((ch | byteMark) & byte_نقاب); ch >>= 6;
            حالت 3: *--ہدف = (UTF8)((ch | byteMark) & byte_نقاب); ch >>= 6;
            حالت 2: *--ہدف = (UTF8)((ch | byteMark) & byte_نقاب); ch >>= 6;
            حالت 1: *--ہدف =  (UTF8)(ch | پہلا_Byte_Mark[بائٹیں_مقصد_Write]);
        }
        ہدف += بائٹیں_مقصد_Write;
    }
    *ماخذ_آغاز = ماخذ_2;
    *ہدف_آغاز = ہدف;
    واپس نتیجہ_2;
}

/* --------------------------------------------------------------------- */

Conversion_نتیجہ ConvertUTF32toUTF8 (
        ثابت UTF32** ماخذ_آغاز, ثابت UTF32* ماخذ_انتہا,
        UTF8** ہدف_آغاز, UTF8* ہدف_انتہا, ConversionFlags flags) {
    Conversion_نتیجہ نتیجہ_2 = conversionOK;
    ثابت UTF32* ماخذ_2 = *ماخذ_آغاز;
    UTF8* ہدف = *ہدف_آغاز;
    جب (ماخذ_2 < ماخذ_انتہا) {
        UTF32 ch;
        غیرعلامتی مختصر بائٹیں_مقصد_Write = 0;
        ثابت UTF32 byte_نقاب = 0xBF;
        ثابت UTF32 byteMark = 0x80;
        ch = *ماخذ_2++;
        اگر (flags == strictConversion ) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            اگر (ch >= UNI_SUR_بلند_آغاز && ch <= UNI_SUR_کم_انتہا) {
                --ماخذ_2; /* return to the illegal value itself */
                نتیجہ_2 = ماخذ_Illegal;
                توڑو;
            }
        }
        /*
         * Figure out how many bytes the result will require. Turn any
         * illegally large UTF32 things (> Plane 17) into replacement chars.
         */
        اگر (ch < (UTF32)0x80) {      بائٹیں_مقصد_Write = 1;
        } ورنہ اگر (ch < (UTF32)0x800) {     بائٹیں_مقصد_Write = 2;
        } ورنہ اگر (ch < (UTF32)0x10000) {   بائٹیں_مقصد_Write = 3;
        } ورنہ اگر (ch <= UNI_MAX_LEGAL_UTF32) {  بائٹیں_مقصد_Write = 4;
        } ورنہ {                            بائٹیں_مقصد_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
                                            نتیجہ_2 = ماخذ_Illegal;
        }

        ہدف += بائٹیں_مقصد_Write;
        اگر (ہدف > ہدف_انتہا) {
            --ماخذ_2; /* Back up source pointer! */
            ہدف -= بائٹیں_مقصد_Write; نتیجہ_2 = ہدف_Exhausted; توڑو;
        }
        انتخاب (بائٹیں_مقصد_Write) { /* note: everything falls through. */
            حالت 4: *--ہدف = (UTF8)((ch | byteMark) & byte_نقاب); ch >>= 6;
            حالت 3: *--ہدف = (UTF8)((ch | byteMark) & byte_نقاب); ch >>= 6;
            حالت 2: *--ہدف = (UTF8)((ch | byteMark) & byte_نقاب); ch >>= 6;
            حالت 1: *--ہدف = (UTF8) (ch | پہلا_Byte_Mark[بائٹیں_مقصد_Write]);
        }
        ہدف += بائٹیں_مقصد_Write;
    }
    *ماخذ_آغاز = ماخذ_2;
    *ہدف_آغاز = ہدف;
    واپس نتیجہ_2;
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

ساکن Boolean isLegalUTF8(ثابت UTF8 *ماخذ_2, صحیح لمبائی_2) {
    UTF8 a;
    ثابت UTF8 *srcptr = ماخذ_2+لمبائی_2;
    انتخاب (لمبائی_2) {
    طےشدہ: واپس جھوٹ;
        /* Everything else falls through when "true"... */
    حالت 4: اگر ((a = (*--srcptr)) < 0x80 || a > 0xBF) واپس جھوٹ;
    حالت 3: اگر ((a = (*--srcptr)) < 0x80 || a > 0xBF) واپس جھوٹ;
    حالت 2: اگر ((a = (*--srcptr)) < 0x80 || a > 0xBF) واپس جھوٹ;

        انتخاب (*ماخذ_2) {
            /* no fall-through in this inner switch */
            حالت 0xE0: اگر (a < 0xA0) واپس جھوٹ; توڑو;
            حالت 0xED: اگر (a > 0x9F) واپس جھوٹ; توڑو;
            حالت 0xF0: اگر (a < 0x90) واپس جھوٹ; توڑو;
            حالت 0xF4: اگر (a > 0x8F) واپس جھوٹ; توڑو;
            طےشدہ:   اگر (a < 0x80) واپس جھوٹ;
        }

    حالت 1: اگر (*ماخذ_2 >= 0x80 && *ماخذ_2 < 0xC2) واپس جھوٹ;
    }
    اگر (*ماخذ_2 > 0xF4) واپس جھوٹ;
    واپس سچ;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 sequence is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8Sequence(ثابت UTF8 *ماخذ_2, ثابت UTF8 *ماخذ_انتہا) {
    صحیح لمبائی_2 = trailing_بائٹیں_For_UTF8[*ماخذ_2]+1;
    اگر (لمبائی_2 > ماخذ_انتہا - ماخذ_2) {
        واپس جھوٹ;
    }
    واپس isLegalUTF8(ماخذ_2, لمبائی_2);
}

/*
 * Exported function to return the size of the first utf-8 code unit sequence,
 * Or 0 if the sequence is not valid;
 */
غیرعلامتی get_UTF8_Sequence_سائز(ثابت UTF8 *ماخذ_2, ثابت UTF8 *ماخذ_انتہا) {
  صحیح لمبائی_2 = trailing_بائٹیں_For_UTF8[*ماخذ_2] + 1;
  واپس (لمبائی_2 <= ماخذ_انتہا - ماخذ_2 && isLegalUTF8(ماخذ_2, لمبائی_2)) ? لمبائی_2
                                                                       : 0;
}

/* --------------------------------------------------------------------- */

غیرعلامتی ڈھونڈو_Maximal_Subpart_کا_Ill_Formed_UTF8_Sequence(ثابت UTF8 *ماخذ_2,
                                                   ثابت UTF8 *ماخذ_انتہا) {
  UTF8 b1, b2, b3;

  أكد(!isLegalUTF8Sequence(ماخذ_2, ماخذ_انتہا));

  /*
   * Unicode 6.3.0, D93b:
   *
   *   Maximal subpart of an ill-formed subsequence: The longest code unit
   *   subsequence starting at an unconvertible offset that is either:
   *   a. the initial subsequence of a well-formed code unit sequence, or
   *   b. a subsequence of length one.
   */

  اگر (ماخذ_2 == ماخذ_انتہا)
    واپس 0;

  /*
   * Perform case analysis.  See Unicode 6.3.0, Table 3-7. Well-Formed UTF-8
   * Byte Sequences.
   */

  b1 = *ماخذ_2;
  ++ماخذ_2;
  اگر (b1 >= 0xC2 && b1 <= 0xDF) {
    /*
     * First byte is valid, but we know that this code unit sequence is
     * invalid, so the maximal subpart has to end after the first byte.
     */
    واپس 1;
  }

  اگر (ماخذ_2 == ماخذ_انتہا)
    واپس 1;

  b2 = *ماخذ_2;
  ++ماخذ_2;

  اگر (b1 == 0xE0) {
    واپس (b2 >= 0xA0 && b2 <= 0xBF) ? 2 : 1;
  }
  اگر (b1 >= 0xE1 && b1 <= 0xEC) {
    واپس (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  اگر (b1 == 0xED) {
    واپس (b2 >= 0x80 && b2 <= 0x9F) ? 2 : 1;
  }
  اگر (b1 >= 0xEE && b1 <= 0xEF) {
    واپس (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  اگر (b1 == 0xF0) {
    اگر (b2 >= 0x90 && b2 <= 0xBF) {
      اگر (ماخذ_2 == ماخذ_انتہا)
        واپس 2;

      b3 = *ماخذ_2;
      واپس (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    واپس 1;
  }
  اگر (b1 >= 0xF1 && b1 <= 0xF3) {
    اگر (b2 >= 0x80 && b2 <= 0xBF) {
      اگر (ماخذ_2 == ماخذ_انتہا)
        واپس 2;

      b3 = *ماخذ_2;
      واپس (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    واپس 1;
  }
  اگر (b1 == 0xF4) {
    اگر (b2 >= 0x80 && b2 <= 0x8F) {
      اگر (ماخذ_2 == ماخذ_انتہا)
        واپس 2;

      b3 = *ماخذ_2;
      واپس (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    واپس 1;
  }

  أكد((b1 >= 0x80 && b1 <= 0xC1) || b1 >= 0xF5);
  /*
   * There are no valid sequences that start with these bytes.  Maximal subpart
   * is defined to have length 1 in these cases.
   */
  واپس 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return the total number of bytes in a codepoint
 * represented in UTF-8, given the value of the first byte.
 */
غیرعلامتی get_عدد_بائٹیں_For_UTF8(UTF8 پہلا_2) {
  واپس trailing_بائٹیں_For_UTF8[پہلا_2] + 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 string is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8String(ثابت UTF8 **ماخذ_2, ثابت UTF8 *ماخذ_انتہا) {
    جب (*ماخذ_2 != ماخذ_انتہا) {
        صحیح لمبائی_2 = trailing_بائٹیں_For_UTF8[**ماخذ_2] + 1;
        اگر (لمبائی_2 > ماخذ_انتہا - *ماخذ_2 || !isLegalUTF8(*ماخذ_2, لمبائی_2))
            واپس جھوٹ;
        *ماخذ_2 += لمبائی_2;
    }
    واپس سچ;
}

/* --------------------------------------------------------------------- */

Conversion_نتیجہ ConvertUTF8toUTF16 (
        ثابت UTF8** ماخذ_آغاز, ثابت UTF8* ماخذ_انتہا,
        UTF16** ہدف_آغاز, UTF16* ہدف_انتہا, ConversionFlags flags) {
    Conversion_نتیجہ نتیجہ_2 = conversionOK;
    ثابت UTF8* ماخذ_2 = *ماخذ_آغاز;
    UTF16* ہدف = *ہدف_آغاز;
    جب (ماخذ_2 < ماخذ_انتہا) {
        UTF32 ch = 0;
        غیرعلامتی مختصر extra_بائٹیں_مقصد_Read = trailing_بائٹیں_For_UTF8[*ماخذ_2];
        اگر (extra_بائٹیں_مقصد_Read >= ماخذ_انتہا - ماخذ_2) {
            نتیجہ_2 = ماخذ_Exhausted; توڑو;
        }
        /* Do this check whether lenient or strict */
        اگر (!isLegalUTF8(ماخذ_2, extra_بائٹیں_مقصد_Read+1)) {
            نتیجہ_2 = ماخذ_Illegal;
            توڑو;
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        انتخاب (extra_بائٹیں_مقصد_Read) {
            حالت 5: ch += *ماخذ_2++; ch <<= 6; /* remember, illegal UTF-8 */
            حالت 4: ch += *ماخذ_2++; ch <<= 6; /* remember, illegal UTF-8 */
            حالت 3: ch += *ماخذ_2++; ch <<= 6;
            حالت 2: ch += *ماخذ_2++; ch <<= 6;
            حالت 1: ch += *ماخذ_2++; ch <<= 6;
            حالت 0: ch += *ماخذ_2++;
        }
        ch -= offsetsFromUTF8[extra_بائٹیں_مقصد_Read];

        اگر (ہدف >= ہدف_انتہا) {
            ماخذ_2 -= (extra_بائٹیں_مقصد_Read+1); /* Back up source pointer! */
            نتیجہ_2 = ہدف_Exhausted; توڑو;
        }
        اگر (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32 */
            اگر (ch >= UNI_SUR_بلند_آغاز && ch <= UNI_SUR_کم_انتہا) {
                اگر (flags == strictConversion) {
                    ماخذ_2 -= (extra_بائٹیں_مقصد_Read+1); /* return to the illegal value itself */
                    نتیجہ_2 = ماخذ_Illegal;
                    توڑو;
                } ورنہ {
                    *ہدف++ = UNI_REPLACEMENT_CHAR;
                }
            } ورنہ {
                *ہدف++ = (UTF16)ch; /* normal case */
            }
        } ورنہ اگر (ch > UNI_MAX_UTF16) {
            اگر (flags == strictConversion) {
                نتیجہ_2 = ماخذ_Illegal;
                ماخذ_2 -= (extra_بائٹیں_مقصد_Read+1); /* return to the start */
                توڑو; /* Bail out; shouldn't continue */
            } ورنہ {
                *ہدف++ = UNI_REPLACEMENT_CHAR;
            }
        } ورنہ {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            اگر (ہدف + 1 >= ہدف_انتہا) {
                ماخذ_2 -= (extra_بائٹیں_مقصد_Read+1); /* Back up source pointer! */
                نتیجہ_2 = ہدف_Exhausted; توڑو;
            }
            ch -= half_بنیاد;
            *ہدف++ = (UTF16)((ch >> half_سرکاؤ) + UNI_SUR_بلند_آغاز);
            *ہدف++ = (UTF16)((ch & half_نقاب) + UNI_SUR_کم_آغاز);
        }
    }
    *ماخذ_آغاز = ماخذ_2;
    *ہدف_آغاز = ہدف;
    واپس نتیجہ_2;
}

/* --------------------------------------------------------------------- */

ساکن Conversion_نتیجہ ConvertUTF8toUTF32Impl(
        ثابت UTF8** ماخذ_آغاز, ثابت UTF8* ماخذ_انتہا,
        UTF32** ہدف_آغاز, UTF32* ہدف_انتہا, ConversionFlags flags,
        Boolean داخلہ_Is_Partial) {
    Conversion_نتیجہ نتیجہ_2 = conversionOK;
    ثابت UTF8* ماخذ_2 = *ماخذ_آغاز;
    UTF32* ہدف = *ہدف_آغاز;
    جب (ماخذ_2 < ماخذ_انتہا) {
        UTF32 ch = 0;
        غیرعلامتی مختصر extra_بائٹیں_مقصد_Read = trailing_بائٹیں_For_UTF8[*ماخذ_2];
        اگر (extra_بائٹیں_مقصد_Read >= ماخذ_انتہا - ماخذ_2) {
            اگر (flags == strictConversion || داخلہ_Is_Partial) {
                نتیجہ_2 = ماخذ_Exhausted;
                توڑو;
            } ورنہ {
                نتیجہ_2 = ماخذ_Illegal;

                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                ماخذ_2 += ڈھونڈو_Maximal_Subpart_کا_Ill_Formed_UTF8_Sequence(ماخذ_2,
                                                                    ماخذ_انتہا);
                *ہدف++ = UNI_REPLACEMENT_CHAR;
                جاری;
            }
        }
        اگر (ہدف >= ہدف_انتہا) {
            نتیجہ_2 = ہدف_Exhausted; توڑو;
        }

        /* Do this check whether lenient or strict */
        اگر (!isLegalUTF8(ماخذ_2, extra_بائٹیں_مقصد_Read+1)) {
            نتیجہ_2 = ماخذ_Illegal;
            اگر (flags == strictConversion) {
                /* Abort conversion. */
                توڑو;
            } ورنہ {
                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                ماخذ_2 += ڈھونڈو_Maximal_Subpart_کا_Ill_Formed_UTF8_Sequence(ماخذ_2,
                                                                    ماخذ_انتہا);
                *ہدف++ = UNI_REPLACEMENT_CHAR;
                جاری;
            }
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        انتخاب (extra_بائٹیں_مقصد_Read) {
            حالت 5: ch += *ماخذ_2++; ch <<= 6;
            حالت 4: ch += *ماخذ_2++; ch <<= 6;
            حالت 3: ch += *ماخذ_2++; ch <<= 6;
            حالت 2: ch += *ماخذ_2++; ch <<= 6;
            حالت 1: ch += *ماخذ_2++; ch <<= 6;
            حالت 0: ch += *ماخذ_2++;
        }
        ch -= offsetsFromUTF8[extra_بائٹیں_مقصد_Read];

        اگر (ch <= UNI_MAX_LEGAL_UTF32) {
            /*
             * UTF-16 surrogate values are illegal in UTF-32, and anything
             * over Plane 17 (> 0x10FFFF) is illegal.
             */
            اگر (ch >= UNI_SUR_بلند_آغاز && ch <= UNI_SUR_کم_انتہا) {
                اگر (flags == strictConversion) {
                    ماخذ_2 -= (extra_بائٹیں_مقصد_Read+1); /* return to the illegal value itself */
                    نتیجہ_2 = ماخذ_Illegal;
                    توڑو;
                } ورنہ {
                    *ہدف++ = UNI_REPLACEMENT_CHAR;
                }
            } ورنہ {
                *ہدف++ = ch;
            }
        } ورنہ { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
            نتیجہ_2 = ماخذ_Illegal;
            *ہدف++ = UNI_REPLACEMENT_CHAR;
        }
    }
    *ماخذ_آغاز = ماخذ_2;
    *ہدف_آغاز = ہدف;
    واپس نتیجہ_2;
}

Conversion_نتیجہ ConvertUTF8toUTF32Partial(ثابت UTF8 **ماخذ_آغاز,
                                           ثابت UTF8 *ماخذ_انتہا,
                                           UTF32 **ہدف_آغاز,
                                           UTF32 *ہدف_انتہا,
                                           ConversionFlags flags) {
  واپس ConvertUTF8toUTF32Impl(ماخذ_آغاز, ماخذ_انتہا, ہدف_آغاز, ہدف_انتہا,
                                flags, /*InputIsPartial=*/سچ);
}

Conversion_نتیجہ ConvertUTF8toUTF32(ثابت UTF8 **ماخذ_آغاز,
                                    ثابت UTF8 *ماخذ_انتہا, UTF32 **ہدف_آغاز,
                                    UTF32 *ہدف_انتہا, ConversionFlags flags) {
  واپس ConvertUTF8toUTF32Impl(ماخذ_آغاز, ماخذ_انتہا, ہدف_آغاز, ہدف_انتہا,
                                flags, /*InputIsPartial=*/جھوٹ);
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

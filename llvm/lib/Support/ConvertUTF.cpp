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

#ಅನ್ತರ್ಭಾವ "llvm/Support/ConvertUTF.h"
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CVTUTF_DEBUG
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <assert.h>

/*
 * This code extensively uses fall-through switches.
 * Keep the compiler from warning about that.
 */
#ಆದರೆ defined(__clang__) && defined(__has_warning)
# ಆದರೆ __has_warning("-Wimplicit-fallthrough")
#  define ConvertUTF_DISABLE_WARNINGS \
    _Pragma("clang diagnostic push")  \
    _Pragma("clang diagnostic ignored \"-Wimplicit-fallthrough\"")
#  define ConvertUTF_RESTORE_WARNINGS \
    _Pragma("clang diagnostic pop")
# endif
#ಅಥವಾಆದರೆ defined(__GNUC__)
# define ConvertUTF_DISABLE_WARNINGS \
   _Pragma("GCC diagnostic push")    \
   _Pragma("GCC diagnostic ignored \"-Wimplicit-fallthrough\"")
# define ConvertUTF_RESTORE_WARNINGS \
   _Pragma("GCC diagnostic pop")
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ConvertUTF_DISABLE_WARNINGS
# define ConvertUTF_DISABLE_WARNINGS
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ConvertUTF_RESTORE_WARNINGS
# define ConvertUTF_RESTORE_WARNINGS
#ಆದರೆ_ಅಂತ್ಯ

ConvertUTF_DISABLE_WARNINGS

ನಾಮಾಕಾಶ llvm {

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ half_ಸರಿಸು  = 10; /* used for shifting by 10 bits */

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ UTF32 half_ತಳ = 0x0010000UL;
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ UTF32 half_ಮುಖವಾಡ = 0x3FFUL;

#ವ್ಯಾಖ್ಯಾನಿಸು UNI_SUR_ಮೇಲಿನ_ಆರಂಭ  (UTF32)0xD800
#ವ್ಯಾಖ್ಯಾನಿಸು UNI_SUR_ಮೇಲಿನ_ಕೊನೆ    (UTF32)0xDBFF
#ವ್ಯಾಖ್ಯಾನಿಸು UNI_SUR_ಕೆಳಗಿನ_ಆರಂಭ   (UTF32)0xDC00
#ವ್ಯಾಖ್ಯಾನಿಸು UNI_SUR_ಕೆಳಗಿನ_ಕೊನೆ     (UTF32)0xDFFF

/* --------------------------------------------------------------------- */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ trailing_ಬೈಟುಗಳು_For_UTF8[256] = {
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
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
                     0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ UTF8 ಮೊದಲ_Byte_Mark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */


/* --------------------------------------------------------------------- */

Conversion_ಫಲಿತಾಂಶ ConvertUTF32toUTF16 (
        ಸ್ಥಿರಾಂಕ UTF32** ಮೂಲ_ಆರಂಭ, ಸ್ಥಿರಾಂಕ UTF32* ಮೂಲ_ಕೊನೆ,
        UTF16** ಗುರಿ_ಆರಂಭ, UTF16* ಗುರಿ_ಕೊನೆ, ConversionFlags flags) {
    Conversion_ಫಲಿತಾಂಶ ಫಲಿತಾಂಶ_2 = conversionOK;
    ಸ್ಥಿರಾಂಕ UTF32* ಮೂಲ_2 = *ಮೂಲ_ಆರಂಭ;
    UTF16* ಗುರಿ = *ಗುರಿ_ಆರಂಭ;
    ಆಗ (ಮೂಲ_2 < ಮೂಲ_ಕೊನೆ) {
        UTF32 ch;
        ಆದರೆ (ಗುರಿ >= ಗುರಿ_ಕೊನೆ) {
            ಫಲಿತಾಂಶ_2 = ಗುರಿ_Exhausted; ನಿಲ್ಲಿಸು;
        }
        ch = *ಮೂಲ_2++;
        ಆದರೆ (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
            ಆದರೆ (ch >= UNI_SUR_ಮೇಲಿನ_ಆರಂಭ && ch <= UNI_SUR_ಕೆಳಗಿನ_ಕೊನೆ) {
                ಆದರೆ (flags == strictConversion) {
                    --ಮೂಲ_2; /* return to the illegal value itself */
                    ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;
                    ನಿಲ್ಲಿಸು;
                } ಇಲ್ಲದಿದ್ದರೆ {
                    *ಗುರಿ++ = UNI_REPLACEMENT_CHAR;
                }
            } ಇಲ್ಲದಿದ್ದರೆ {
                *ಗುರಿ++ = (UTF16)ch; /* normal case */
            }
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ch > UNI_MAX_LEGAL_UTF32) {
            ಆದರೆ (flags == strictConversion) {
                ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;
            } ಇಲ್ಲದಿದ್ದರೆ {
                *ಗುರಿ++ = UNI_REPLACEMENT_CHAR;
            }
        } ಇಲ್ಲದಿದ್ದರೆ {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            ಆದರೆ (ಗುರಿ + 1 >= ಗುರಿ_ಕೊನೆ) {
                --ಮೂಲ_2; /* Back up source pointer! */
                ಫಲಿತಾಂಶ_2 = ಗುರಿ_Exhausted; ನಿಲ್ಲಿಸು;
            }
            ch -= half_ತಳ;
            *ಗುರಿ++ = (UTF16)((ch >> half_ಸರಿಸು) + UNI_SUR_ಮೇಲಿನ_ಆರಂಭ);
            *ಗುರಿ++ = (UTF16)((ch & half_ಮುಖವಾಡ) + UNI_SUR_ಕೆಳಗಿನ_ಆರಂಭ);
        }
    }
    *ಮೂಲ_ಆರಂಭ = ಮೂಲ_2;
    *ಗುರಿ_ಆರಂಭ = ಗುರಿ;
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ_2;
}

/* --------------------------------------------------------------------- */

Conversion_ಫಲಿತಾಂಶ ConvertUTF16toUTF32 (
        ಸ್ಥಿರಾಂಕ UTF16** ಮೂಲ_ಆರಂಭ, ಸ್ಥಿರಾಂಕ UTF16* ಮೂಲ_ಕೊನೆ,
        UTF32** ಗುರಿ_ಆರಂಭ, UTF32* ಗುರಿ_ಕೊನೆ, ConversionFlags flags) {
    Conversion_ಫಲಿತಾಂಶ ಫಲಿತಾಂಶ_2 = conversionOK;
    ಸ್ಥಿರಾಂಕ UTF16* ಮೂಲ_2 = *ಮೂಲ_ಆರಂಭ;
    UTF32* ಗುರಿ = *ಗುರಿ_ಆರಂಭ;
    UTF32 ch, ch2;
    ಆಗ (ಮೂಲ_2 < ಮೂಲ_ಕೊನೆ) {
        ಸ್ಥಿರಾಂಕ UTF16* old_ಮೂಲ = ಮೂಲ_2; /*  In case we have to back up because of target overflow. */
        ch = *ಮೂಲ_2++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        ಆದರೆ (ch >= UNI_SUR_ಮೇಲಿನ_ಆರಂಭ && ch <= UNI_SUR_ಮೇಲಿನ_ಕೊನೆ) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            ಆದರೆ (ಮೂಲ_2 < ಮೂಲ_ಕೊನೆ) {
                ch2 = *ಮೂಲ_2;
                /* If it's a low surrogate, convert to UTF32. */
                ಆದರೆ (ch2 >= UNI_SUR_ಕೆಳಗಿನ_ಆರಂಭ && ch2 <= UNI_SUR_ಕೆಳಗಿನ_ಕೊನೆ) {
                    ch = ((ch - UNI_SUR_ಮೇಲಿನ_ಆರಂಭ) << half_ಸರಿಸು)
                        + (ch2 - UNI_SUR_ಕೆಳಗಿನ_ಆರಂಭ) + half_ತಳ;
                    ++ಮೂಲ_2;
                } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --ಮೂಲ_2; /* return to the illegal value itself */
                    ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;
                    ನಿಲ್ಲಿಸು;
                }
            } ಇಲ್ಲದಿದ್ದರೆ { /* We don't have the 16 bits following the high surrogate. */
                --ಮೂಲ_2; /* return to the high surrogate */
                ಫಲಿತಾಂಶ_2 = ಮೂಲ_Exhausted;
                ನಿಲ್ಲಿಸು;
            }
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            ಆದರೆ (ch >= UNI_SUR_ಕೆಳಗಿನ_ಆರಂಭ && ch <= UNI_SUR_ಕೆಳಗಿನ_ಕೊನೆ) {
                --ಮೂಲ_2; /* return to the illegal value itself */
                ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;
                ನಿಲ್ಲಿಸು;
            }
        }
        ಆದರೆ (ಗುರಿ >= ಗುರಿ_ಕೊನೆ) {
            ಮೂಲ_2 = old_ಮೂಲ; /* Back up source pointer! */
            ಫಲಿತಾಂಶ_2 = ಗುರಿ_Exhausted; ನಿಲ್ಲಿಸು;
        }
        *ಗುರಿ++ = ch;
    }
    *ಮೂಲ_ಆರಂಭ = ಮೂಲ_2;
    *ಗುರಿ_ಆರಂಭ = ಗುರಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CVTUTF_DEBUG
ಆದರೆ (ಫಲಿತಾಂಶ_2 == ಮೂಲ_Illegal) {
    fprintf(stderr, "ConvertUTF16toUTF32 illegal seq 0x%04x,%04x\n", ch, ch2);
    fflush(stderr);
}
#ಆದರೆ_ಅಂತ್ಯ
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ_2;
}
Conversion_ಫಲಿತಾಂಶ ConvertUTF16toUTF8 (
        ಸ್ಥಿರಾಂಕ UTF16** ಮೂಲ_ಆರಂಭ, ಸ್ಥಿರಾಂಕ UTF16* ಮೂಲ_ಕೊನೆ,
        UTF8** ಗುರಿ_ಆರಂಭ, UTF8* ಗುರಿ_ಕೊನೆ, ConversionFlags flags) {
    Conversion_ಫಲಿತಾಂಶ ಫಲಿತಾಂಶ_2 = conversionOK;
    ಸ್ಥಿರಾಂಕ UTF16* ಮೂಲ_2 = *ಮೂಲ_ಆರಂಭ;
    UTF8* ಗುರಿ = *ಗುರಿ_ಆರಂಭ;
    ಆಗ (ಮೂಲ_2 < ಮೂಲ_ಕೊನೆ) {
        UTF32 ch;
        ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ಬೈಟುಗಳು_ಗಮ್ಯ_Write = 0;
        ಸ್ಥಿರಾಂಕ UTF32 byte_ಮುಖವಾಡ = 0xBF;
        ಸ್ಥಿರಾಂಕ UTF32 byteMark = 0x80;
        ಸ್ಥಿರಾಂಕ UTF16* old_ಮೂಲ = ಮೂಲ_2; /* In case we have to back up because of target overflow. */
        ch = *ಮೂಲ_2++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        ಆದರೆ (ch >= UNI_SUR_ಮೇಲಿನ_ಆರಂಭ && ch <= UNI_SUR_ಮೇಲಿನ_ಕೊನೆ) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            ಆದರೆ (ಮೂಲ_2 < ಮೂಲ_ಕೊನೆ) {
                UTF32 ch2 = *ಮೂಲ_2;
                /* If it's a low surrogate, convert to UTF32. */
                ಆದರೆ (ch2 >= UNI_SUR_ಕೆಳಗಿನ_ಆರಂಭ && ch2 <= UNI_SUR_ಕೆಳಗಿನ_ಕೊನೆ) {
                    ch = ((ch - UNI_SUR_ಮೇಲಿನ_ಆರಂಭ) << half_ಸರಿಸು)
                        + (ch2 - UNI_SUR_ಕೆಳಗಿನ_ಆರಂಭ) + half_ತಳ;
                    ++ಮೂಲ_2;
                } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --ಮೂಲ_2; /* return to the illegal value itself */
                    ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;
                    ನಿಲ್ಲಿಸು;
                }
            } ಇಲ್ಲದಿದ್ದರೆ { /* We don't have the 16 bits following the high surrogate. */
                --ಮೂಲ_2; /* return to the high surrogate */
                ಫಲಿತಾಂಶ_2 = ಮೂಲ_Exhausted;
                ನಿಲ್ಲಿಸು;
            }
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            ಆದರೆ (ch >= UNI_SUR_ಕೆಳಗಿನ_ಆರಂಭ && ch <= UNI_SUR_ಕೆಳಗಿನ_ಕೊನೆ) {
                --ಮೂಲ_2; /* return to the illegal value itself */
                ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;
                ನಿಲ್ಲಿಸು;
            }
        }
        /* Figure out how many bytes the result will require */
        ಆದರೆ (ch < (UTF32)0x80) {      ಬೈಟುಗಳು_ಗಮ್ಯ_Write = 1;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ch < (UTF32)0x800) {     ಬೈಟುಗಳು_ಗಮ್ಯ_Write = 2;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ch < (UTF32)0x10000) {   ಬೈಟುಗಳು_ಗಮ್ಯ_Write = 3;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ch < (UTF32)0x110000) {  ಬೈಟುಗಳು_ಗಮ್ಯ_Write = 4;
        } ಇಲ್ಲದಿದ್ದರೆ {                            ಬೈಟುಗಳು_ಗಮ್ಯ_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
        }

        ಗುರಿ += ಬೈಟುಗಳು_ಗಮ್ಯ_Write;
        ಆದರೆ (ಗುರಿ > ಗುರಿ_ಕೊನೆ) {
            ಮೂಲ_2 = old_ಮೂಲ; /* Back up source pointer! */
            ಗುರಿ -= ಬೈಟುಗಳು_ಗಮ್ಯ_Write; ಫಲಿತಾಂಶ_2 = ಗುರಿ_Exhausted; ನಿಲ್ಲಿಸು;
        }
        ಆಯ್ಕೆ (ಬೈಟುಗಳು_ಗಮ್ಯ_Write) { /* note: everything falls through. */
            ಸ್ಥಿತಿ 4: *--ಗುರಿ = (UTF8)((ch | byteMark) & byte_ಮುಖವಾಡ); ch >>= 6;
            ಸ್ಥಿತಿ 3: *--ಗುರಿ = (UTF8)((ch | byteMark) & byte_ಮುಖವಾಡ); ch >>= 6;
            ಸ್ಥಿತಿ 2: *--ಗುರಿ = (UTF8)((ch | byteMark) & byte_ಮುಖವಾಡ); ch >>= 6;
            ಸ್ಥಿತಿ 1: *--ಗುರಿ =  (UTF8)(ch | ಮೊದಲ_Byte_Mark[ಬೈಟುಗಳು_ಗಮ್ಯ_Write]);
        }
        ಗುರಿ += ಬೈಟುಗಳು_ಗಮ್ಯ_Write;
    }
    *ಮೂಲ_ಆರಂಭ = ಮೂಲ_2;
    *ಗುರಿ_ಆರಂಭ = ಗುರಿ;
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ_2;
}

/* --------------------------------------------------------------------- */

Conversion_ಫಲಿತಾಂಶ ConvertUTF32toUTF8 (
        ಸ್ಥಿರಾಂಕ UTF32** ಮೂಲ_ಆರಂಭ, ಸ್ಥಿರಾಂಕ UTF32* ಮೂಲ_ಕೊನೆ,
        UTF8** ಗುರಿ_ಆರಂಭ, UTF8* ಗುರಿ_ಕೊನೆ, ConversionFlags flags) {
    Conversion_ಫಲಿತಾಂಶ ಫಲಿತಾಂಶ_2 = conversionOK;
    ಸ್ಥಿರಾಂಕ UTF32* ಮೂಲ_2 = *ಮೂಲ_ಆರಂಭ;
    UTF8* ಗುರಿ = *ಗುರಿ_ಆರಂಭ;
    ಆಗ (ಮೂಲ_2 < ಮೂಲ_ಕೊನೆ) {
        UTF32 ch;
        ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ಬೈಟುಗಳು_ಗಮ್ಯ_Write = 0;
        ಸ್ಥಿರಾಂಕ UTF32 byte_ಮುಖವಾಡ = 0xBF;
        ಸ್ಥಿರಾಂಕ UTF32 byteMark = 0x80;
        ch = *ಮೂಲ_2++;
        ಆದರೆ (flags == strictConversion ) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            ಆದರೆ (ch >= UNI_SUR_ಮೇಲಿನ_ಆರಂಭ && ch <= UNI_SUR_ಕೆಳಗಿನ_ಕೊನೆ) {
                --ಮೂಲ_2; /* return to the illegal value itself */
                ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;
                ನಿಲ್ಲಿಸು;
            }
        }
        /*
         * Figure out how many bytes the result will require. Turn any
         * illegally large UTF32 things (> Plane 17) into replacement chars.
         */
        ಆದರೆ (ch < (UTF32)0x80) {      ಬೈಟುಗಳು_ಗಮ್ಯ_Write = 1;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ch < (UTF32)0x800) {     ಬೈಟುಗಳು_ಗಮ್ಯ_Write = 2;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ch < (UTF32)0x10000) {   ಬೈಟುಗಳು_ಗಮ್ಯ_Write = 3;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ch <= UNI_MAX_LEGAL_UTF32) {  ಬೈಟುಗಳು_ಗಮ್ಯ_Write = 4;
        } ಇಲ್ಲದಿದ್ದರೆ {                            ಬೈಟುಗಳು_ಗಮ್ಯ_Write = 3;
                                            ch = UNI_REPLACEMENT_CHAR;
                                            ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;
        }

        ಗುರಿ += ಬೈಟುಗಳು_ಗಮ್ಯ_Write;
        ಆದರೆ (ಗುರಿ > ಗುರಿ_ಕೊನೆ) {
            --ಮೂಲ_2; /* Back up source pointer! */
            ಗುರಿ -= ಬೈಟುಗಳು_ಗಮ್ಯ_Write; ಫಲಿತಾಂಶ_2 = ಗುರಿ_Exhausted; ನಿಲ್ಲಿಸು;
        }
        ಆಯ್ಕೆ (ಬೈಟುಗಳು_ಗಮ್ಯ_Write) { /* note: everything falls through. */
            ಸ್ಥಿತಿ 4: *--ಗುರಿ = (UTF8)((ch | byteMark) & byte_ಮುಖವಾಡ); ch >>= 6;
            ಸ್ಥಿತಿ 3: *--ಗುರಿ = (UTF8)((ch | byteMark) & byte_ಮುಖವಾಡ); ch >>= 6;
            ಸ್ಥಿತಿ 2: *--ಗುರಿ = (UTF8)((ch | byteMark) & byte_ಮುಖವಾಡ); ch >>= 6;
            ಸ್ಥಿತಿ 1: *--ಗುರಿ = (UTF8) (ch | ಮೊದಲ_Byte_Mark[ಬೈಟುಗಳು_ಗಮ್ಯ_Write]);
        }
        ಗುರಿ += ಬೈಟುಗಳು_ಗಮ್ಯ_Write;
    }
    *ಮೂಲ_ಆರಂಭ = ಮೂಲ_2;
    *ಗುರಿ_ಆರಂಭ = ಗುರಿ;
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ_2;
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

ಸ್ಥಿರ Boolean isLegalUTF8(ಸ್ಥಿರಾಂಕ UTF8 *ಮೂಲ_2, ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ_2) {
    UTF8 a;
    ಸ್ಥಿರಾಂಕ UTF8 *srcptr = ಮೂಲ_2+ಉದ್ದಳತೆ_2;
    ಆಯ್ಕೆ (ಉದ್ದಳತೆ_2) {
    ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
        /* Everything else falls through when "true"... */
    ಸ್ಥಿತಿ 4: ಆದರೆ ((a = (*--srcptr)) < 0x80 || a > 0xBF) ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    ಸ್ಥಿತಿ 3: ಆದರೆ ((a = (*--srcptr)) < 0x80 || a > 0xBF) ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    ಸ್ಥಿತಿ 2: ಆದರೆ ((a = (*--srcptr)) < 0x80 || a > 0xBF) ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

        ಆಯ್ಕೆ (*ಮೂಲ_2) {
            /* no fall-through in this inner switch */
            ಸ್ಥಿತಿ 0xE0: ಆದರೆ (a < 0xA0) ಹಿಂತಿರುಗಿ ಸುಳ್ಳು; ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 0xED: ಆದರೆ (a > 0x9F) ಹಿಂತಿರುಗಿ ಸುಳ್ಳು; ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 0xF0: ಆದರೆ (a < 0x90) ಹಿಂತಿರುಗಿ ಸುಳ್ಳು; ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 0xF4: ಆದರೆ (a > 0x8F) ಹಿಂತಿರುಗಿ ಸುಳ್ಳು; ನಿಲ್ಲಿಸು;
            ಪೂರ್ವನಿಯೋಜಿತ:   ಆದರೆ (a < 0x80) ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
        }

    ಸ್ಥಿತಿ 1: ಆದರೆ (*ಮೂಲ_2 >= 0x80 && *ಮೂಲ_2 < 0xC2) ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }
    ಆದರೆ (*ಮೂಲ_2 > 0xF4) ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    ಹಿಂತಿರುಗಿ ನಿಜ;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 sequence is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8Sequence(ಸ್ಥಿರಾಂಕ UTF8 *ಮೂಲ_2, ಸ್ಥಿರಾಂಕ UTF8 *ಮೂಲ_ಕೊನೆ) {
    ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ_2 = trailing_ಬೈಟುಗಳು_For_UTF8[*ಮೂಲ_2]+1;
    ಆದರೆ (ಉದ್ದಳತೆ_2 > ಮೂಲ_ಕೊನೆ - ಮೂಲ_2) {
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }
    ಹಿಂತಿರುಗಿ isLegalUTF8(ಮೂಲ_2, ಉದ್ದಳತೆ_2);
}

/*
 * Exported function to return the size of the first utf-8 code unit sequence,
 * Or 0 if the sequence is not valid;
 */
ಚಿಹ್ನೆರಹಿತ get_UTF8_Sequence_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ UTF8 *ಮೂಲ_2, ಸ್ಥಿರಾಂಕ UTF8 *ಮೂಲ_ಕೊನೆ) {
  ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ_2 = trailing_ಬೈಟುಗಳು_For_UTF8[*ಮೂಲ_2] + 1;
  ಹಿಂತಿರುಗಿ (ಉದ್ದಳತೆ_2 <= ಮೂಲ_ಕೊನೆ - ಮೂಲ_2 && isLegalUTF8(ಮೂಲ_2, ಉದ್ದಳತೆ_2)) ? ಉದ್ದಳತೆ_2
                                                                       : 0;
}

/* --------------------------------------------------------------------- */

ಚಿಹ್ನೆರಹಿತ ಕಂಡುಹಿಡಿ_Maximal_Subpart_ದ_Ill_Formed_UTF8_Sequence(ಸ್ಥಿರಾಂಕ UTF8 *ಮೂಲ_2,
                                                   ಸ್ಥಿರಾಂಕ UTF8 *ಮೂಲ_ಕೊನೆ) {
  UTF8 b1, b2, b3;

  ಪ್ರತಿಜ್ಞಾ(!isLegalUTF8Sequence(ಮೂಲ_2, ಮೂಲ_ಕೊನೆ));

  /*
   * Unicode 6.3.0, D93b:
   *
   *   Maximal subpart of an ill-formed subsequence: The longest code unit
   *   subsequence starting at an unconvertible offset that is either:
   *   a. the initial subsequence of a well-formed code unit sequence, or
   *   b. a subsequence of length one.
   */

  ಆದರೆ (ಮೂಲ_2 == ಮೂಲ_ಕೊನೆ)
    ಹಿಂತಿರುಗಿ 0;

  /*
   * Perform case analysis.  See Unicode 6.3.0, Table 3-7. Well-Formed UTF-8
   * Byte Sequences.
   */

  b1 = *ಮೂಲ_2;
  ++ಮೂಲ_2;
  ಆದರೆ (b1 >= 0xC2 && b1 <= 0xDF) {
    /*
     * First byte is valid, but we know that this code unit sequence is
     * invalid, so the maximal subpart has to end after the first byte.
     */
    ಹಿಂತಿರುಗಿ 1;
  }

  ಆದರೆ (ಮೂಲ_2 == ಮೂಲ_ಕೊನೆ)
    ಹಿಂತಿರುಗಿ 1;

  b2 = *ಮೂಲ_2;
  ++ಮೂಲ_2;

  ಆದರೆ (b1 == 0xE0) {
    ಹಿಂತಿರುಗಿ (b2 >= 0xA0 && b2 <= 0xBF) ? 2 : 1;
  }
  ಆದರೆ (b1 >= 0xE1 && b1 <= 0xEC) {
    ಹಿಂತಿರುಗಿ (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  ಆದರೆ (b1 == 0xED) {
    ಹಿಂತಿರುಗಿ (b2 >= 0x80 && b2 <= 0x9F) ? 2 : 1;
  }
  ಆದರೆ (b1 >= 0xEE && b1 <= 0xEF) {
    ಹಿಂತಿರುಗಿ (b2 >= 0x80 && b2 <= 0xBF) ? 2 : 1;
  }
  ಆದರೆ (b1 == 0xF0) {
    ಆದರೆ (b2 >= 0x90 && b2 <= 0xBF) {
      ಆದರೆ (ಮೂಲ_2 == ಮೂಲ_ಕೊನೆ)
        ಹಿಂತಿರುಗಿ 2;

      b3 = *ಮೂಲ_2;
      ಹಿಂತಿರುಗಿ (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    ಹಿಂತಿರುಗಿ 1;
  }
  ಆದರೆ (b1 >= 0xF1 && b1 <= 0xF3) {
    ಆದರೆ (b2 >= 0x80 && b2 <= 0xBF) {
      ಆದರೆ (ಮೂಲ_2 == ಮೂಲ_ಕೊನೆ)
        ಹಿಂತಿರುಗಿ 2;

      b3 = *ಮೂಲ_2;
      ಹಿಂತಿರುಗಿ (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    ಹಿಂತಿರುಗಿ 1;
  }
  ಆದರೆ (b1 == 0xF4) {
    ಆದರೆ (b2 >= 0x80 && b2 <= 0x8F) {
      ಆದರೆ (ಮೂಲ_2 == ಮೂಲ_ಕೊನೆ)
        ಹಿಂತಿರುಗಿ 2;

      b3 = *ಮೂಲ_2;
      ಹಿಂತಿರುಗಿ (b3 >= 0x80 && b3 <= 0xBF) ? 3 : 2;
    }
    ಹಿಂತಿರುಗಿ 1;
  }

  ಪ್ರತಿಜ್ಞಾ((b1 >= 0x80 && b1 <= 0xC1) || b1 >= 0xF5);
  /*
   * There are no valid sequences that start with these bytes.  Maximal subpart
   * is defined to have length 1 in these cases.
   */
  ಹಿಂತಿರುಗಿ 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return the total number of bytes in a codepoint
 * represented in UTF-8, given the value of the first byte.
 */
ಚಿಹ್ನೆರಹಿತ get_ಅಂಕೆ_ಬೈಟುಗಳು_For_UTF8(UTF8 ಮೊದಲ_2) {
  ಹಿಂತಿರುಗಿ trailing_ಬೈಟುಗಳು_For_UTF8[ಮೊದಲ_2] + 1;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 string is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8String(ಸ್ಥಿರಾಂಕ UTF8 **ಮೂಲ_2, ಸ್ಥಿರಾಂಕ UTF8 *ಮೂಲ_ಕೊನೆ) {
    ಆಗ (*ಮೂಲ_2 != ಮೂಲ_ಕೊನೆ) {
        ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ_2 = trailing_ಬೈಟುಗಳು_For_UTF8[**ಮೂಲ_2] + 1;
        ಆದರೆ (ಉದ್ದಳತೆ_2 > ಮೂಲ_ಕೊನೆ - *ಮೂಲ_2 || !isLegalUTF8(*ಮೂಲ_2, ಉದ್ದಳತೆ_2))
            ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
        *ಮೂಲ_2 += ಉದ್ದಳತೆ_2;
    }
    ಹಿಂತಿರುಗಿ ನಿಜ;
}

/* --------------------------------------------------------------------- */

Conversion_ಫಲಿತಾಂಶ ConvertUTF8toUTF16 (
        ಸ್ಥಿರಾಂಕ UTF8** ಮೂಲ_ಆರಂಭ, ಸ್ಥಿರಾಂಕ UTF8* ಮೂಲ_ಕೊನೆ,
        UTF16** ಗುರಿ_ಆರಂಭ, UTF16* ಗುರಿ_ಕೊನೆ, ConversionFlags flags) {
    Conversion_ಫಲಿತಾಂಶ ಫಲಿತಾಂಶ_2 = conversionOK;
    ಸ್ಥಿರಾಂಕ UTF8* ಮೂಲ_2 = *ಮೂಲ_ಆರಂಭ;
    UTF16* ಗುರಿ = *ಗುರಿ_ಆರಂಭ;
    ಆಗ (ಮೂಲ_2 < ಮೂಲ_ಕೊನೆ) {
        UTF32 ch = 0;
        ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ extra_ಬೈಟುಗಳು_ಗಮ್ಯ_Read = trailing_ಬೈಟುಗಳು_For_UTF8[*ಮೂಲ_2];
        ಆದರೆ (extra_ಬೈಟುಗಳು_ಗಮ್ಯ_Read >= ಮೂಲ_ಕೊನೆ - ಮೂಲ_2) {
            ಫಲಿತಾಂಶ_2 = ಮೂಲ_Exhausted; ನಿಲ್ಲಿಸು;
        }
        /* Do this check whether lenient or strict */
        ಆದರೆ (!isLegalUTF8(ಮೂಲ_2, extra_ಬೈಟುಗಳು_ಗಮ್ಯ_Read+1)) {
            ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;
            ನಿಲ್ಲಿಸು;
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        ಆಯ್ಕೆ (extra_ಬೈಟುಗಳು_ಗಮ್ಯ_Read) {
            ಸ್ಥಿತಿ 5: ch += *ಮೂಲ_2++; ch <<= 6; /* remember, illegal UTF-8 */
            ಸ್ಥಿತಿ 4: ch += *ಮೂಲ_2++; ch <<= 6; /* remember, illegal UTF-8 */
            ಸ್ಥಿತಿ 3: ch += *ಮೂಲ_2++; ch <<= 6;
            ಸ್ಥಿತಿ 2: ch += *ಮೂಲ_2++; ch <<= 6;
            ಸ್ಥಿತಿ 1: ch += *ಮೂಲ_2++; ch <<= 6;
            ಸ್ಥಿತಿ 0: ch += *ಮೂಲ_2++;
        }
        ch -= offsetsFromUTF8[extra_ಬೈಟುಗಳು_ಗಮ್ಯ_Read];

        ಆದರೆ (ಗುರಿ >= ಗುರಿ_ಕೊನೆ) {
            ಮೂಲ_2 -= (extra_ಬೈಟುಗಳು_ಗಮ್ಯ_Read+1); /* Back up source pointer! */
            ಫಲಿತಾಂಶ_2 = ಗುರಿ_Exhausted; ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32 */
            ಆದರೆ (ch >= UNI_SUR_ಮೇಲಿನ_ಆರಂಭ && ch <= UNI_SUR_ಕೆಳಗಿನ_ಕೊನೆ) {
                ಆದರೆ (flags == strictConversion) {
                    ಮೂಲ_2 -= (extra_ಬೈಟುಗಳು_ಗಮ್ಯ_Read+1); /* return to the illegal value itself */
                    ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;
                    ನಿಲ್ಲಿಸು;
                } ಇಲ್ಲದಿದ್ದರೆ {
                    *ಗುರಿ++ = UNI_REPLACEMENT_CHAR;
                }
            } ಇಲ್ಲದಿದ್ದರೆ {
                *ಗುರಿ++ = (UTF16)ch; /* normal case */
            }
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ch > UNI_MAX_UTF16) {
            ಆದರೆ (flags == strictConversion) {
                ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;
                ಮೂಲ_2 -= (extra_ಬೈಟುಗಳು_ಗಮ್ಯ_Read+1); /* return to the start */
                ನಿಲ್ಲಿಸು; /* Bail out; shouldn't continue */
            } ಇಲ್ಲದಿದ್ದರೆ {
                *ಗುರಿ++ = UNI_REPLACEMENT_CHAR;
            }
        } ಇಲ್ಲದಿದ್ದರೆ {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            ಆದರೆ (ಗುರಿ + 1 >= ಗುರಿ_ಕೊನೆ) {
                ಮೂಲ_2 -= (extra_ಬೈಟುಗಳು_ಗಮ್ಯ_Read+1); /* Back up source pointer! */
                ಫಲಿತಾಂಶ_2 = ಗುರಿ_Exhausted; ನಿಲ್ಲಿಸು;
            }
            ch -= half_ತಳ;
            *ಗುರಿ++ = (UTF16)((ch >> half_ಸರಿಸು) + UNI_SUR_ಮೇಲಿನ_ಆರಂಭ);
            *ಗುರಿ++ = (UTF16)((ch & half_ಮುಖವಾಡ) + UNI_SUR_ಕೆಳಗಿನ_ಆರಂಭ);
        }
    }
    *ಮೂಲ_ಆರಂಭ = ಮೂಲ_2;
    *ಗುರಿ_ಆರಂಭ = ಗುರಿ;
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ_2;
}

/* --------------------------------------------------------------------- */

ಸ್ಥಿರ Conversion_ಫಲಿತಾಂಶ ConvertUTF8toUTF32Impl(
        ಸ್ಥಿರಾಂಕ UTF8** ಮೂಲ_ಆರಂಭ, ಸ್ಥಿರಾಂಕ UTF8* ಮೂಲ_ಕೊನೆ,
        UTF32** ಗುರಿ_ಆರಂಭ, UTF32* ಗುರಿ_ಕೊನೆ, ConversionFlags flags,
        Boolean ಒಳಹರಿವು_Is_Partial) {
    Conversion_ಫಲಿತಾಂಶ ಫಲಿತಾಂಶ_2 = conversionOK;
    ಸ್ಥಿರಾಂಕ UTF8* ಮೂಲ_2 = *ಮೂಲ_ಆರಂಭ;
    UTF32* ಗುರಿ = *ಗುರಿ_ಆರಂಭ;
    ಆಗ (ಮೂಲ_2 < ಮೂಲ_ಕೊನೆ) {
        UTF32 ch = 0;
        ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ extra_ಬೈಟುಗಳು_ಗಮ್ಯ_Read = trailing_ಬೈಟುಗಳು_For_UTF8[*ಮೂಲ_2];
        ಆದರೆ (extra_ಬೈಟುಗಳು_ಗಮ್ಯ_Read >= ಮೂಲ_ಕೊನೆ - ಮೂಲ_2) {
            ಆದರೆ (flags == strictConversion || ಒಳಹರಿವು_Is_Partial) {
                ಫಲಿತಾಂಶ_2 = ಮೂಲ_Exhausted;
                ನಿಲ್ಲಿಸು;
            } ಇಲ್ಲದಿದ್ದರೆ {
                ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;

                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                ಮೂಲ_2 += ಕಂಡುಹಿಡಿ_Maximal_Subpart_ದ_Ill_Formed_UTF8_Sequence(ಮೂಲ_2,
                                                                    ಮೂಲ_ಕೊನೆ);
                *ಗುರಿ++ = UNI_REPLACEMENT_CHAR;
                ಮುಂದುವರಿ;
            }
        }
        ಆದರೆ (ಗುರಿ >= ಗುರಿ_ಕೊನೆ) {
            ಫಲಿತಾಂಶ_2 = ಗುರಿ_Exhausted; ನಿಲ್ಲಿಸು;
        }

        /* Do this check whether lenient or strict */
        ಆದರೆ (!isLegalUTF8(ಮೂಲ_2, extra_ಬೈಟುಗಳು_ಗಮ್ಯ_Read+1)) {
            ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;
            ಆದರೆ (flags == strictConversion) {
                /* Abort conversion. */
                ನಿಲ್ಲಿಸು;
            } ಇಲ್ಲದಿದ್ದರೆ {
                /*
                 * Replace the maximal subpart of ill-formed sequence with
                 * replacement character.
                 */
                ಮೂಲ_2 += ಕಂಡುಹಿಡಿ_Maximal_Subpart_ದ_Ill_Formed_UTF8_Sequence(ಮೂಲ_2,
                                                                    ಮೂಲ_ಕೊನೆ);
                *ಗುರಿ++ = UNI_REPLACEMENT_CHAR;
                ಮುಂದುವರಿ;
            }
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        ಆಯ್ಕೆ (extra_ಬೈಟುಗಳು_ಗಮ್ಯ_Read) {
            ಸ್ಥಿತಿ 5: ch += *ಮೂಲ_2++; ch <<= 6;
            ಸ್ಥಿತಿ 4: ch += *ಮೂಲ_2++; ch <<= 6;
            ಸ್ಥಿತಿ 3: ch += *ಮೂಲ_2++; ch <<= 6;
            ಸ್ಥಿತಿ 2: ch += *ಮೂಲ_2++; ch <<= 6;
            ಸ್ಥಿತಿ 1: ch += *ಮೂಲ_2++; ch <<= 6;
            ಸ್ಥಿತಿ 0: ch += *ಮೂಲ_2++;
        }
        ch -= offsetsFromUTF8[extra_ಬೈಟುಗಳು_ಗಮ್ಯ_Read];

        ಆದರೆ (ch <= UNI_MAX_LEGAL_UTF32) {
            /*
             * UTF-16 surrogate values are illegal in UTF-32, and anything
             * over Plane 17 (> 0x10FFFF) is illegal.
             */
            ಆದರೆ (ch >= UNI_SUR_ಮೇಲಿನ_ಆರಂಭ && ch <= UNI_SUR_ಕೆಳಗಿನ_ಕೊನೆ) {
                ಆದರೆ (flags == strictConversion) {
                    ಮೂಲ_2 -= (extra_ಬೈಟುಗಳು_ಗಮ್ಯ_Read+1); /* return to the illegal value itself */
                    ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;
                    ನಿಲ್ಲಿಸು;
                } ಇಲ್ಲದಿದ್ದರೆ {
                    *ಗುರಿ++ = UNI_REPLACEMENT_CHAR;
                }
            } ಇಲ್ಲದಿದ್ದರೆ {
                *ಗುರಿ++ = ch;
            }
        } ಇಲ್ಲದಿದ್ದರೆ { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
            ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;
            *ಗುರಿ++ = UNI_REPLACEMENT_CHAR;
        }
    }
    *ಮೂಲ_ಆರಂಭ = ಮೂಲ_2;
    *ಗುರಿ_ಆರಂಭ = ಗುರಿ;
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ_2;
}

Conversion_ಫಲಿತಾಂಶ ConvertUTF8toUTF32Partial(ಸ್ಥಿರಾಂಕ UTF8 **ಮೂಲ_ಆರಂಭ,
                                           ಸ್ಥಿರಾಂಕ UTF8 *ಮೂಲ_ಕೊನೆ,
                                           UTF32 **ಗುರಿ_ಆರಂಭ,
                                           UTF32 *ಗುರಿ_ಕೊನೆ,
                                           ConversionFlags flags) {
  ಹಿಂತಿರುಗಿ ConvertUTF8toUTF32Impl(ಮೂಲ_ಆರಂಭ, ಮೂಲ_ಕೊನೆ, ಗುರಿ_ಆರಂಭ, ಗುರಿ_ಕೊನೆ,
                                flags, /*InputIsPartial=*/ನಿಜ);
}

Conversion_ಫಲಿತಾಂಶ ConvertUTF8toUTF32(ಸ್ಥಿರಾಂಕ UTF8 **ಮೂಲ_ಆರಂಭ,
                                    ಸ್ಥಿರಾಂಕ UTF8 *ಮೂಲ_ಕೊನೆ, UTF32 **ಗುರಿ_ಆರಂಭ,
                                    UTF32 *ಗುರಿ_ಕೊನೆ, ConversionFlags flags) {
  ಹಿಂತಿರುಗಿ ConvertUTF8toUTF32Impl(ಮೂಲ_ಆರಂಭ, ಮೂಲ_ಕೊನೆ, ಗುರಿ_ಆರಂಭ, ಗುರಿ_ಕೊನೆ,
                                flags, /*InputIsPartial=*/ಸುಳ್ಳು);
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

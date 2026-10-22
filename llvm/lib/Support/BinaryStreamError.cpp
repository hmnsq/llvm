//===- BinaryStreamError.cpp - Error extensions for streams -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/BinaryStreamError.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"

ఉపయోగించి నామాకాశ llvm;

అక్షరం ద్విమాన_ప్రవాహం_Error::ID = 0;

ద్విమాన_ప్రవాహం_Error::ద్విమాన_ప్రవాహం_Error(ప్రవాహం_error_సంకేతం C)
    : ద్విమాన_ప్రవాహం_Error(C, "") {}

ద్విమాన_ప్రవాహం_Error::ద్విమాన_ప్రవాహం_Error(StringRef Context)
    : ద్విమాన_ప్రవాహం_Error(ప్రవాహం_error_సంకేతం::unspecified, Context) {}

ద్విమాన_ప్రవాహం_Error::ద్విమాన_ప్రవాహం_Error(ప్రవాహం_error_సంకేతం C, StringRef Context)
    : సంకేతం_2(C) {
  Err_సందేశం = "Stream Error: ";
  ఎంపిక (C) {
  స్థితి ప్రవాహం_error_సంకేతం::unspecified:
    Err_సందేశం += "An unspecified error has occurred.";
    ఆపు;
  స్థితి ప్రవాహం_error_సంకేతం::ప్రవాహం_too_short:
    Err_సందేశం += "The stream is too short to perform the requested operation.";
    ఆపు;
  స్థితి ప్రవాహం_error_సంకేతం::invalid_array_కొలత:
    Err_సందేశం += "The buffer size is not a multiple of the array element size.";
    ఆపు;
  స్థితి ప్రవాహం_error_సంకేతం::invalid_స్థానభ్రంశం:
    Err_సందేశం += "The specified offset is invalid for the current stream.";
    ఆపు;
  స్థితి ప్రవాహం_error_సంకేతం::filesystem_error:
    Err_సందేశం += "An I/O error occurred on the file system.";
    ఆపు;
  }

  ఒకవేళ (!Context.ఖాళీలు()) {
    Err_సందేశం += "  ";
    Err_సందేశం += Context;
  }
}

శూన్య ద్విమాన_ప్రవాహం_Error::సంవర్గమానం(raw_ostream &OS) స్థిరరాశి { OS << Err_సందేశం; }

StringRef ద్విమాన_ప్రవాహం_Error::get_Error_సందేశం() స్థిరరాశి { తిరిగి Err_సందేశం; }

std::error_సంకేతం ద్విమాన_ప్రవాహం_Error::convert_గమ్యం_Error_సంకేతం() స్థిరరాశి {
  తిరిగి inconvertible_Error_సంకేతం();
}

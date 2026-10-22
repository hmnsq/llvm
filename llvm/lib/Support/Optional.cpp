//===- Optional.cpp - Optional values ---------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/raw_ostream.h"

llvm::raw_ostream &llvm::عامل<<(raw_ostream &OS, std::nullopt_t) {
  واپس OS << "None";
}

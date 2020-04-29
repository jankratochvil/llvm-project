//===-- SymbolFileDWARFDwz.h ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef SymbolFileDWARFDwz_SymbolFileDWARFDwz_h_
#define SymbolFileDWARFDwz_SymbolFileDWARFDwz_h_

#include "SymbolFileDWARF.h"

typedef std::unique_ptr<SymbolFileDWARFDwz> SymbolFileDWARFDwzUP;

class SymbolFileDWARFDwz : public SymbolFileDWARF {
public:
  SymbolFileDWARFDwz(lldb::ObjectFileSP objfile, lldb::ModuleSP m_module);

  bool GetIsDwz() const override { return true; }

  static void InitializeForDWARF(SymbolFileDWARF &dwarf);

  void ClearForDWARF(SymbolFileDWARF &dwarf);

private:
  void SetForDWARF(SymbolFileDWARF &dwarf);

  size_t m_use_count = 0;
  lldb::ModuleSP m_module;

  DISALLOW_COPY_AND_ASSIGN(SymbolFileDWARFDwz);
};

#endif // SymbolFileDWARFDwz_SymbolFileDWARFDwz_h_

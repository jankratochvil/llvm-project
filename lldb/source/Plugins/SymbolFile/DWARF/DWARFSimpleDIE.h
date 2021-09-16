//===-- DWARFSimpleDIE.h ----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFSIMPLEDIE_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFSIMPLEDIE_H

#include "DWARFBaseDIE.h"

class DWARFSimpleDIE {
public:
  DWARFSimpleDIE() {}
  DWARFSimpleDIE(DWARFUnit *cu,const DWARFDebugInfoEntry *die):m_cu(cu),m_die(die) { assert(m_cu); assert(m_die); }
  DWARFSimpleDIE(const DWARFBaseDIE &die):DWARFSimpleDIE(die.GetCU(), die.GetDIE()) {}
  DWARFUnit *GetCU() const { return m_cu; }
  const DWARFDebugInfoEntry *GetDIE() const { return m_die; }
  dw_offset_t GetOffset() const;
  size_t GetAttributes(DWARFAttributes &attributes, DWARFBaseDIE::Recurse recurse) const;
  bool IsValid() const { return m_die != nullptr; }
  explicit operator bool() const { return IsValid(); }

private:
  DWARFUnit *m_cu=nullptr;
  const DWARFDebugInfoEntry *m_die=nullptr;
};

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFSIMPLEDIE_H

//===-- DWARFUnitPair.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Plugins/SymbolFile/DWARF/DWARFUnitPair.h"
#include "Plugins/SymbolFile/DWARF/DWARFCompileUnit.h"
#include "Plugins/SymbolFile/DWARF/DWARFTypeUnit.h"
#include <assert.h>

DWARFUnitPair::DWARFUnitPair():m_cu(nullptr),m_main_cu(nullptr) {}
DWARFUnitPair::DWARFUnitPair(DWARFUnit *cu,DWARFUnit *main_cu):m_cu(cu),m_main_cu(main_cu) {
  assert(m_cu);
  assert(m_main_cu);
  assert(!m_main_cu->GetSymbolFileDWARF().GetIsDwz());
}
DWARFUnitPair::DWARFUnitPair(DWARFUnit *main_cu):DWARFUnitPair(static_cast<DWARFUnit *>(main_cu), main_cu) {}
DWARFUnit *DWARFUnitPair::operator ->() const { assert(m_cu); return m_cu; }
DWARFUnit &DWARFUnitPair::operator *() const { assert(m_cu); return *m_cu; }
DWARFUnit *DWARFUnitPair::GetCU() const { assert(m_cu); return m_cu; }
DWARFUnit *DWARFUnitPair::GetMainCU() const { return m_main_cu; }

DWARFUnitPair::operator bool() const{ return m_cu != nullptr; }
DWARFUnitPair::operator DWARFUnit *() const { /*assert(m_cu);*/ return m_cu; }
DWARFUnitPair::operator DWARFUnit &() const { assert(m_cu); return *m_cu; }
bool DWARFUnitPair::operator ==(const DWARFUnitPair &rhs) const {
  return m_cu == rhs.m_cu && m_main_cu == rhs.m_main_cu;
}
void DWARFUnitPair::Clear() { m_cu = nullptr; m_main_cu=nullptr; }

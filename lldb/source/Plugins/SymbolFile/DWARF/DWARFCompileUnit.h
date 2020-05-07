//===-- DWARFCompileUnit.h --------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFCOMPILEUNIT_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFCOMPILEUNIT_H

#include "DWARFUnit.h"
#include "llvm/Support/Error.h"

class DWARFCompileUnit : public DWARFUnit {
public:
  void BuildAddressRangeTable(DWARFDebugAranges *debug_aranges) override;

  void Dump(lldb_private::Stream *s) const override;

  static bool classof(const DWARFUnit *unit) { return !unit->IsTypeUnit(); }

  DWARFCompileUnit &GetNonSkeletonUnit();

  MainDWARFCompileUnit *GetMainDWARFCompileUnit(MainDWARFCompileUnit *main_unit) override;

  DWARFBaseDIE GetUnitDWARFDIEOnly() { return {this, GetUnitDIEPtrOnly()}; }

  DWARFDIE DIE() { return {this, DIEPtr()}; }

  DWARFDIE LookupAddress(const dw_addr_t address);

private:
  DWARFCompileUnit(SymbolFileDWARF &dwarf, lldb::user_id_t uid,
                   const DWARFUnitHeader &header,
                   const DWARFAbbreviationDeclarationSet &abbrevs,
                   DIERef::Section section, bool is_dwo)
      : DWARFUnit(dwarf, uid, header, abbrevs, section, is_dwo) {}

  DISALLOW_COPY_AND_ASSIGN(DWARFCompileUnit);

  friend class DWARFUnit;
};

class MainDWARFCompileUnit:public DWARFCompileUnit {
private:
  using DWARFCompileUnit::DWARFCompileUnit;
public:
  operator MainDWARFUnit *() { return reinterpret_cast<MainDWARFUnit *>(this); }
  operator MainDWARFUnit &() { return reinterpret_cast<MainDWARFUnit &>(*this); }
  MainDWARFCompileUnit &GetNonSkeletonUnit();
  bool ContainsDIERef(DIERef die_ref) const;
  bool ContainsUID(lldb::user_id_t uid) const;
};
static_assert(sizeof(MainDWARFCompileUnit)==sizeof(DWARFCompileUnit),"");

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFCOMPILEUNIT_H

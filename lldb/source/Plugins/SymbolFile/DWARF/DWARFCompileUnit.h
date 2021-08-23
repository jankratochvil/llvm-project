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

  DWARFDIE LookupAddress(const dw_addr_t address);

  DWARFDIE GetDIE(dw_offset_t die_offset) { return DWARFUnit::GetDIE(this,die_offset); }

  DWARFBaseDIE GetUnitDIEOnly() { return {DWARFUnitPair(this), DIEPtr()}; }

  DWARFDIE DIE() { return {DWARFUnitPair(this), DIEPtr()}; }

  size_t AppendDIEsWithTag(const dw_tag_t tag, std::vector<DWARFDIE> &dies,
                           uint32_t depth = UINT32_MAX) const;

  static DWARFCompileUnit *GetMainUnit(const lldb_private::SymbolContext &sc,
                                       SymbolFileDWARF **dwarf_return);
  static DWARFCompileUnit *GetMainUnit(lldb_private::CompileUnit &comp_unit,
                                       SymbolFileDWARF **dwarf_return);

private:
  DWARFCompileUnit(SymbolFileDWARF &dwarf, lldb::user_id_t uid,
                   const DWARFUnitHeader &header,
                   const DWARFAbbreviationDeclarationSet &abbrevs,
                   DIERef::Section section, bool is_dwo)
      : DWARFUnit(dwarf, uid, header, abbrevs, section, is_dwo) {}

  DWARFCompileUnit(const DWARFCompileUnit &) = delete;
  const DWARFCompileUnit &operator=(const DWARFCompileUnit &) = delete;

  static DWARFCompileUnit *GetMainUnit(lldb_private::Module &module,
                                       lldb_private::CompileUnit *comp_unit,
                                       SymbolFileDWARF **dwarf_return);

  friend class DWARFUnit;
};

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFCOMPILEUNIT_H

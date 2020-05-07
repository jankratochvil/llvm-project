//===-- DWARFCompileUnit.cpp ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "DWARFCompileUnit.h"
#include "DWARFDebugAranges.h"
#include "SymbolFileDWARFDebugMap.h"
#include "SymbolFileDWARFDwo.h"

#include "lldb/Symbol/CompileUnit.h"
#include "lldb/Symbol/LineTable.h"
#include "lldb/Utility/Stream.h"

using namespace lldb;
using namespace lldb_private;

void DWARFCompileUnit::Dump(Stream *s) const {
  s->Printf("0x%8.8x: Compile Unit: length = 0x%8.8x, version = 0x%4.4x, "
            "abbr_offset = 0x%8.8x, addr_size = 0x%2.2x (next CU at "
            "{0x%8.8x})\n",
            GetOffset(), GetLength(), GetVersion(), GetAbbrevOffset(),
            GetAddressByteSize(), GetNextUnitOffset());
}

void DWARFCompileUnit::BuildAddressRangeTable(
    DWARFDebugAranges *debug_aranges) {
  // This function is usually called if there in no .debug_aranges section in
  // order to produce a compile unit level set of address ranges that is
  // accurate.

  size_t num_debug_aranges = debug_aranges->GetNumRanges();

  // First get the compile unit DIE only and check contains ranges information.
  const DWARFDebugInfoEntry *die = GetUnitDIEPtrOnly();

  // DWZ partial units never contain PC.
  if (die && die->Tag() == DW_TAG_partial_unit)
    return;

  const dw_offset_t cu_offset = GetOffset();
  if (die) {
    DWARFRangeList ranges;
    // It does not make sense to build address table for DW_TAG_partial_unit as
    // those never contain any addresses.
    const size_t num_ranges =
        die->GetAttributeAddressRanges(this, ranges, /*check_hi_lo_pc=*/true);
    if (num_ranges > 0) {
      for (size_t i = 0; i < num_ranges; ++i) {
        const DWARFRangeList::Entry &range = ranges.GetEntryRef(i);
        debug_aranges->AppendRange(cu_offset, range.GetRangeBase(),
                                   range.GetRangeEnd());
      }

      return;
    }
  }

  if (debug_aranges->GetNumRanges() == num_debug_aranges) {
    // We got nothing from the debug info, maybe we have a line tables only
    // situation. Check the line tables and build the arange table from this.
    SymbolContext sc;
    sc.comp_unit = m_dwarf.GetCompUnitForDWARFCompUnit(*this);
    if (sc.comp_unit) {
      SymbolFileDWARFDebugMap *debug_map_sym_file =
          m_dwarf.GetDebugMapSymfile();
      if (debug_map_sym_file == nullptr) {
        if (LineTable *line_table = sc.comp_unit->GetLineTable()) {
          LineTable::FileAddressRanges file_ranges;
          const bool append = true;
          const size_t num_ranges =
              line_table->GetContiguousFileAddressRanges(file_ranges, append);
          for (uint32_t idx = 0; idx < num_ranges; ++idx) {
            const LineTable::FileAddressRanges::Entry &range =
                file_ranges.GetEntryRef(idx);
            debug_aranges->AppendRange(cu_offset, range.GetRangeBase(),
                                       range.GetRangeEnd());
          }
        }
      } else
        debug_map_sym_file->AddOSOARanges(&m_dwarf, debug_aranges);
    }
  }

  if (debug_aranges->GetNumRanges() == num_debug_aranges) {
    // We got nothing from the functions, maybe we have a line tables only
    // situation. Check the line tables and build the arange table from this.
    SymbolContext sc;
    sc.comp_unit = m_dwarf.GetCompUnitForDWARFCompUnit(*this);
    if (sc.comp_unit) {
      if (LineTable *line_table = sc.comp_unit->GetLineTable()) {
        LineTable::FileAddressRanges file_ranges;
        const bool append = true;
        const size_t num_ranges =
            line_table->GetContiguousFileAddressRanges(file_ranges, append);
        for (uint32_t idx = 0; idx < num_ranges; ++idx) {
          const LineTable::FileAddressRanges::Entry &range =
              file_ranges.GetEntryRef(idx);
          debug_aranges->AppendRange(GetOffset(), range.GetRangeBase(),
                                     range.GetRangeEnd());
        }
      }
    }
  }
}

MainDWARFCompileUnit *
DWARFCompileUnit::GetMainDWARFCompileUnit(MainDWARFCompileUnit *main_unit) {
  if (!main_unit)
    main_unit = reinterpret_cast<MainDWARFCompileUnit *>(this);
  main_unit = &main_unit->GetNonSkeletonUnit();
  return main_unit;
}

DWARFDIE DWARFCompileUnit::LookupAddress(const dw_addr_t address) {
  if (GetUnitDIEPtrOnly()) {
    const DWARFDebugAranges &func_aranges = GetFunctionAranges();

    // Re-check the aranges auto pointer contents in case it was created above
    if (!func_aranges.IsEmpty())
      return GetDIE(func_aranges.FindAddress(address));
  }
  return DWARFDIE();
}

DWARFCompileUnit &DWARFCompileUnit::GetNonSkeletonUnit() {
  ExtractUnitDIEIfNeeded();
  if (m_dwo)
    return *m_dwo;
  return *this;
}

MainDWARFCompileUnit &MainDWARFCompileUnit::GetNonSkeletonUnit() {
  ExtractUnitDIEIfNeeded();
  if (m_dwo)
    return reinterpret_cast<MainDWARFCompileUnit &>(*m_dwo);
  return *this;
}

bool MainDWARFCompileUnit::ContainsDIERef(DIERef die_ref) const {
  if (die_ref.main_cu())
    return GetID() == *die_ref.main_cu();
  if (m_dwarf.GetDwoNum() != die_ref.dwo_num())
    return false;
  if (m_section != die_ref.section())
    return false;
  lldbassert(ContainsDIEOffset(die_ref.die_offset()) ==
             (GetOffset() <= die_ref.die_offset() &&
              die_ref.die_offset() < GetNextUnitOffset()));
  return ContainsDIEOffset(die_ref.die_offset());
}

bool MainDWARFCompileUnit::ContainsUID(user_id_t uid) const {
  llvm::Optional<SymbolFileDWARF::DecodedUID> decoded =
      m_dwarf.DecodeUIDUnlocked(uid);
  if (!decoded)
    return false;
  if (&decoded->dwarf != &m_dwarf)
    return false;
  return ContainsDIERef(decoded->ref);
}

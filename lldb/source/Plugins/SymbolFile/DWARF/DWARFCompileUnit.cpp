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

#include "lldb/Core/Module.h"
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

  // DWZ partial units never contain any PC.
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

DWARFCompileUnit &DWARFCompileUnit::GetNonSkeletonUnit() {
  return llvm::cast<DWARFCompileUnit>(DWARFUnit::GetNonSkeletonUnit());
}

DWARFDIE DWARFCompileUnit::LookupAddress(const dw_addr_t address) {
  if (DIE()) {
    const DWARFDebugAranges &func_aranges = GetFunctionAranges();

    // Re-check the aranges auto pointer contents in case it was created above
    if (!func_aranges.IsEmpty())
      return GetDIE(func_aranges.FindAddress(address));
  }
  return DWARFDIE();
}

size_t DWARFCompileUnit::AppendDIEsWithTag(const dw_tag_t tag,
                                           std::vector<DWARFDIE> &dies,
                                           uint32_t depth) const {
  size_t old_size = dies.size();
  {
    llvm::sys::ScopedReader lock(m_die_array_mutex);
    DWARFDebugInfoEntry::const_iterator pos;
    DWARFDebugInfoEntry::const_iterator end = m_die_array.end();
    DWARFUnitPair unitpair =
        DWARFUnitPair(const_cast<DWARFCompileUnit *>(this));
    for (pos = m_die_array.begin(); pos != end; ++pos) {
      if (pos->Tag() == tag)
        dies.emplace_back(unitpair, &(*pos));
    }
  }

  // Return the number of DIEs added to the collection
  return dies.size() - old_size;
}

DWARFCompileUnit *
DWARFCompileUnit::GetMainUnit(Module &module, CompileUnit *comp_unit,
                              SymbolFileDWARF **dwarf_return) {
  SymbolFile *symfile = module.GetSymbolFile();
  SymbolFileDWARF *dwarf;
  if (auto *debug_map_symfile =
          llvm::dyn_cast<SymbolFileDWARFDebugMap>(symfile)) {
    lldbassert(comp_unit);
    dwarf = debug_map_symfile->GetSymbolFile(*comp_unit);
  } else {
    dwarf = llvm::dyn_cast<SymbolFileDWARF>(symfile);
    lldbassert(dwarf);
  }
  if (dwarf_return)
    *dwarf_return = dwarf;
  if (!comp_unit) {
    lldbassert(dwarf_return);
    return nullptr;
  }
  return dwarf->GetDWARFCompileUnit(comp_unit);
}

DWARFCompileUnit *
DWARFCompileUnit::GetMainUnit(CompileUnit &comp_unit,
                              SymbolFileDWARF **dwarf_return) {
  ModuleSP module_sp = comp_unit.CalculateSymbolContextModule();
  lldbassert(module_sp);
  return GetMainUnit(*module_sp, &comp_unit, dwarf_return);
}

DWARFCompileUnit *
DWARFCompileUnit::GetMainUnit(const SymbolContext &sc,
                              SymbolFileDWARF **dwarf_return) {
  lldbassert(sc.module_sp);
  return GetMainUnit(*sc.module_sp, sc.comp_unit, dwarf_return);
}

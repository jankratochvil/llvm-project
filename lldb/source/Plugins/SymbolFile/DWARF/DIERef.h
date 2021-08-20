//===-- DIERef.h ------------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DIEREF_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DIEREF_H

#include "lldb/Core/dwarf.h"
#include "llvm/ADT/Optional.h"
#include "llvm/Support/FormatProviders.h"
#include <cassert>
#include <vector>

/// Identifies a DWARF debug info entry within a given Module. It contains
/// multiple "coordinates":
/// - dwo_num: -gsplit-dwarf only: identifies the dwo file in the Module.
///   If this field is not set, the DIERef references the main file.
/// - main_cu: DWZ only: index of main compilation unit which used
///   DW_TAG_imported_unit for this DIERef or some its parent.
///   For transitive imports this is the very first CU.
///   It is always a CU in the main file (and not DWZ common symbol file).
/// - DWZ common flag: DWZ only: the DIE is in a shared DWZ common symbol file.
/// - section: identifies the section of the debug info entry in the given file:
///   debug_info or debug_types.
/// - die_offset: The offset of the debug info entry as an absolute offset from
///   the beginning of the section specified in the section field.
class DIERef {
public:
  enum Section : uint8_t { DebugInfo, DebugTypes };

  // DWZ only: Is the DIE located in DWZ common symbol file?
  enum DwzCommon : uint8_t { MainDwz, CommonDwz };

  enum Kind : uint8_t {
    NoneKind,

    // DWZ only: The DIE is in DW_TAG_partial_unit in main symbol file and it
    // needs main_cu.
    MainDwzKind,

    // -gsplit-dwarf only: The DIE is in a separate dwo file specified by
    // dwo_num.
    DwoKind,

    // DWZ only: The DIE is in DW_TAG_partial_unit in DWZ common symbol file and
    // it needs main_cu.
    DwzCommonKind
  };

  DIERef(llvm::Optional<uint32_t> dwo_num, llvm::Optional<uint32_t> main_cu,
         DwzCommon dwz_common, Section section, dw_offset_t die_offset)
      : m_data(dwo_num.getValueOr(0) | main_cu.getValueOr(0)),
        m_data_kind(dwo_num ? DwoKind
                            : (main_cu ? (dwz_common == MainDwz ? MainDwzKind
                                                                : DwzCommonKind)
                                       : NoneKind)),
        m_section(section), m_die_offset(die_offset) {
    assert(this->dwo_num() == dwo_num && "Dwo number out of range?");
    assert(this->main_cu() == main_cu && "Main Cu number out of range?");
    assert(dwz_common == MainDwz || main_cu);
  }

  llvm::Optional<uint32_t> dwo_num() const {
    if (m_data_kind == DwoKind)
      return m_data;
    return llvm::None;
  }

  // It indexes DWARFCompileUnit's excl. DWARFTypeUnit's.
  // It is the index used as parameter of SymbolFileDWARF::GetDWARFUnitIndex.
  llvm::Optional<uint32_t> main_cu() const {
    if (m_data_kind == MainDwzKind || m_data_kind == DwzCommonKind)
      return m_data;
    return llvm::None;
  }

  DwzCommon dwz_common() const {
    assert(m_data_kind == MainDwzKind || m_data_kind == DwzCommonKind);
    return m_data_kind == MainDwzKind ? MainDwz : CommonDwz;
  }

  Kind kind_get() const { return Kind(m_data_kind); }

  Section section() const { return static_cast<Section>(m_section); }

  dw_offset_t die_offset() const { return m_die_offset; }

  bool operator<(DIERef other) const {
    if (m_data_kind != other.m_data_kind)
      return m_data_kind < other.m_data_kind;
    if (m_data_kind != NoneKind && (m_data != other.m_data))
      return m_data < other.m_data;
    if (m_section != other.m_section)
      return m_section < other.m_section;
    return m_die_offset < other.m_die_offset;
  }

  bool operator==(DIERef other) const {
    if (m_data_kind != other.m_data_kind)
      return m_data_kind == other.m_data_kind;
    if (m_data_kind != NoneKind && (m_data != other.m_data))
      return m_data == other.m_data;
    if (m_section != other.m_section)
      return m_section == other.m_section;
    return m_die_offset == other.m_die_offset;
  }

private:
  uint32_t m_data : 29;
  uint32_t m_data_kind : 2; // Kind type
  uint32_t m_section : 1;   // Section type
  dw_offset_t m_die_offset;
};
static_assert(sizeof(DIERef) == 8, "");

typedef std::vector<DIERef> DIEArray;

namespace llvm {
template<> struct format_provider<DIERef> {
  static void format(const DIERef &ref, raw_ostream &OS, StringRef Style);
};
} // namespace llvm

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DIEREF_H

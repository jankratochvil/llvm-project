//===-- NameToDIE.cpp -----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "NameToDIE.h"
#include "DWARFCompileUnit.h"
#include "lldb/Symbol/ObjectFile.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/Utility/RegularExpression.h"
#include "lldb/Utility/Stream.h"
#include "lldb/Utility/StreamString.h"

using namespace lldb;
using namespace lldb_private;

void NameToDIE::Finalize() {
  m_map.Sort();
  m_map.SizeToFit();
}

void NameToDIE::Insert(ConstString name, const DIERef &die_ref) {
  m_map.Append(name, die_ref);
}

bool NameToDIE::Find(ConstString name,
                     llvm::function_ref<bool(DIERef ref)> callback) const {
  for (const auto &entry : m_map.equal_range(name))
    if (!callback(entry.value))
      return false;
  return true;
}

bool NameToDIE::Find(const RegularExpression &regex,
                     llvm::function_ref<bool(DIERef ref)> callback) const {
  for (const auto &entry : m_map)
    if (regex.Execute(entry.cstring.GetCString())) {
      if (!callback(entry.value))
        return false;
    }
  return true;
}

void NameToDIE::FindAllEntriesForUnit(
    const MainDWARFCompileUnit &unit,
    llvm::function_ref<bool(DIERef ref)> callback) const {
  const uint32_t size = m_map.GetSize();
  for (uint32_t i = 0; i < size; ++i) {
    const DIERef &die_ref = m_map.GetValueAtIndexUnchecked(i);
    if (unit.ContainsDIERef(die_ref)) {
      if (!callback(die_ref))
        return;
    }
  }
}

void NameToDIE::Dump(Stream *s) {
  llvm::raw_ostream &OS = s->AsRawOstream();
  const uint32_t size = m_map.GetSize();
  for (uint32_t i = 0; i < size; ++i) {
    DIERef ref = m_map.GetValueAtIndexUnchecked(i);
    if (ref.dwo_num())
      OS << llvm::format_hex_no_prefix(*ref.dwo_num(), 8) << "/";
    if (ref.main_cu())
      OS << llvm::format_hex_no_prefix(*ref.main_cu(), 8) << "/";
    OS << (ref.section() == DIERef::Section::DebugTypes ? "TYPE" : "INFO");
    switch (ref.kind_get()) {
      case DIERef::Kind::NoneKind:
      case DIERef::Kind::DwoKind:
        break;
      case DIERef::Kind::MainDwzKind:
        OS << "/DWZ";
        break;
      case DIERef::Kind::DwzCommonKind:
        OS << "/DWZCOMMON";
        break;
    }
    OS << "/" << llvm::format_hex_no_prefix(ref.die_offset(), 8) << " \""
       << m_map.GetCStringAtIndexUnchecked(i).GetStringRef() << "\"\n";
  }
}

void NameToDIE::ForEach(
    std::function<bool(ConstString name, const DIERef &die_ref)> const
        &callback) const {
  const uint32_t size = m_map.GetSize();
  for (uint32_t i = 0; i < size; ++i) {
    if (!callback(m_map.GetCStringAtIndexUnchecked(i),
                  m_map.GetValueAtIndexUnchecked(i)))
      break;
  }
}

void NameToDIE::Append(const NameToDIE &other) {
  const uint32_t size = other.m_map.GetSize();
  for (uint32_t i = 0; i < size; ++i) {
    m_map.Append(other.m_map.GetCStringAtIndexUnchecked(i),
                 other.m_map.GetValueAtIndexUnchecked(i));
  }
}

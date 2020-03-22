//===-- NameToDIE.cpp -----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "NameToDIE.h"
#include "DWARFUnit.h"
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

void NameToDIE::Insert(ConstString name, user_id_t uid) {
  m_map.Append(name, uid);
}

size_t NameToDIE::Find(ConstString name, std::vector<lldb::user_id_t> &info_array) const {
  return m_map.GetValues(name, info_array);
}

size_t NameToDIE::Find(const RegularExpression &regex,
                       std::vector<lldb::user_id_t> &info_array) const {
  return m_map.GetValues(regex, info_array);
}

size_t NameToDIE::FindAllEntriesForUnit(const DWARFUnit &unit,
                                        std::vector<lldb::user_id_t> &info_array) const {
  const size_t initial_size = info_array.size();
  const uint32_t size = m_map.GetSize();
  for (uint32_t i = 0; i < size; ++i) {
    user_id_t uid = m_map.GetValueAtIndexUnchecked(i);
    llvm::Optional<DIERef> die_ref_opt = unit.GetSymbolFileDWARF().GetDIERef(uid);
    if (!die_ref_opt)
      continue;
    DIERef die_ref = *die_ref_opt;
    if (unit.GetSymbolFileDWARF().GetDwoNum() == die_ref.dwo_num() &&
        unit.GetDebugSection() == die_ref.section() &&
        unit.GetOffset() <= die_ref.die_offset() &&
        die_ref.die_offset() < unit.GetNextUnitOffset())
      info_array.push_back(uid);
  }
  return info_array.size() - initial_size;
}

void NameToDIE::Dump(Stream *s) {
  llvm::raw_ostream &OS = s->AsRawOstream();
  const uint32_t size = m_map.GetSize();
  for (uint32_t i = 0; i < size; ++i) {
    user_id_t uid = m_map.GetValueAtIndexUnchecked(i);
    uint32_t upper = (uid >> 32) & 0x1fffffff;
    if (upper != 0x1fffffff)
      OS << llvm::format_hex_no_prefix(upper, 8) << "/";
    OS << (uid & (1ULL << 63) ? "TYPE" : "INFO");
    if (uid & (1ULL << 61))
      OS << "/DWZCOMMON";
    else if (uid & (1ULL << 62))
      OS << "/DWZ";
    OS << "/" << llvm::format_hex_no_prefix(uid & 0xffffffff, 8) << " \"" << m_map.GetCStringAtIndexUnchecked(i).GetStringRef() << "\"\n";
  }
}

void NameToDIE::ForEach(
    std::function<bool(ConstString name, user_id_t uid)> const
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

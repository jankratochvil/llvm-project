//===-- DWARFBaseDIE.cpp --------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "DWARFBaseDIE.h"

#include "DWARFCompileUnit.h"
#include "DWARFDebugInfoEntry.h"
#include "DWARFSimpleDIE.h"
#include "DWARFUnit.h"
#include "SymbolFileDWARF.h"

#include "lldb/Core/Module.h"
#include "lldb/Symbol/ObjectFile.h"
#include "lldb/Utility/Log.h"

using namespace lldb_private;

llvm::Optional<DIERef> DWARFBaseDIE::GetDIERef() const {
  if (!IsValid())
    return llvm::None;

  DWARFUnit *main_unit = GetMainCU();

  if (GetCU()->GetSymbolFileDWARF().GetDwoNum().hasValue())
    main_unit = nullptr;
  if (GetCU().GetCU() == main_unit)
    main_unit = nullptr;

  return DIERef(
      m_cu->GetSymbolFileDWARF().GetDwoNum(),
      (!main_unit ? llvm::None : llvm::Optional<uint32_t>(main_unit->GetID())),
      GetCU()->GetSymbolFileDWARF().GetIsDwz() ? DIERef::CommonDwz
                                               : DIERef::MainDwz,
      m_cu->GetDebugSection(), m_die->GetOffset());
}

dw_tag_t DWARFBaseDIE::Tag() const {
  if (m_die)
    return m_die->Tag();
  else
    return llvm::dwarf::DW_TAG_null;
}

const char *DWARFBaseDIE::GetTagAsCString() const {
  return lldb_private::DW_TAG_value_to_name(Tag());
}

const char *DWARFBaseDIE::GetAttributeValueAsString(const dw_attr_t attr,
                                                const char *fail_value) const {
  if (IsValid())
    return m_die->GetAttributeValueAsString(GetCU(), attr, fail_value);
  else
    return fail_value;
}

uint64_t DWARFBaseDIE::GetAttributeValueAsUnsigned(const dw_attr_t attr,
                                               uint64_t fail_value) const {
  if (IsValid())
    return m_die->GetAttributeValueAsUnsigned(GetCU(), attr, fail_value);
  else
    return fail_value;
}

uint64_t DWARFBaseDIE::GetAttributeValueAsAddress(const dw_attr_t attr,
                                              uint64_t fail_value) const {
  if (IsValid())
    return m_die->GetAttributeValueAsAddress(GetCU(), attr, fail_value);
  else
    return fail_value;
}

lldb::user_id_t DWARFBaseDIE::GetID() const {
  if (IsValid())
    return GetMainDWARF()->GetUID(*this);
  return LLDB_INVALID_UID;
}

const char *DWARFBaseDIE::GetName() const {
  if (IsValid())
    return m_die->GetName(m_cu);
  else
    return nullptr;
}

lldb::ModuleSP DWARFBaseDIE::GetModule() const {
  SymbolFileDWARF *dwarf = GetDWARF();
  if (dwarf)
    return dwarf->GetObjectFile()->GetModule();
  else
    return lldb::ModuleSP();
}

dw_offset_t DWARFBaseDIE::GetOffset() const {
  if (IsValid())
    return m_die->GetOffset();
  else
    return DW_INVALID_OFFSET;
}

SymbolFileDWARF *DWARFBaseDIE::GetDWARF() const {
  if (m_cu)
    return &m_cu->GetSymbolFileDWARF();
  else
    return nullptr;
}

SymbolFileDWARF *DWARFBaseDIE::GetMainDWARF() const {
  if (m_cu.GetMainCU())
    return &m_cu.GetMainCU()->GetSymbolFileDWARF();
  return GetDWARF();
}

bool DWARFBaseDIE::HasChildren() const {
  return m_die && m_die->HasChildren();
}

bool DWARFBaseDIE::Supports_DW_AT_APPLE_objc_complete_type() const {
  return IsValid() && GetDWARF()->Supports_DW_AT_APPLE_objc_complete_type(m_cu);
}

size_t DWARFBaseDIE::GetAttributes(DWARFAttributes &attributes,
                                   Recurse recurse) const {
  return DWARFSimpleDIE(*this).GetAttributes(attributes, recurse);
}

bool operator==(const DWARFBaseDIE &lhs, const DWARFBaseDIE &rhs) {
  return lhs.GetDIE() == rhs.GetDIE() && lhs.GetCU() == rhs.GetCU() &&
         lhs.GetMainCU() == rhs.GetMainCU();
}

bool operator!=(const DWARFBaseDIE &lhs, const DWARFBaseDIE &rhs) {
  return !(lhs == rhs);
}

const DWARFDataExtractor &DWARFBaseDIE::GetData() const {
  // Clients must check if this DIE is valid before calling this function.
  assert(IsValid());
  return m_cu->GetData();
}

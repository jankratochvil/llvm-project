//===-- DWARFSimpleDIE.cpp ------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "DWARFSimpleDIE.h"
#include "DWARFDebugInfoEntry.h"

dw_offset_t DWARFSimpleDIE::GetOffset() const { return !m_die ? DW_INVALID_OFFSET : m_die->GetOffset(); }

size_t DWARFSimpleDIE::GetAttributes(DWARFAttributes &attributes,
                                     DWARFBaseDIE::Recurse recurse) const {
  if (IsValid())
    return m_die->GetAttributes(m_cu, attributes, recurse);
  attributes.Clear();
  return 0;
}

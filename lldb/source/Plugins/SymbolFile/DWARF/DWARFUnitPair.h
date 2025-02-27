//===-- DWARFUnitPair.h -----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_DWARFUnitPair_h_
#define liblldb_DWARFUnitPair_h_

#include "lldb/Symbol/TypeSystem.h"
#include "lldb/lldb-enumerations.h"
#include "llvm/Support/Error.h"

class DWARFUnit;

class DWARFUnitPair {
public:
  DWARFUnitPair();
  DWARFUnitPair(DWARFUnit *cu, DWARFUnit *main_cu);
  DWARFUnitPair(DWARFUnit *main_cu);
  DWARFUnit *operator->() const;
  DWARFUnit &operator*() const;
  DWARFUnit *GetCU() const;
  DWARFUnit *GetMainCU() const;
  explicit operator bool() const;
  operator DWARFUnit *() const;
  operator DWARFUnit &() const;
  bool operator==(const DWARFUnitPair &rhs) const;
  void Clear();

private:
  DWARFUnit *m_cu;
  // For non-DWZ setups it is equal to 'm_cu'.
  DWARFUnit *m_main_cu;
};

#endif // liblldb_DWARFUnitPair_h_

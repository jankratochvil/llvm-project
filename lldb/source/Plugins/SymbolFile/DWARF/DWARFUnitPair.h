//===-- DWARFUnitPair.h -----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_DWARFUnitPair_h_
#define liblldb_DWARFUnitPair_h_

#include "llvm/Support/Error.h"
#include "lldb/lldb-enumerations.h"
#include "lldb/Symbol/TypeSystem.h"

class DWARFUnit;
class DWARFCompileUnit;
class DWARFTypeUnit;

class DWARFUnitPair {
public:
  DWARFUnitPair();
  DWARFUnitPair(DWARFUnit *cu,DWARFCompileUnit *main_cu);
  DWARFUnitPair(DWARFCompileUnit *cu,DWARFCompileUnit *main_cu)=delete;
  DWARFUnitPair(DWARFCompileUnit *main_cu);
  DWARFUnit *operator ->() const;
  DWARFUnit &operator *() const;
  DWARFUnit *GetCU() const;
  DWARFCompileUnit *GetMainCU() const;
  DWARFCompileUnit *GetMainCUOrNull() const;
  explicit operator bool() const;
  operator DWARFUnit *() const;
  operator DWARFUnit &() const;
  bool operator ==(const DWARFUnitPair &rhs) const;
  void Clear();
  uint64_t GetDWARFLanguageType() const;
  DWARFUnitPair GetNonSkeletonUnit() const;

private:
  DWARFUnit *m_cu;
  // For non-DWZ setups it is either equal to 'm_cu' or nullptr if 'm_cu' is a DWARFTypeUnit.
  DWARFCompileUnit *m_main_cu;

  template<class DWARFCompileUnitT,class DWARFTypeUnitT> auto CompileOrTypeMethod(DWARFCompileUnitT compile_method,DWARFTypeUnitT type_method) const -> decltype((m_main_cu->*compile_method)());
};

#endif // liblldb_DWARFUnitPair_h_

//===-- DWARFIndex.cpp ----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Plugins/SymbolFile/DWARF/DWARFIndex.h"
#include "Plugins/Language/ObjC/ObjCLanguage.h"
#include "Plugins/SymbolFile/DWARF/DWARFDIE.h"
#include "Plugins/SymbolFile/DWARF/SymbolFileDWARF.h"
#include "Plugins/SymbolFile/DWARF/DWARFCompileUnit.h"

using namespace lldb_private;
using namespace lldb;

DWARFIndex::~DWARFIndex() = default;

void DWARFIndex::ProcessFunctionDIE(llvm::StringRef name, user_id_t uid,
                                    SymbolFileDWARF &dwarf,
                                    const CompilerDeclContext &parent_decl_ctx,
                                    uint32_t name_type_mask,
                                    std::vector<std::pair<DWARFCompileUnit *, DWARFDIE>> &dies) {
  DWARFCompileUnit *main_unit;
  DWARFDIE die = dwarf.GetDIE(uid, &main_unit);
  if (!die) {
    ReportInvalidDIEID(uid, name);
    return;
  }

  // Exit early if we're searching exclusively for methods or selectors and
  // we have a context specified (no methods in namespaces).
  uint32_t looking_for_nonmethods =
      name_type_mask & ~(eFunctionNameTypeMethod | eFunctionNameTypeSelector);
  if (!looking_for_nonmethods && parent_decl_ctx.IsValid())
    return;

  // Otherwise, we need to also check that the context matches. If it does not
  // match, we do nothing.
  if (!SymbolFileDWARF::DIEInDeclContext(parent_decl_ctx, main_unit, die))
    return;

  auto diepair = die.MainCUtoDWARFDIEPair(main_unit);

  // In case of a full match, we just insert everything we find.
  if (name_type_mask & eFunctionNameTypeFull) {
    dies.push_back(diepair);
    return;
  }

  // If looking for ObjC selectors, we need to also check if the name is a
  // possible selector.
  if (name_type_mask & eFunctionNameTypeSelector &&
      ObjCLanguage::IsPossibleObjCMethodName(die.GetName())) {
    dies.push_back(diepair);
    return;
  }

  bool looking_for_methods = name_type_mask & lldb::eFunctionNameTypeMethod;
  bool looking_for_functions = name_type_mask & lldb::eFunctionNameTypeBase;
  if (looking_for_methods || looking_for_functions) {
    // If we're looking for either methods or functions, we definitely want this
    // die. Otherwise, only keep it if the die type matches what we are
    // searching for.
    if ((looking_for_methods && looking_for_functions) ||
        looking_for_methods == die.IsMethod())
      dies.push_back(diepair);
  }
}

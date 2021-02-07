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

#include "lldb/Core/Module.h"

using namespace lldb_private;
using namespace lldb;

DWARFIndex::~DWARFIndex() = default;

bool DWARFIndex::ProcessFunctionDIE(
    llvm::StringRef name, DWARFUnit *main_unit, DIERef ref,
    SymbolFileDWARF &dwarf, const CompilerDeclContext &parent_decl_ctx,
    uint32_t name_type_mask,
    llvm::function_ref<bool(DWARFUnit *main_unit, DWARFDIE die)> callback) {
  DWARFDIE die = dwarf.GetDIE(ref);
  if (!die) {
    ReportInvalidDIERef(ref, name);
    return true;
  }
  main_unit = die.GetMainDWARFUnit(main_unit);

  // Exit early if we're searching exclusively for methods or selectors and
  // we have a context specified (no methods in namespaces).
  uint32_t looking_for_nonmethods =
      name_type_mask & ~(eFunctionNameTypeMethod | eFunctionNameTypeSelector);
  if (!looking_for_nonmethods && parent_decl_ctx.IsValid())
    return true;

  // Otherwise, we need to also check that the context matches. If it does not
  // match, we do nothing.
  if (!SymbolFileDWARF::DIEInDeclContext(parent_decl_ctx, main_unit, die))
    return true;

  // In case of a full match, we just insert everything we find.
  if (name_type_mask & eFunctionNameTypeFull)
    return callback(main_unit, die);

  // If looking for ObjC selectors, we need to also check if the name is a
  // possible selector.
  if (name_type_mask & eFunctionNameTypeSelector &&
      ObjCLanguage::IsPossibleObjCMethodName(die.GetName()))
    return callback(main_unit, die);

  bool looking_for_methods = name_type_mask & lldb::eFunctionNameTypeMethod;
  bool looking_for_functions = name_type_mask & lldb::eFunctionNameTypeBase;
  if (looking_for_methods || looking_for_functions) {
    // If we're looking for either methods or functions, we definitely want this
    // die. Otherwise, only keep it if the die type matches what we are
    // searching for.
    if ((looking_for_methods && looking_for_functions) ||
        looking_for_methods == die.IsMethod())
      return callback(main_unit, die);
  }

  return true;
}

DWARFIndex::DIECallbackImpl::DIECallbackImpl(
    const DWARFIndex &index,
    llvm::function_ref<bool(DWARFUnit *main_unit, DWARFDIE die)> callback,
    llvm::StringRef name)
    : m_index(index),
      m_dwarf(*llvm::cast<SymbolFileDWARF>(index.m_module.GetSymbolFile())),
      m_callback(callback), m_name(name) {}

bool DWARFIndex::DIERefCallbackImpl::operator()(DIERef ref) const {
  DWARFUnit *main_unit;
  if (DWARFDIE die = m_dwarf.GetDIE(ref, &main_unit))
    return m_callback(main_unit, die);
  m_index.ReportInvalidDIERef(ref, m_name);
  return true;
}

bool DWARFIndex::DIEUIDCallbackImpl::operator()(user_id_t uid) const {
  DWARFUnit *main_unit;
  if (DWARFDIE die = m_dwarf.GetDIEUnlocked(uid, &main_unit))
    return m_callback(main_unit, die);
  m_index.ReportInvalidDIEUID(uid, m_name);
  return true;
}

void DWARFIndex::ReportInvalidDIERef(DIERef ref, llvm::StringRef name) const {
  m_module.ReportErrorIfModifyDetected(
      "the DWARF debug information has been modified (accelerator table had "
      "bad DIERef 0x%8.8x for '%s')\n",
      ref.die_offset(), name.str().c_str());
}

void DWARFIndex::ReportInvalidDIEUID(user_id_t uid,
                                     llvm::StringRef name) const {
  m_module.ReportErrorIfModifyDetected(
      "the DWARF debug information has been modified (accelerator table had "
      "bad user_id_t 0x%8.8lx for '%s')\n",
      uid, name.str().c_str());
}

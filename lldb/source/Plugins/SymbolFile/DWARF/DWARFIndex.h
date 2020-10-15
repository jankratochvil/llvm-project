//===-- DWARFIndex.h -------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFINDEX_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFINDEX_H

#include "Plugins/SymbolFile/DWARF/DIERef.h"
#include "Plugins/SymbolFile/DWARF/DWARFDIE.h"
#include "Plugins/SymbolFile/DWARF/DWARFFormValue.h"

class DWARFDeclContext;
class DWARFDIE;

namespace lldb_private {
class DWARFIndex {
public:
  DWARFIndex(Module &module) : m_module(module) {}
  virtual ~DWARFIndex();

  virtual void Preload() = 0;

  /// Finds global variables with the given base name. Any additional filtering
  /// (e.g., to only retrieve variables from a given context) should be done by
  /// the consumer.
  virtual void GetGlobalVariables(
      ConstString basename,
      llvm::function_ref<bool(DWARFCompileUnit *main_unit, DWARFDIE die)>
          callback) = 0;

  virtual void GetGlobalVariables(
      const RegularExpression &regex,
      llvm::function_ref<bool(DWARFCompileUnit *main_unit, DWARFDIE die)>
          callback) = 0;
  virtual void GetGlobalVariables(
      const DWARFCompileUnit &main_unit,
      llvm::function_ref<bool(DWARFCompileUnit *main_unit, DWARFDIE die)>
          callback) = 0;
  virtual void GetObjCMethods(
      ConstString class_name,
      llvm::function_ref<bool(DWARFCompileUnit *main_unit, DWARFDIE die)>
          callback) = 0;
  virtual void GetCompleteObjCClass(
      ConstString class_name, bool must_be_implementation,
      llvm::function_ref<bool(DWARFCompileUnit *main_unit, DWARFDIE die)>
          callback) = 0;
  virtual void
  GetTypes(ConstString name,
           llvm::function_ref<bool(DWARFCompileUnit *main_unit, DWARFDIE die)>
               callback) = 0;
  virtual void
  GetTypes(const DWARFDeclContext &context,
           llvm::function_ref<bool(DWARFCompileUnit *main_unit, DWARFDIE die)>
               callback) = 0;
  virtual void GetNamespaces(
      ConstString name,
      llvm::function_ref<bool(DWARFCompileUnit *main_unit, DWARFDIE die)>
          callback) = 0;
  virtual void GetFunctions(
      ConstString name, SymbolFileDWARF &dwarf,
      const CompilerDeclContext &parent_decl_ctx, uint32_t name_type_mask,
      llvm::function_ref<bool(DWARFCompileUnit *main_unit, DWARFDIE die)>
          callback) = 0;
  virtual void GetFunctions(
      const RegularExpression &regex,
      llvm::function_ref<bool(DWARFCompileUnit *main_unit, DWARFDIE die)>
          callback) = 0;

  virtual void Dump(Stream &s) = 0;

protected:
  Module &m_module;

  /// Helper function implementing common logic for processing function dies. If
  /// the function given by "ref" matches search criteria given by
  /// "parent_decl_ctx" and "name_type_mask", it is inserted into the "dies"
  /// vector.
  bool ProcessFunctionDIE(
      llvm::StringRef name, DWARFCompileUnit *main_unit, DIERef ref,
      SymbolFileDWARF &dwarf, const CompilerDeclContext &parent_decl_ctx,
      uint32_t name_type_mask,
      llvm::function_ref<bool(DWARFCompileUnit *main_unit, DWARFDIE die)>
          callback);

  class DIECallbackImpl {
  public:
    DIECallbackImpl(
        const DWARFIndex &index,
        llvm::function_ref<bool(DWARFCompileUnit *main_unit, DWARFDIE die)>
            callback,
        llvm::StringRef name);

  protected:
    const DWARFIndex &m_index;
    SymbolFileDWARF &m_dwarf;
    const llvm::function_ref<bool(DWARFCompileUnit *main_unit, DWARFDIE die)>
        m_callback;
    const llvm::StringRef m_name;
  };
  class DIERefCallbackImpl : protected DIECallbackImpl {
  public:
    using DIECallbackImpl::DIECallbackImpl;
    bool operator()(DIERef ref) const;
  };
  class DIEUIDCallbackImpl : protected DIECallbackImpl {
  public:
    using DIECallbackImpl::DIECallbackImpl;
    bool operator()(lldb::user_id_t uid) const;
  };
  DIERefCallbackImpl DIERefCallback(
      llvm::function_ref<bool(DWARFCompileUnit *main_unit, DWARFDIE die)>
          callback,
      llvm::StringRef name = {}) const {
    return DIERefCallbackImpl(*this, callback, name);
  }
  DIEUIDCallbackImpl DIEUIDCallback(
      llvm::function_ref<bool(DWARFCompileUnit *main_unit, DWARFDIE die)>
          callback,
      llvm::StringRef name = {}) const {
    return DIEUIDCallbackImpl(*this, callback, name);
  }

  void ReportInvalidDIERef(DIERef ref, llvm::StringRef name) const;
  void ReportInvalidDIEUID(lldb::user_id_t, llvm::StringRef name) const;
};
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFINDEX_H

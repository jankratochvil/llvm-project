//===-- AppleDWARFIndex.h --------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_APPLEDWARFINDEX_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_APPLEDWARFINDEX_H

#include "Plugins/SymbolFile/DWARF/DWARFIndex.h"
#include "Plugins/SymbolFile/DWARF/HashedNameToDIE.h"
#include "Plugins/SymbolFile/DWARF/SymbolFileDWARF.h"
#include "lldb/Symbol/ObjectFile.h"

namespace lldb_private {
class AppleDWARFIndex : public DWARFIndex {
public:
  static std::unique_ptr<AppleDWARFIndex>
  Create(SymbolFileDWARF &dwarf, DWARFDataExtractor apple_names,
         DWARFDataExtractor apple_namespaces, DWARFDataExtractor apple_types,
         DWARFDataExtractor apple_objc, DWARFDataExtractor debug_str);

  AppleDWARFIndex(
      SymbolFileDWARF &dwarf, std::unique_ptr<DWARFMappedHash::MemoryTable> apple_names,
      std::unique_ptr<DWARFMappedHash::MemoryTable> apple_namespaces,
      std::unique_ptr<DWARFMappedHash::MemoryTable> apple_types,
      std::unique_ptr<DWARFMappedHash::MemoryTable> apple_objc)
      : DWARFIndex(*dwarf.GetObjectFile()->GetModule()), m_dwarf(dwarf), m_apple_names_up(std::move(apple_names)),
        m_apple_namespaces_up(std::move(apple_namespaces)),
        m_apple_types_up(std::move(apple_types)),
        m_apple_objc_up(std::move(apple_objc)) {}

  void Preload() override {}

  void GetGlobalVariables(ConstString basename, std::vector<lldb::user_id_t> &offsets) override;
  void GetGlobalVariables(const RegularExpression &regex,
                          std::vector<lldb::user_id_t> &offsets) override;
  void GetGlobalVariables(const DWARFUnit &cu, std::vector<lldb::user_id_t> &offsets) override;
  void GetObjCMethods(ConstString class_name, std::vector<lldb::user_id_t> &offsets) override;
  void GetCompleteObjCClass(ConstString class_name, bool must_be_implementation,
                            std::vector<lldb::user_id_t> &offsets) override;
  void GetTypes(ConstString name, std::vector<lldb::user_id_t> &offsets) override;
  void GetTypes(const DWARFDeclContext &context, std::vector<lldb::user_id_t> &offsets) override;
  void GetNamespaces(ConstString name, std::vector<lldb::user_id_t> &offsets) override;
  void GetFunctions(ConstString name, SymbolFileDWARF &dwarf,
                    const CompilerDeclContext &parent_decl_ctx,
                    uint32_t name_type_mask,
                    std::vector<std::pair<DWARFUnit *, DWARFDIE>> &dies) override;
  void GetFunctions(const RegularExpression &regex, std::vector<lldb::user_id_t> &offsets) override;

  void ReportInvalidDIEID(lldb::user_id_t uid, llvm::StringRef name) override;
  void Dump(Stream &s) override;

private:
  SymbolFileDWARF &m_dwarf;
  std::unique_ptr<DWARFMappedHash::MemoryTable> m_apple_names_up;
  std::unique_ptr<DWARFMappedHash::MemoryTable> m_apple_namespaces_up;
  std::unique_ptr<DWARFMappedHash::MemoryTable> m_apple_types_up;
  std::unique_ptr<DWARFMappedHash::MemoryTable> m_apple_objc_up;
};
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_APPLEDWARFINDEX_H

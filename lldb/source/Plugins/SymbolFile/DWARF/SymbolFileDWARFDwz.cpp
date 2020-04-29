//===-- SymbolFileDWARFDwz.cpp ----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SymbolFileDWARFDwz.h"
#include "DWARFDebugInfo.h"
#include "lldb/Symbol/ObjectFile.h"
#include "lldb/Core/ModuleSpec.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Core/Module.h"
#include "lldb/Utility/FileSpec.h"
#include "llvm/ADT/PointerUnion.h"

using namespace lldb;
using namespace lldb_private;

SymbolFileDWARFDwz::SymbolFileDWARFDwz(ObjectFileSP objfile, ModuleSP module)
    : SymbolFileDWARF(std::move(objfile), /*dwo_section_list*/ nullptr),
    m_module(std::move(module)) {
  InitializeObject();
  // Call private DWARFDebugInfo::ParseCompileUnitHeadersIfNeeded() as
  // otherwise DWARFCompileUnit::GetAbbreviations() would have no data.
  DebugInfo().GetNumUnits();
}

// C++14: Use heterogenous lookup instead.
class DwzAsKey {
public:
  DwzAsKey(SymbolFileDWARFDwz &dwz) : ptr(&dwz) {}
  DwzAsKey(FileSpec &fspec) : ptr(&fspec) {}
  DwzAsKey() {}
  SymbolFileDWARFDwz &GetDwz() const { return *ptr.get<SymbolFileDWARFDwz *>(); }
  FileSpec &GetFileSpec() const {
    if (ptr.is<FileSpec *>())
      return *ptr.get<FileSpec *>();
    return GetDwz().GetObjectFile()->GetFileSpec();
  }
  bool operator==(const DwzAsKey &rhs) const {
    return GetFileSpec() == rhs.GetFileSpec();
  }
  bool operator!=(const DwzAsKey &rhs) const { return !(*this == rhs); }
  class Hasher {
  public:
    size_t operator()(const DwzAsKey &key) const {
      return FileSpec::Hasher()(key.GetFileSpec());
    }
  };

private:
  llvm::PointerUnion<SymbolFileDWARFDwz *, FileSpec *> ptr;
};

static std::unordered_map<DwzAsKey, SymbolFileDWARFDwzUP, DwzAsKey::Hasher> dwz_map;
static llvm::sys::RWMutex dwz_map_mutex;

void SymbolFileDWARFDwz::InitializeForDWARF(SymbolFileDWARF &dwarf) {
  const DWARFDataExtractor &section_extractor(
      dwarf.GetDWARFContext().getOrLoadGNUDebugAltLink());
  if (section_extractor.GetByteSize() == 0)
    return; // .gnu_debugaltlink does not exist
  if (dwarf.GetIsDwz()) {
    dwarf.GetObjectFile()->GetModule()->ReportWarning(
        "Error reading DWZ common file - it does contain .gnu_debugaltlink");
    return;
  }
  lldb::offset_t offset = 0;
  const char *link_cstr(section_extractor.GetCStr(&offset));
  if (!link_cstr) {
    dwarf.GetObjectFile()->GetModule()->ReportWarning(
        "Cannot get DWZ common file - missing section .gnu_debugaltlink");
    return;
  }
  lldb::offset_t uuid_bytes_size = section_extractor.BytesLeft(offset);
  const void *uuid_bytes(section_extractor.GetData(&offset, uuid_bytes_size));
  if (!uuid_bytes) {
    dwarf.GetObjectFile()->GetModule()->ReportWarning(
        "Cannot get DWZ common file - missing build-id"
        " in section .gnu_debugaltlink with string \"%s\"",
        link_cstr);
    return;
  }
  UUID link_uuid = UUID::fromOptionalData(uuid_bytes, uuid_bytes_size);
  if (!link_uuid.IsValid()) {
    dwarf.GetObjectFile()->GetModule()->ReportWarning(
        "Cannot get DWZ common file - invalid build-id size %" PRIu64
        " in section .gnu_debugaltlink with string \"%s\"",
        uuid_bytes_size, link_cstr);
    return;
  }
  // For objfile "/usr/lib/debug/usr/bin/true.debug"
  // link_cstr is "../../.dwz/coreutils-8.25-17.fc25.x86_64".
  ModuleSpec dwz_module_spec;
  FileSpec &dwz_fspec = dwz_module_spec.GetFileSpec();
  dwz_fspec = FileSpec(link_cstr);
  dwz_fspec.PrependPathComponent(
      dwarf.GetObjectFile()->GetFileSpec().CopyByRemovingLastPathComponent());
  DwzAsKey dwz_fspec_lookup(dwz_fspec);
  {
    llvm::sys::ScopedReader lock(dwz_map_mutex);
    const auto it = dwz_map.find(dwz_fspec_lookup);
    if (it != dwz_map.end()) {
      it->second->SetForDWARF(dwarf);
      return;
    }
  }
  dwz_module_spec.GetArchitecture() =
      dwarf.GetObjectFile()->GetModule()->GetArchitecture();
  ModuleSP dwz_module = std::make_shared<Module>(dwz_module_spec);
  DataBufferSP dwz_file_data_sp;
  lldb::offset_t dwz_file_data_offset = 0;
  lldb::ObjectFileSP dwz_obj_file = ObjectFile::FindPlugin(
      dwz_module, &dwz_fspec, 0, FileSystem::Instance().GetByteSize(dwz_fspec),
      dwz_file_data_sp, dwz_file_data_offset);
  if (!dwz_obj_file) {
    dwarf.GetObjectFile()->GetModule()->ReportWarning(
        "Cannot get DWZ common file - file \"%s\" cannot be opened",
        dwz_fspec.GetCString());
    return;
  }
  lldbassert(dwz_obj_file->GetFileSpec() == dwz_fspec);
  UUID dwz_uuid = dwz_obj_file->GetUUID();
  if (!dwz_uuid) {
    dwarf.GetObjectFile()->GetModule()->ReportWarning(
        "Cannot get DWZ common file - file \"%s\" does not have build-id",
        dwz_fspec.GetCString());
    return;
  }
  if (link_uuid != dwz_uuid) {
    dwarf.GetObjectFile()->GetModule()->ReportWarning(
        "Cannot get DWZ common file - expected build-id %s but file \"%s\""
        " has build-id %s",
        link_uuid.GetAsString().c_str(), dwz_fspec.GetCString(),
        dwz_uuid.GetAsString().c_str());
    return;
  }
  auto dwz_symbol_file = std::make_unique<SymbolFileDWARFDwz>(
      dwz_obj_file, std::move(dwz_module));
  assert(DwzAsKey(*dwz_symbol_file) == dwz_fspec_lookup);
  {
    llvm::sys::ScopedWriter lock(dwz_map_mutex);
    const auto it = dwz_map.find(dwz_fspec_lookup);
    if (it != dwz_map.end())
      it->second->SetForDWARF(dwarf);
    else {
      dwz_symbol_file->SetForDWARF(dwarf);
      const auto insertpair = dwz_map.emplace(
          DwzAsKey(*dwz_symbol_file), std::move(dwz_symbol_file));
      lldbassert(insertpair.second);
    }
  }
}

void SymbolFileDWARFDwz::SetForDWARF(SymbolFileDWARF &dwarf) {
  lldbassert(&dwarf != this);
  lldbassert(!dwarf.m_dwz_symfile);
  dwarf.m_dwz_symfile = this;
  ++m_use_count;
}

void SymbolFileDWARFDwz::ClearForDWARF(SymbolFileDWARF &main_dwarf) {
  lldbassert(main_dwarf.m_dwz_symfile == this);
  llvm::sys::ScopedWriter lock(dwz_map_mutex);
  lldbassert(m_use_count);
  if (--m_use_count)
    return;
  size_t erased = dwz_map.erase(DwzAsKey(*this));
  lldbassert(erased == 1);
}

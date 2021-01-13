//===-- SymbolFileDWZTests.cpp ----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "gtest/gtest.h"

#include "TestingSupport/TestUtilities.h"

#include "Plugins/ObjectFile/ELF/ObjectFileELF.h"
#include "Plugins/SymbolFile/DWARF/SymbolFileDWARF.h"
#include "Plugins/SymbolVendor/ELF/SymbolVendorELF.h"
#include "lldb/Core/Module.h"
#include "lldb/Host/HostInfo.h"
#include "Plugins/TypeSystem/Clang/TypeSystemClang.h"
#include "lldb/Symbol/SymbolVendor.h"
#include "lldb/Utility/ArchSpec.h"
#include "lldb/Utility/FileSpec.h"

#if defined(_MSC_VER)
#include "lldb/Host/windows/windows.h"
#include <objbase.h>
#endif

#include <algorithm>

using namespace lldb_private;

class SymbolFileDWZTests : public testing::Test {
public:
  void SetUp() override {
// Initialize and TearDown the plugin every time, so we get a brand new
// AST every time so that modifications to the AST from each test don't
// leak into the next test.
#if defined(_MSC_VER)
    ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif

    FileSystem::Initialize();
    HostInfo::Initialize();
    SymbolFileDWARF::Initialize();
    TypeSystemClang::Initialize();
    ObjectFileELF::Initialize();
    SymbolVendorELF::Initialize();

    m_dwztest_out = GetInputFilePath("dwztest.out");
  }

  void TearDown() override {
    SymbolVendorELF::Terminate();
    ObjectFileELF::Terminate();
    TypeSystemClang::Terminate();
    SymbolFileDWARF::Terminate();
    HostInfo::Terminate();
    FileSystem::Terminate();

#if defined(_MSC_VER)
    ::CoUninitialize();
#endif
  }

protected:
  std::string m_dwztest_out;
};

TEST_F(SymbolFileDWZTests, TestSimpleClassTypes) {
  FileSpec fspec(m_dwztest_out);
  ArchSpec aspec("x86_64-pc-linux");
  lldb::ModuleSP module = std::make_shared<Module>(fspec, aspec);

  SymbolFile *symfile = module->GetSymbolFile();
  EXPECT_NE(nullptr, symfile);
  EXPECT_EQ(symfile->GetPluginName(), SymbolFileDWARF::GetPluginNameStatic());
  SymbolContext sc;
  llvm::DenseSet<SymbolFile *> searched_files;
  TypeMap results;
  symfile->FindTypes(ConstString("StructMovedToDWZCommonFile"), CompilerDeclContext(), 0,
                     searched_files, results);
  EXPECT_EQ(1u, results.GetSize());
  lldb::TypeSP udt_type = results.GetTypeAtIndex(0);
  EXPECT_EQ(ConstString("StructMovedToDWZCommonFile"), udt_type->GetName());
  CompilerType compiler_type = udt_type->GetForwardCompilerType();
  EXPECT_TRUE(TypeSystemClang::IsClassType(compiler_type.GetOpaqueQualType()));
}

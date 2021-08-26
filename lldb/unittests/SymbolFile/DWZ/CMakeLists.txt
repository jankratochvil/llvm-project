add_lldb_unittest(SymbolFileDWZTests
  SymbolFileDWZTests.cpp

  LINK_LIBS
    lldbCore
    lldbHost
    lldbSymbol
    lldbPluginSymbolFileDWARF
    lldbUtilityHelpers
    lldbPluginObjectFileELF
    lldbPluginSymbolVendorELF
  LINK_COMPONENTS
    Support
  )

set(test_inputs
   dwztest.out
   dwztest.debug
   dwztest.debug.dwz)

add_unittest_inputs(SymbolFileDWZTests "${test_inputs}")

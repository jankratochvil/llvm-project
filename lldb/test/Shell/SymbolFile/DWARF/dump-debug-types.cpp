// XFAIL: target-arm && linux-gnu
// UNSUPPORTED: system-windows
//
// Test to verify we are correctly generating anonymous flags when parsing
// anonymous class and unnamed structs from DWARF to the a clang AST node.

// RUN: %clangxx_host -g -fdebug-types-section -c -o %t.o %s
// RUN: lldb-test symbols -dump-clang-ast %t.o | FileCheck %s

struct StructName {} a;

// CHECK: StructName

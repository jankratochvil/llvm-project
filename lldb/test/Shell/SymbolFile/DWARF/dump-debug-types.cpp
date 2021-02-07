// Test dumping types does work even for -fdebug-types-section.

// RUN: %clangxx_host -g -fdebug-types-section -c -o %t.o %s
// RUN: lldb-test symbols -dump-clang-ast %t.o | FileCheck %s

struct StructName {
} a;

// CHECK: StructName

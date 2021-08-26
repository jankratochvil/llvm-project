// Test dumping types does work even for -fdebug-types-section.

// error: unsupported option '-fdebug-types-section' for target 'x86_64-apple-darwin20.1.0'
// UNSUPPORTED: system-darwin

// RUN: %clangxx_host -g -fdebug-types-section -c -o %t.o %s
// RUN: lldb-test symbols -dump-clang-ast %t.o | FileCheck %s

struct StructName {
} a;

// CHECK: StructName

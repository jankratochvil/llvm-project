// Test LLDB does not assert on miscalculated field offsets.

// RUN: %clang_host %s -g -c -o %t.o
// RUN: %lldb %t.o -o "p c.c" -o exit | FileCheck %s

// CHECK: (lldb) p c.c
// CHECK-NEXT: (char) $0 = '\0'

struct A {};
struct B {
  [[no_unique_address]] A a;
};
struct C : B {
  char c;
} c;

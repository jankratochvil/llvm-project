// Test listing global variables when dwz -m is not in use.

// REQUIRES: target-x86_64, system-linux, native

// RUN: %clang_host -Wall -g -O2 -c -o %t-1.o %s -DMAIN
// RUN: %clang_host -Wall -g -O2 -c -o %t-2.o %s
// RUN: %clang_host -Wall -o %t %t-1.o %t-2.o
// RUN: (! which eu-strip dwz) || (! test -x /usr/lib/rpm/sepdebugcrcfix) || \
// RUN:   (eu-strip --remove-comment -f %t.debug %t && dwz %t.debug && \
// RUN:     /usr/lib/rpm/sepdebugcrcfix . "$(realpath --relative-to=$PWD %t)")
// RUN: %lldb %t -o 'b main' -o r -o 'target variable -c' \
// RUN:   -o exit | FileCheck %s

// CHECK-LABEL: (lldb) target variable -c
// CHECK: `dwz-global-variable.c:{{[0-9]*}}: (const int) file_static_variable = 42

static const int file_static_variable = 42;
static const int make_it_dwz_worth = 42;
#ifdef MAIN
int func(void);
int main(void) { return func() + file_static_variable + make_it_dwz_worth; }
#else
int func(void) { return file_static_variable + make_it_dwz_worth; }
#endif

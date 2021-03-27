# Workaround for DWZ tool bug producing corrupted DWARF:
# Multifile drops DW_TAG_namespace::DW_AT_export_symbols
# https://sourceware.org/bugzilla/show_bug.cgi?id=27572

# REQUIRES: target-x86_64, system-linux, native

# RUN: %clang_host -o %t %s
# RUN: %lldb %t -o 'b main' -o r -o 'p N::A::m' -o 'p N::m' \
# RUN:   -o exit | FileCheck %s

# CHECK-LABEL: (lldb) p N::A::m
# CHECK-NEXT: (int (*)()) $0 = 0x{{.*}}`N::A::m())
# CHECK-LABEL: (lldb) p N::m
# Failing case was:
# error: <user expression 1>:1:4: no member named 'm' in namespace 'N'
# CHECK-NEXT: (int (*)()) $1 = 0x{{.*}}`N::A::m())

	.text
	.globl	main                            # -- Begin function main
	.type	main,@function
main:                                   # @main
.Lfunc_begin0:
	pushq	%rbp
	movq	%rsp, %rbp
	popq	%rbp
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
                                        # -- End function
	.type	_ZN1N1AL1mEv,@function
_ZN1N1AL1mEv:                           # @_ZN1N1AL1mEv
.Lfunc_begin1:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	$42, %eax
	popq	%rbp
	retq
.Lfunc_end1:
	.size	_ZN1N1AL1mEv, .Lfunc_end1-_ZN1N1AL1mEv
                                        # -- End function
	.section	.debug_abbrev,"",@progbits
.Labbrev0:
	.byte	1                               # Abbreviation Code
	.byte	17                              # DW_TAG_compile_unit
	.byte	1                               # DW_CHILDREN_yes
	.byte	37                              # DW_AT_producer
	.byte	8                               # DW_FORM_string
	.byte	19                              # DW_AT_language
	.byte	5                               # DW_FORM_data2
	.byte	3                               # DW_AT_name
	.byte	8                               # DW_FORM_string
	.byte	17                              # DW_AT_low_pc
	.byte	1                               # DW_FORM_addr
	.byte	18                              # DW_AT_high_pc
	.byte	6                               # DW_FORM_data4
	.byte	0                               # EOM(1)
	.byte	0                               # EOM(2)
	.byte	3                               # Abbreviation Code
	.byte	57                              # DW_TAG_namespace
	.byte	1                               # DW_CHILDREN_yes
	.byte	3                               # DW_AT_name
	.byte	8                               # DW_FORM_string
	.byte	0                               # EOM(1)
	.byte	0                               # EOM(2)
	.byte	4                               # Abbreviation Code
	.byte	57                              # DW_TAG_namespace
	.byte	1                               # DW_CHILDREN_yes
	.byte	3                               # DW_AT_name
	.byte	8                               # DW_FORM_string
	.ascii	"\211\001"                      # DW_AT_export_symbols
	.byte	25                              # DW_FORM_flag_present
	.byte	0                               # EOM(1)
	.byte	0                               # EOM(2)
	.byte	5                               # Abbreviation Code
	.byte	46                              # DW_TAG_subprogram
	.byte	0                               # DW_CHILDREN_no
	.byte	17                              # DW_AT_low_pc
	.byte	1                               # DW_FORM_addr
	.byte	18                              # DW_AT_high_pc
	.byte	6                               # DW_FORM_data4
	.byte	64                              # DW_AT_frame_base
	.byte	24                              # DW_FORM_exprloc
	.byte	0x47                            # DW_AT_specification
	.byte	0x10                            # DW_FORM_ref_addr
	.byte	0                               # EOM(1)
	.byte	0                               # EOM(2)
	.byte	6                               # Abbreviation Code
	.byte	36                              # DW_TAG_base_type
	.byte	0                               # DW_CHILDREN_no
	.byte	3                               # DW_AT_name
	.byte	8                               # DW_FORM_string
	.byte	62                              # DW_AT_encoding
	.byte	11                              # DW_FORM_data1
	.byte	11                              # DW_AT_byte_size
	.byte	11                              # DW_FORM_data1
	.byte	0                               # EOM(1)
	.byte	0                               # EOM(2)
	.byte	7                               # Abbreviation Code
	.byte	0x3c                              # DW_TAG_partial_unit
	.byte	1                               # DW_CHILDREN_yes
	.byte	0                               # EOM(1)
	.byte	0                               # EOM(2)
	.byte	8                               # Abbreviation Code
	.byte	46                              # DW_TAG_subprogram
	.byte	0                               # DW_CHILDREN_no
	.byte	110                             # DW_AT_linkage_name
	.byte	8                               # DW_FORM_string
	.byte	3                               # DW_AT_name
	.byte	8                               # DW_FORM_string
	.byte	58                              # DW_AT_decl_file
	.byte	11                              # DW_FORM_data1
	.byte	59                              # DW_AT_decl_line
	.byte	11                              # DW_FORM_data1
	.byte	73                              # DW_AT_type
	.byte	0x10                            # DW_FORM_ref_addr
	.byte	0                               # EOM(1)
	.byte	0                               # EOM(2)
	.byte	9                               # Abbreviation Code
	.byte	0x3d                            # DW_TAG_imported_unit
	.byte	0                               # DW_CHILDREN_no
	.byte	0x18                            # DW_AT_import
	.byte	0x10                            # DW_FORM_ref_addr
	.byte	0                               # EOM(1)
	.byte	0                               # EOM(2)
	.byte	0                               # EOM(3)
	.section	.debug_info,"",@progbits
.Lcu_begin0:
	.long	.Ldebug_info_end0-.Ldebug_info_start0 # Length of Unit
.Ldebug_info_start0:
	.short	4                               # DWARF version number
	.long	.Labbrev0                       # Offset Into Abbrev. Section
	.byte	8                               # Address Size (in bytes)
	.byte	1                               # Abbrev [1] DW_TAG_compile_unit
	                                        # DW_AT_producer
	.asciz	"clang version 11.0.0 (Fedora 11.0.0-2.fc33)"
	.short	33                              # DW_AT_language
	                                        # DW_AT_name
	.asciz	"-"
	.quad	.Lfunc_begin0                   # DW_AT_low_pc
	.long	.Lfunc_end1-.Lfunc_begin0       # DW_AT_high_pc

	.byte	9                               # Abbrev [0] DW_TAG_imported_unit
	.long	.Lpartial_unit                  # DW_AT_import

	.byte	3                               # Abbrev [3] DW_TAG_namespace
	                                        # DW_AT_name
	.asciz	"N"
	.byte	4                               # Abbrev [4] DW_TAG_namespace
	                                        # DW_AT_name
	.asciz	"A"
                                        # DW_AT_export_symbols
	.byte	0                               # End Of Children Mark
	.byte	0                               # End Of Children Mark
	.byte	5                               # Abbrev [5] DW_TAG_subprogram
	.quad	.Lfunc_begin1                   # DW_AT_low_pc
	.long	.Lfunc_end1-.Lfunc_begin1       # DW_AT_high_pc
	.byte	1                               # DW_AT_frame_base
	.byte	86
	.long	.Lmethod_decl			# DW_AT_specification
.Ltype_int:
	.byte	6                               # Abbrev [6] DW_TAG_base_type
	                                        # DW_AT_name
	.asciz	"int"
	.byte	5                               # DW_AT_encoding
	.byte	4                               # DW_AT_byte_size
	.byte	0                               # End Of Children Mark
.Ldebug_info_end0:

.Lcu_begin1:
	.long	.Ldebug_info_end1-.Ldebug_info_start1 # Length of Unit
.Ldebug_info_start1:
	.short	4                               # DWARF version number
	.long	.Labbrev0                       # Offset Into Abbrev. Section
	.byte	8                               # Address Size (in bytes)
.Lpartial_unit:
	.byte	7                               # Abbrev [7] DW_TAG_partial_unit
	.byte	3                               # Abbrev [3] DW_TAG_namespace
	                                        # DW_AT_name
	.asciz	"N"
	.byte	3                               # Abbrev [3] DW_TAG_namespace
	                                        # DW_AT_name
	.asciz	"A"
                                                # The Bug - missing DW_AT_export_symbols
.Lmethod_decl:
	.byte	8                               # Abbrev [8] DW_TAG_subprogram
	                                        # DW_AT_linkage_name
	.asciz	"_ZN1N1AL1mEv"
	                                        # DW_AT_name
	.asciz	"m"
	.byte	1                               # DW_AT_decl_file
	.byte	1                               # DW_AT_decl_line
	.long	.Ltype_int                      # DW_AT_type (DW_FORM_ref_addr)
	.byte	0                               # End Of Children Mark
	.byte	0                               # End Of Children Mark
	.byte	0                               # End Of Children Mark
.Ldebug_info_end1:

	.section	".note.GNU-stack","",@progbits

""" Testing separate debug info loading for base binary with a symlink. """
import os
import time
import lldb
import sys
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
from lldbsuite.test import lldbutil


class TestTargetSymbolsBuildidSymlink(TestBase):

    mydir = TestBase.compute_mydir(__file__)

    @no_debug_info_test  # Prevent the genaration of the dwarf version of this test
    @skipUnlessPlatform(['linux'])
    @skipIf(hostoslist=["windows"])
    @skipIfRemote # llvm.org/pr36237
    @skipUnlessDWZInstalled
    def test_target_symbols_buildid_symlink(self):
        self.build()
        exe = self.getBuildArtifact("stripped.out")

        lldbutil.run_to_name_breakpoint(self, "main", exe_name = exe)

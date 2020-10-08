LEVEL = ../../make
C_SOURCES := main.c
LD_EXTRAS += -Wl,--build-id=sha1

all: .build-id

.PHONY: .build-id
stripped.out stripped.debug stripped.debug.dwz: a.out
	eu-strip --remove-comment -f stripped.debug -F stripped.debugx -o stripped.out $<
	cp -p stripped.debug stripped.debug.dup
	dwz -m stripped.debug.dwz stripped.debug stripped.debug.dup

.build-id: stripped.debug
	$(OBJCOPY) -j .note.gnu.build-id -O binary $< tmp
	rm -rf .build-id
	fn=`od -An -tx1 <tmp|tr -d ' \n'|sed -e 's/^.\{32\}//' -e 's#^..#.build-id/&/#' -e 's#$$#.debug#'` && \
	mkdir -p `dirname $$fn` && \
	ln -s ../../$< $$fn
	$(RM) tmp

clean::
	$(RM) -r stripped.out .build-id stripped.debug stripped.debug.dup stripped.debug.dwz

include $(LEVEL)/Makefile.rules

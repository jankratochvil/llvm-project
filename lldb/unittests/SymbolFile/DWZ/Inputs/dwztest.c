// gcc -Wall -g -o dwztest.out dwztest.c; eu-strip --remove-comment -f
// dwztest.debug dwztest.out; cp -p dwztest.debug dwztest.debug.dup; dwz -m
// dwztest.debug.dwz dwztest.debug dwztest.debug.dup;rm dwztest.debug.dup;
// /usr/lib/rpm/sepdebugcrcfix . dwztest.out

struct StructMovedToDWZCommonFile {
  int i1, i2, i3, i4, i5, i6, i7, i8, i9;
} VarWithStructMovedToDWZCommonFile;
static const int sizeof_StructMovedToDWZCommonFile =
    sizeof(struct StructMovedToDWZCommonFile);
int main() { return sizeof_StructMovedToDWZCommonFile; }

mofilereader
============

Very small piece of code (compiled as static lib) that you can use to read cross-platform gettext .mo files with no dependencies. The lib is under 60 lines of code.

How to use
==========

~~~c++
#include "moFileReaderLib.h"

moFileReader i18n;

#define _(X) i18n.lookup(X)

int main(int argc, char**argv) {
  // read File into buf
  i18n.readMemory(buf, fsize);
  // ...
  const char* translated = _("hello world!")
}
~~~

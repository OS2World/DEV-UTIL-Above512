# NMAKE-compatible MAKE file for a stand-alone ABOVE512 executable.

ABOVE512.exe:    ABOVE512.obj ABOVE512.def
         ILINK /NOFREE ABOVE512.obj,ABOVE512.exe,,,ABOVE512.def

ABOVE512.obj:    ABOVE512.c
         icc -c  ABOVE512.c


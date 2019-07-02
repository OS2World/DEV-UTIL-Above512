# NMAKE-compatible MAKE file for a stand-alone ABOVE512 Rexx DLL.

ABOVE512.DLL:    Rx512MiB.OBJ Rx512MiB.DEF
         ILINK /NOFREE Rx512MiB.obj,ABOVE512.DLL,,REXX,Rx512MiB.DEF

Rx512MiB.OBJ:    Rx512MiB.C
         icc -c -Ge- -Gm+ Rx512MiB.C


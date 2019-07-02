/* ABOVE512 Rexx DLL, LX format 32-bit DLL module 'loading above 512 MiB'
      marking utility, version 0.01d (internal/experimental use only)
   Copyright 2004 Takayuki 'January June' Suwa. */


#pragma strings(readonly)

#define INCL_BASE
#define INCL_ERRORS
#define INCL_REXXSAA
#define _DLL
#define _MT

#define FOR_EXEHDR 1
#define INVALID_ROUTINE 40
#define OBJHIMEM 0x00010000L  /* Object is loaded above 512 MiB if available */
#define VALID_ROUTINE 0

#include <os2.h>
#include <exe386.h>
#include <rexxsaa.h>
#include <stdio.h>
#include <string.h>

typedef USHORT WORD;
typedef ULONG DWORD;


RexxFunctionHandler Above512;


/*  */
extern APIRET APIENTRY DosReplaceModule(PSZ pszOldModule,
                                        PSZ pszNewModule,
                                        PSZ pszBackupModule);

/*  */
#pragma pack(1)
struct StubHeader_t
{
USHORT usID;              /* == 0x5a4d */
USHORT ausContents1[11];
USHORT usRelOff;          /* == 30 or > 32 */
USHORT ausContents2[17];
ULONG ulNewHeaderOff;
};
#pragma pack()

/*  */
ULONG Above512(CHAR *name, ULONG numargs, RXSTRING args[],
               CHAR *queuename, RXSTRING *retstr)
{
BOOL bModified;
FILESTATUS3 xfs3;
HFILE hfModule;
PSZ pszModule;
PVOID pvBuffer;
static const PSZ apszObjectTypes[]={"swappable","permanent","resident","contiguous","long lockable","","swappable",""};
struct e32_exe* pxLXHeader;
struct o32_obj* pxObjTable;
ULONG ulIndex,ulOptions,ulWork;
unsigned long ulCheckMask,ulCheckPattern,ulModifyMask,ulModifyPattern;

if (numargs<2)
   return INVALID_ROUTINE;

pszModule=(PSZ)NULL;
retstr->strlength=1;
ulOptions=(ULONG)0;
ulCheckMask=(unsigned long)0;
ulCheckPattern=(unsigned long)~0;
ulModifyMask=(unsigned long)0;
ulModifyPattern=(unsigned long)0;
for (ulIndex=(ULONG)0;ulIndex<(ULONG)numargs;ulIndex++)
   if (args[ulIndex].strptr[0]=='/'||args[ulIndex].strptr[0]=='-')
      switch (args[ulIndex].strptr[1])
         {
         case 'C':
         case 'c':
            ulOptions|=(ULONG)1;
            ulCheckMask=OBJEXEC|OBJRSRC|OBJINVALID|OBJALIAS16|OBJBIGDEF|OBJHIMEM;
            ulCheckPattern=OBJEXEC|OBJBIGDEF;
            ulModifyMask=(unsigned long)~0;
            ulModifyPattern=OBJHIMEM;
            break;
         case 'U':
         case 'u':
            ulOptions|=(ULONG)1;
            ulCheckMask=OBJRSRC|OBJINVALID|OBJALIAS16|OBJBIGDEF|OBJHIMEM;
            ulCheckPattern=OBJBIGDEF|OBJHIMEM;
            ulModifyMask=~OBJHIMEM;
            ulModifyPattern=(unsigned long)0;
            break;
         case '!':
            ulOptions|=(ULONG)4;
            break;
         }
   else
      pszModule=(PSZ)args[ulIndex].strptr;
    
if (pszModule==(PSZ)NULL)
   {
   strcpy(retstr->strptr,"1\0");
   return VALID_ROUTINE;
   }                     
if (ulOptions==(ULONG)0)
   {
   strcpy(retstr->strptr,"2\0");
   return VALID_ROUTINE;
   }                     
if ((ulOptions&(ULONG)4)!=(ULONG)0)
   (VOID)DosReplaceModule(pszModule,(PSZ)NULL,(PSZ)NULL);
if (DosOpen(pszModule,
            &hfModule,
            &ulWork,
            (ULONG)0,
            FILE_NORMAL,
            OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
            (ulOptions&(ULONG)1)!=(ULONG)0?OPEN_FLAGS_FAIL_ON_ERROR|OPEN_FLAGS_SEQUENTIAL|OPEN_FLAGS_NOINHERIT|OPEN_SHARE_DENYREADWRITE|OPEN_ACCESS_READWRITE
                                          :OPEN_FLAGS_FAIL_ON_ERROR|OPEN_FLAGS_SEQUENTIAL|OPEN_FLAGS_NOINHERIT|OPEN_SHARE_DENYNONE|OPEN_ACCESS_READONLY,
            (PEAOP2)NULL)!=NO_ERROR)
   {
   strcpy(retstr->strptr,"3\0");
   return VALID_ROUTINE;
   }
(VOID)DosQueryFileInfo(hfModule,FIL_STANDARD,(PVOID)&xfs3,(ULONG)sizeof(xfs3));
(VOID)DosAllocMem(&pvBuffer,xfs3.cbFile,PAG_READ|PAG_WRITE|PAG_COMMIT);
if (DosRead(hfModule,pvBuffer,xfs3.cbFile,&ulWork)!= NO_ERROR||ulWork!=xfs3.cbFile)
   {
   (VOID)DosFreeMem(pvBuffer);
   (VOID)DosClose(hfModule);
   strcpy(retstr->strptr,"4\0");
   return VALID_ROUTINE;
   }
pxLXHeader=(struct e32_exe*)NULL;
switch (((PUSHORT)pvBuffer)[0])
   {
   case (USHORT)0x5a4d:  /* 'MZ' */
      if (((struct StubHeader_t*)pvBuffer)->usRelOff>=(USHORT)sizeof(struct StubHeader_t))
         {
         pxLXHeader=(struct e32_exe*)&((PUCHAR)pvBuffer)[((struct StubHeader_t*)pvBuffer)->ulNewHeaderOff];
         if(((PUSHORT)pxLXHeader)[0] != (USHORT)0x584c)  /* 'LX' */
            pxLXHeader=(struct e32_exe*)NULL;
         }
      break;
   case (USHORT)0x584c:  /* 'LX' */
      pxLXHeader=(struct e32_exe*)pvBuffer;
      break;
   }
if (pxLXHeader==(struct e32_exe*)NULL||pxLXHeader->e32_level!=E32LEVEL||pxLXHeader->e32_cpu<E32CPU386||(pxLXHeader->e32_mflags&(E32NOLOAD|E32MODMASK))!=E32MODDLL)
   {
   (VOID)DosFreeMem(pvBuffer);
   (VOID)DosClose(hfModule);
   strcpy(retstr->strptr,"5\0");
   return VALID_ROUTINE;
   }
bModified=(BOOL)FALSE;
for (ulIndex=(ULONG)0,pxObjTable=(struct o32_obj*)&((PUCHAR)pxLXHeader)[pxLXHeader->e32_objtab];ulIndex<(ULONG)pxLXHeader->e32_objcnt;ulIndex++,pxObjTable++)
   {
   if (((pxObjTable->o32_flags&ulCheckMask)^ulCheckPattern)==(unsigned long)0)
      {
      pxObjTable->o32_flags=(pxObjTable->o32_flags&ulModifyMask)^ulModifyPattern;
      bModified=(BOOL)TRUE;
      }
   }
if (bModified!=(BOOL)FALSE)
   {
   (VOID)DosSetFilePtr(hfModule,(LONG)0,FILE_BEGIN,&ulWork);
   if (DosWrite(hfModule,pvBuffer,xfs3.cbFile,&ulWork)!= NO_ERROR||ulWork!=xfs3.cbFile)
      {
      (VOID)DosFreeMem(pvBuffer);
      (VOID)DosClose(hfModule);
      strcpy(retstr->strptr,"6\0");
      return VALID_ROUTINE;
      }
   }
(VOID)DosFreeMem(pvBuffer);
(VOID)DosClose(hfModule);
strcpy(retstr->strptr,"0\0");
if (bModified!=(BOOL)FALSE&&DosSetPathInfo(pszModule,FIL_STANDARD,(PVOID)&xfs3,(ULONG)sizeof(xfs3),(ULONG)0)!=NO_ERROR)
   strcpy(retstr->strptr,"7\0");
return VALID_ROUTINE;
}


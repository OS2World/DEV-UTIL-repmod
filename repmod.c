//
// Replace locked OS/2 modules (EXE, DLL)
//
// Author: J. van Wijk
//
#define REPMOD_N   "Replace (locked) module"
#define REPMOD_C "(c) 1998; Jan van Wijk"
#define REPMOD_V   "1.04 27-07-98" // Removed debug and trace-code
//efine REPMOD_V   "1.03 28-11-96" // Fixed %ul error; use API for del (todo)
//efine REPMOD_V   "1.02 21-11-96" // Copy to non-existing target; delete only
//efine REPMOD_V   "1.01 29-05-96" // Added copy/delete non-locked modules
//efine REPMOD_V   "1.00 28-05-96" // Initial version

#define  INCL_BASE
#include <os2.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>


APIRET APIENTRY DosReplaceModule                // undocumented, repl locked
(                                               //       OS/2 DLL or EXE mod
   PSZ                 oldmod,                  // IN    Old modulename
   PSZ                 newmod,                  // IN    New modulename
   PSZ                 bakmod                   // IN    Backup module
);


int main (int argc, char *argv[]);
                                                // 
/*****************************************************************************/
/* Main function of the program, handle commandline-arguments                */
/*****************************************************************************/
int main (int argc, char *argv[])
{
   ULONG               rc = 0;
   char               *old = NULL;
   char               *new = NULL;
   char               *sav = NULL;
   BOOL                del = FALSE;
   char                cmd[256];
   int                 confirm;

   if ((argc == 1) || (*argv[1] == '?'))        // help wanted
   {
      printf("\nUsage: %s   oldmodule [newmodule | -d | -u   [savemodule]]\n",
              argv[0]);
      printf("\n     oldmodule = Modulename (EXE, DLL) to be replaced\n");
      printf(  "     newmodule = New module to replace oldmodule.\n");
      printf(  " or    -d        Unlock and delete oldmodule.\n");
      printf(  " or    -u        Just unlock oldmodule, no delete or replace.\n");
      printf(  "    savemodule = Filename to receive backup of oldmodule\n");
      printf("\nExamples:\n");
      printf("\n Replace a dll:    %s  x:\\dll\\test.dll x:\\develop\\test.dll\n", argv[0]);
      printf(  " Delete and Save:  %s  x:\\dll\\test.dll -d  x:\\dll\\test.org\n", argv[0]);
      printf(  " Unlock only       %s  x:\\dll\\test.dll\n",                       argv[0]);
      printf("\n  The newmodule will replace the oldmodule, even if it\n");
      printf(  "  is in-use (locked). The contents will be moved to the\n");
      printf(  "  swapfile for later use. The newmodule will be loaded\n");
      printf(  "  only after all users have released the oldmodule.\n");
      printf(  "  This is usualy only guaranteed after a reboot.\n");
      printf("\n  %s; %s  %s\n", REPMOD_N, REPMOD_V, REPMOD_C);
   }
   else
   {
      if (argc > 1)
      {
         old = argv[1];
         if (argc > 2)
         {
            if      ((*argv[2] == '-') && (toupper(argv[2][1]) == 'D'))
            {
               printf("Delete (locked) '%s'\n", old);
               del = TRUE;
            }
            else if ((*argv[2] == '-') && (toupper(argv[2][1]) == 'U'))
            {
               printf("Unlock module '%s'\n", old);
            }
            else
            {
               new = argv[2];
               printf("Replace (locked) '%s' by new module '%s'\n", old, new);
            }
            if (argc > 3)
            {
               sav = argv[3];
               printf("Saving backup copy of '%s' as '%s'\n", old, sav);
            }
         }
         else
         {
            printf("Unlock module '%s'\n", old);
         }
      }
      if (old != NULL)
      {
         rc = DosReplaceModule( old, new, sav);
         switch (rc)
         {
            case NO_ERROR:
               if (new == NULL)                 // API just unlocks, delete mod
               {
                  if (del == TRUE)
                  {
                     sprintf( cmd, "del %s > NUL", old);
                     rc = system( cmd);
                     if (rc == 0)
                     {
                        printf("Locked OS/2 module deleted successfully\n");
                     }
                     else
                     {
                        printf("Delete OS/2 module %s failed, rc=%lu\n", old, rc);
                     }
                  }
                  else
                  {
                     printf("Locked OS/2 module unlocked successfully\n");
                  }
               }
               else
               {
                  printf("Locked OS/2 module replaced successfully");
               }
               break;

            case ERROR_FILE_NOT_FOUND:
               printf("Module '%s' does not exist yet, ", old);
               if (sav != NULL)
               {
                  printf("no save possible; ");
               }
               if (new != NULL)
               {
                  printf("executing copy '%s' ...\n", new);
                  sprintf( cmd, "copy %s %s > NUL", new, old);
                  rc = system( cmd);
               }
               else
               {
                  printf("no unlock or delete possible.\n");
                  rc = 0;
               }
               break;

            case ERROR_MODULE_IN_USE:
               printf("Module '%s' is not locked, ", old);
               if (sav != NULL)
               {
                  sprintf( cmd, "copy %s %s > NUL", old, sav);
                  rc = system( cmd);
               }
               if (new != NULL)
               {
                  printf("executing copy '%s' ...\n", new);
                  sprintf( cmd, "copy %s %s > NUL", new, old);
                  rc = system( cmd);
               }
               else if (del == TRUE)
               {
                  printf("executing delete '%s' ...\n", old);
                  sprintf( cmd, "del %s > NUL", old);
                  rc = system( cmd);
               }
               else
               {
                  printf("no unlock needed.\n");
               }
               break;

            default:
               printf("Replace/delete locked OS/2 module failed, rc=%lu\n", rc);
               break;
         }
         switch (rc)
         {
            case ERROR_FILE_NOT_FOUND:
               printf("File not found\n");
               break;

            case ERROR_PATH_NOT_FOUND:
               printf("Path not found\n");
               break;

            case ERROR_INVALID_PATH:
               printf("Invalid path, no wildcards allowed\n");
               break;

            case ERROR_ACCESS_DENIED:
               printf("Access denied\n");
               break;

            case ERROR_SHARING_VIOLATION:
               printf("File is locked\n");
               break;

            case ERROR_INVALID_DRIVE:
               printf("Drive is invalid\n");
               break;

            case ERROR_WRITE_PROTECT:
               printf("Drive/diskette is write-protected\n");
               break;

            case ERROR_FILENAME_EXCED_RANGE:
               printf("Invalid filename, probably on FAT\n");
               break;

            case ERROR_INVALID_PARAMETER:
               printf("Incorrect parameter\n");
               break;

            default:
               break;
         }
      }
   }
   return(rc);
}
/*---------------------------------------------------------------------------*/


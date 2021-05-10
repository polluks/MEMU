/* main.c - Entry point for builds with a full operating system */

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#define BOOLEAN BOOLEANx
#include <windows.h>
#include <direct.h>
#define chdir(x) _chdir(x)
#undef BOOLEAN
#else
#include <unistd.h>
#endif
#include "types.h"

//  Test for a regular file

#ifdef WIN32
#define S_ISREG(mode) (mode & _S_IFREG)
#endif

BOOLEAN cfg_test_file (const char *psPath)
    {
    struct stat st;
    int iErr = stat (psPath, &st);
    return ( ( iErr == 0 ) && ( S_ISREG (st.st_mode) ) );
    }

char * cfg_exe_path (void)
    {
#ifdef UNIX    
    static const char *psLink = "/proc/self/exe";
    struct stat st;
    char *psPath = NULL;
    ssize_t n;
    if ( lstat(psLink, &st) == -1 ) return NULL;
    while (1)
        {
        psPath = realloc (psPath, st.st_size + 1);
        if ( psPath == NULL ) return NULL;
        n = readlink (psLink, psPath, st.st_size + 1);
        if ( n == -1 )
            {
            free (psPath);
            return NULL;
            }
        if ( n <= st.st_size ) break;
        st.st_size = n;
        }
    psPath[n] = '\0';
    return psPath;
#endif
#ifdef WIN32
    char *psPath = malloc (MAX_PATH + 1);
    DWORD n;
    if ( psPath == NULL ) return NULL;
    n = GetModuleFileName (NULL, psPath, MAX_PATH + 1);
    if ( ( n == 0 ) || ( GetLastError () == ERROR_INSUFFICIENT_BUFFER ) )
        {
        free (psPath);
        return NULL;
        }
    return psPath;
#endif
    }

int memu (int argc, const char **argv);

int main (int argc, const char *argv[])
    {
    const char *argvn[] = {argv[0], "-config-file", "memu0.cfg", "-config-file", "memu.cfg"};
    if ( argc == 1 )
        {
        char *psExe = cfg_exe_path ();
        char *psDEnd = NULL;
        if ( psExe != NULL )
            {
            psDEnd = strrchr (psExe, '/');
#ifdef WIN32
            char *psD2 = strrchr (psExe, '\\');
            if ( psD2 > psDEnd ) psDEnd = psD2;
#endif
            if ( psDEnd != NULL )
                {
                *psDEnd = '\0';
                chdir (psExe);
                }
            if ( cfg_test_file (argvn[4]) )
                {
                if ( cfg_test_file (argvn[2]) )
                    {
                    argc = 5;
                    }
                else
                    {
                    argc = 3;
                    argvn[2] = argvn[4];
                    }
                argv = argvn;
                }
            else
                {
                fprintf (stderr, "*** No configuration files found ***\n");
                }
            free (psExe);
            }
        else
            {
            fprintf (stderr, "*** Program folder not found ***\n");
            }
        }
    return memu (argc, argv);
    }

//  Find the configuration file
#if 0
#ifdef __circle__
char * cfg_find_file (const char *argv0)
    {
    static char *psFile = "/memu.cfg";
    return psFile;
    }
#else
//  Combine directory and file names
static char *cfg_make_path (const char *psDir, char chDirSep, const char *psFile)
    {
    char * psPath;
    if ( psDir != NULL )
        {
        int nDir = strlen (psDir);
        int nPath = nDir + strlen (psFile) + 2;
        psPath = emalloc (nPath);
        strcpy (psPath, psDir);
        psPath[nDir] = chDirSep;
        strcpy (&psPath[nDir+1], psFile);
        }
    else
        {
        psPath = estrdup (psFile);
        }
    return psPath;
    }

//  Find the configuration file
char * cfg_find_file (const char *argv0)
    {
    char *psCfg, *ps1, *ps2;
    char *psPath = getenv ("PATH");
    int nProg = strlen (argv0);
    char *psProg = (char *) emalloc (nProg+4);
    char chPathSep, chDirSep;
    // printf ("argv0 = %s\n", argv0);
    // printf ("psPath = %s\n", psPath);
    // printf ("nProg = %d\n", nProg);

    /* Name of configuration file */

    if ( argv0 == NULL ) return NULL;
    strcpy (psProg, argv0);
    if ( strcasecmp (&psProg[nProg - 4], ".exe") == 0 )
        {
        nProg -= 4;
        psProg[nProg] = '\0';
        }
    strcpy (&psProg[nProg], ".cfg");
    nProg += 4;
    // printf ("psProg = %s\n", psProg);

    /* Guess OS */

    if ( ( strchr (argv0, '\\') != NULL ) || ( ( psPath != NULL ) && ( strchr (psPath, ';' ) != NULL ) ) )
        {
        // printf ("Windows OS\n");
        chPathSep = ';';
        chDirSep = '\\';
        }
    else
        {
        // printf ("Linux OS\n");
        chPathSep = ':';
        chDirSep = '/';
        }

    /* Direct location */

    if ( strchr (psProg, chDirSep) != NULL )
        {
        if ( cfg_test_file (psProg) )
            {
            // printf ("Found config file.\n");
            return psProg;
            }
        if ( ( psProg[0] == chDirSep ) || ( ( psProg[1] == ':' ) && ( psProg[2] == '\\' ) ) )
            {
            // printf ("Absolute path and no config file.\n");
            free ((void *) psProg);
            return NULL;
            }
        }

    /* Search path */

    if ( psPath == NULL ) return NULL;
    psPath = estrdup (psPath);
    ps2 = psPath - 1;
    while ( ps2 != NULL )
        {
        ps1 = ps2 + 1;
        ps2 = strchr (ps1, chPathSep);
        if ( ps2 != NULL ) *ps2 = '\0';
        // printf ("Try folder = %s\n", ps1);
        psCfg = cfg_make_path (ps1, chDirSep, psProg);
        if ( cfg_test_file (psCfg) )
            {
            // printf ("Found %s\n", psCfg);
            free ((void *) psPath);
            free ((void *) psProg);
            return psCfg;
            }
        free ((void *) psCfg);
        }
    free ((void *) psPath);
    free ((void *) psProg);
    // printf ("No configuration file.\n");
    return NULL;
    }
#endif
#endif

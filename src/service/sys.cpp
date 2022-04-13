#include "sys.h"
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <sys/param.h>
#include <mach-o/dyld.h> /* _NSGetExecutablePath : must add -framework CoreFoundation to link line */
#include <string.h>
#endif

int get_app_path (char *pname, int pathsize)
{
    long result = 0;

#ifdef __linux__

    /* Oddly, the readlink(2) man page says no NULL is appended. */
    /* So you have to do it yourself, based on the return value: */
    pathsize --; /* Preserve a space to add the trailing NULL */
    result = readlink("/proc/self/exe", pname, pathsize);
    if (result > 0)
    {
        pname[result] = 0; /* add the #@!%ing NULL */

        if ((access(pname, 0) == 0))
            return 0; /* file exists, return OK */
        /*else name doesn't seem to exist, return FAIL (falls
through) */
    }
#endif /* LINUX */

#ifdef _WIN32
    result = GetModuleFileNameA(NULL, pname, pathsize);
    if (result > 0)
    {
        /* fix up the dir slashes... */
        int len = strlen(pname);
        int idx;
        for (idx = 0; idx < len; idx++)
        {
            if (pname[idx] == '\\') pname[idx] = '/';
        }
        if ((_access(pname, 0) == 0))
            return 0; /* file exists, return OK */
        /*else name doesn't seem to exist, return FAIL (falls
through) */
    }
#endif /* WIN32 */

#ifdef SOLARIS
    char *p = getexecname();
    if (p)
    {
        /* According to the Sun manpages, getexecname will
"normally" return an */
        /* absolute path - BUT might not... AND that IF it is not,
pre-pending */
        /* getcwd() will "usually" be the correct thing... Urgh!
*/

        /* check pathname is absolute (begins with a / ???) */
        if (p[0] == '/') /* assume this means we have an
absolute path */
        {
            strncpy(pname, p, pathsize);
            if ((access(pname, 0) == 0))
                return 0; /* file exists, return OK */
        }
        else /* if not, prepend getcwd() then check if file
exists */
        {
            getcwd(pname, pathsize);
            result = strlen(pname);
            strncat(pname, "/", (pathsize - result));
            result ++;
            strncat(pname, p, (pathsize - result));

            if ((access(pname, 0) == 0))
                return 0; /* file exists, return OK */
            /*else name doesn't seem to exist, return FAIL
(falls through) */
        }
    }
#endif /* SOLARIS */

#ifdef __APPLE__ /* assume this is OSX */
    /*
from http://www.hmug.org/man/3/NSModule.html

extern int _NSGetExecutablePath(char *buf, uint32_t *bufsize);

_NSGetExecutablePath copies the path of the executable
into the buffer and returns 0 if the path was successfully
copied in the provided buffer. If the buffer is not large
enough, -1 is returned and the expected buffer size is
copied in *bufsize. Note that _NSGetExecutablePath will
return "a path" to the executable not a "real path" to the
executable. That is the path may be a symbolic link and
not the real file. And with deep directories the total
bufsize needed could be more than MAXPATHLEN.
*/
    int status = -1;
    uint32_t ul_tmp = pathsize;
    char *given_path = (char*)malloc(MAX_PATH * 2);
    if (!given_path) return status;

    pathsize = MAX_PATH * 2;
    result = _NSGetExecutablePath(given_path, &ul_tmp);
    if (result == 0)
    { /* OK, we got something - now try and resolve the real path...
*/
        if (realpath(given_path, pname) != NULL)
        {
            if ((access(pname, 0) == 0))
                status = 0; /* file exists, return OK */
        }
    }
    free (given_path);
    return status;
#endif /* APPLE */

    return -1; /* Path Lookup Failed */
} /* where_do_I_live */


#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <login_cap.h>

#include <skalibs/sgetopt.h>
#include <skalibs/strerr.h>
#include <skalibs/exec.h>

#define PROG "setclasscontext"
#define USAGE PROG " [ -i | -I ] [ -P ] [ -N ] [ -L ] [ -R ] [ -M ] [ -E [ -u ] ] [--] class prog..."
#define dieusage() strerr_dieusage(100, USAGE)

int main (int argc, char const *const *argv)
{
  char ignore = 0;
  char env = 0;
  char unset = 0;
  char do_opts = 1;
  
  unsigned int flags = 0;

  argc--; argv++;
  while (do_opts) {
    if (2 > argc) dieusage();
    if ('-' != **argv) break;
    if ('-' == (*argv)[1] && 0 == (*argv)[2]) {
      do_opts = 0;
      break;
    }
      
    const char *o;
    for (o=argv[0]; *++o && do_opts;)
      switch (*o) {
      case 'E' : env = 1 ; break;
      case 'u' : unset = 1 ; break;
      case 'i' : ignore = 0 ; break;
      case 'I' : ignore = 1 ; break;
      case 'P' : flags |= LOGIN_SETPATH  ; break;
      case 'N' : flags |= LOGIN_SETPRIORITY  ; break;
      case 'L' : flags |= LOGIN_SETRESOURCES  ; break;
      case 'R' : flags |= LOGIN_SETRTABLE  ; break;
      case 'M' : flags |= LOGIN_SETUMASK  ; break;
      default: dieusage();
      }
    argc--; argv++;
  }
  if (!flags) {
    strerr_warn("nothing to set");
    if (ignore) goto end;
    exit(103);
  }
  const char *class = env ? getenv(*argv) : *argv;
  {
    if (!class) {
      if (ignore) goto end;
      strerr_die(101, "environment variable '", *argv, "' not set");
    }
    if (!*class) class = "default";
    if (unset && env) unsetenv(*argv);
    argc--; argv++;
    if (setclasscontext(class, flags)) {
      if (ignore) goto end;
      strerr_die(102, "setclasscontext failed: ", strerror(errno));
    }
  }

 end:
  xexec(argv);
}
 

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <skalibs/strerr.h>
#include <skalibs/exec.h>
#include <skalibs/sgetopt.h>

#define PROG "explode_dir"
#define USAGE PROG " [ -b basedelim ] [ -d delim ] [ -p portdelim ] [--] class prog..."
#define dieusage() strerr_dieusage(100, USAGE)

#define NUL '\0'
#define COMMA ,
/* #define LC(x) ((x)|('a'-'A')) */
#define UC(x) ((x)&~('a'-'A'))

/* enum state { */
/*   NEW_DIR, */
/*   IN_BASE, */
/*   IN_EXT, */
/*   IN_NETCFG, */
/*   IN_PROTO, */
/*   IN_VER, */
/*   IN_ADDR, */
/* } ; */

/* #define IN(c,s) {\ */
    
enum parts { Separator, EndCmd, NetPart };

// The flat parameter arrays mapped to your enum indices
const char **newstyle	      = (const char *[]){ "_-.", "_",  ":"  } ;
const char *const tosubpath[] = { "_-",  "_-", ""   } ;
const char *const toip[]      = { "",    "",   "-_" } ;
const char *const *rules;

const char *const v6chars = "0123456789:abcdef";
const char *const v4chars = ".0123456789";
/* a full netspec is:
 * :tcp:ipv4:192.168.1.0+28:192.168.1.5:5000-6000
 * or
 * :udp:ipv6_B16::0BOE:0+112_:_B16::0B0E:f00D_:domain-http
 */
/*
 * however, I'm currently planning to impelement
 * 192.168.1.1:53
 */

int main(int argc, char const *const *argv) {
  {
    const char *av = argc && argv ? strrchr(argv[0], '/') : "/<undef>";
    av = av ? av+1 : argv[0] ? argv[0] : "<NULL>";
    if (strcmp("pwd-to-subpath", argv[0])) rules = tosubpath ; 
    else if (strcmp("pwd-to-ip", argv[0])) rules = toip ;
    else rules = newstyle ;
  }
  if (*newstyle) {
    subgetopt l = SUBGETOPT_ZERO ;
    for (;;)
    {
      int opt = subgetopt_r(argc, (char const *const *)argv, "b:d:p:", &l) ;
      if (opt == -1) break ;
      switch (opt)
      {
      case 'd': newstyle[Separator] = l.arg ; break ;
      case 'b': newstyle[EndCmd] = l.arg ; break ;
      case 'p': newstyle[NetPart] = l.arg ; break ;
      default: dieusage() ;
      }
    }
    argc -= l.ind ; argv += l.ind ;
  }
  char *path = getcwd(NULL, 0) ;
  if (!path) strerr_diefu1sys(111, "getcwd failed") ;
  if (!*path) strerr_dief(111, "getcwd returned an empty path") ;
  if ('/' != *path) {
    strerr_warn("getcwd gave a relative path: '", path, "'");
    char *t = path;
    path = realpath(path, NULL);
    if (!path) strerr_diefu1sys(111, "realpath failed") ;
    if (!*path) strerr_dief(111, "realpath returned an empty path") ;
    if ('/' != *path)
      strerr_warn("realpath gave a relative path: '", path, "'");
  }
  char *pcpy = strdup(path) ;
  if (!pcpy) strerr_diefu2sys(111, "strdup failed to copy", path) ;
  {
#define PARSEPART \
    X(base) X(basename) X(exts) X(addrpart) X(portpart) X(topart) X(addr)
#define PARSECOUNT X(l) X(ext)
    char *gotaddr = NULL ;
    char *addrtype = NULL ;
    char *lastslash = NULL ;
#define X(part) char * part ;
    PARSEPART ;
#undef X
#define X(count) uint count ;
    PARSECOUNT ;
#undef X
    size_t i, unwind = 1;
    for (i = l = 0; path[i]; i++, l++) {
      if ('/' == path[i]) {
	if (path[1+i]) {
	  lastslash && (*lastslash='/') ;
	  *(lastslash = &path[i]) = NUL ;
	    goto reset;
	} else {
	  if (!i)
	    strerr_die(100, "should be called from a service directory") ;
	  /* huh? a trailing slash. never mind */
	  goto unwind;
	}
	continue;
      }
      if (addrpart) {
	if (strchr(addrtype, UC(path[i]))) continue;
	if (strchr(rules[NetPart], path[i])
	    || strchr(rules[Separator], path[i])) {
	  if (addrpart == &pcpy[i]) { /* haven't started */
	    pcpy[i] = NUL;
	    addrpart++;
	    continue;
	  }
	  pcpy[i] = NUL ;
	  gotaddr = addrpart ;
	  addr = NULL ;
	  goto unwind ;
	}
	/* uh-oh, this isn't an address. lets try again as a port */
	unwind = &pcpy[i] - addrpart ;
	portpart = addrpart ;
	addrpart = NULL ;
	goto unwind ;
      }
      if (strchr(rules[NetPart], pcpy[i])) {
	pcpy[i] = 0;
	if (toip == rules) {
	  addrpart = &pcpy[i]+1;
	  continue;
	}
	if (portpart && toip != rules) {
	  strerr_warn("garbage at end of net spec '", &pcpy[i], "'");
	  pcpy[i] = rules[Separator][0];
	  goto unwind;
	}
      }
      continue;
    reset: /*damn, a subpath*/
#define X(part) part = NULL;
      PARSEPART ;
#undef X
#define X(count) count = 0;      
      PARSECOUNT ;
#undef X
      continue;
    unwind:
      i -= unwind; l -= unwind; unwind = 1;
      
    }
  }
  /* { */
  /*   char *lastslash = NULL; */
  /*   char *basename; */
  /*   char *base; */
  /*   const char *const netspec = {{{"", "T", "TCP", "Tcp", "t", "tcp"}, */
  /* 				  {"U", "UDP", "Udp", "u", "udp"}}, */
  /* 				 {{"", "-", ":", "_"}}, */
  /* 				 {{"", "4", "i4", "ip4", "ipv4"}, */
  /* 				  {"6", "i6", "ip6", "ipv6"}}, */
  /* 				 {{"", "-", ":", "_"}}, */
  /* 				 {{""}, */
  /* 				  {"\b.\b.\b.\b"}, */
  /* 				  {"\b.\b.\b.\b^\a"}, */
				 
  /* 				 {{"", "-", ":", "_"}}, */
      
    
  /*   int state = NEW_DIR ; */
  /*   for (char *p = path, char *c = pcpy; *p; ++p) { */
  /*     if ('/' == *c) { */
  /* 	lastslash && *lastslash = '/'; */
  /* 	lastslash = p; */
  /* 	*p = NULL; */
  /* 	state = NEW_DIR; */
  /*     } */
  /*     if (NEW_DIR == state) { */
  /* 	basename = p; */
  /* 	base = c; */
  /* 	state == IN_BASE; */
  /*     } */
      
		       
	    
  
}

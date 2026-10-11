#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <skalibs/strerr.h>
#include <skalibs/exec.h>
#include <skalibs/sgetopt.h>

#include "servicedirparse.i"

#define PROG "explode_dir"
#define USAGE PROG " [ -b basedelim ] [ -d delim ] [ -p portdelim ] [--] class prog..."
#define dieusage() strerr_dieusage(100, USAGE)

#define NUL '\0'

/* a full netspec is:
 * :tcp:ipv4:192.168.1.0+28:192.168.1.5:5000-6000
 * or
 * :udp:ipv6_B16::0BOE:0+112_:_B16::0B0E:f00D_:domain-http
 */
/*
 * however, I'm currently planning to impelement
 * 192.168.1.1:53
 */

const char* validatepart(struct_parts parts[], enum_parts p, uint i) {
  struct_parts part = parts[p];
  switch (p) {
  case basename_n:
    break;
  case base_n:
    {
      parts[basename_n].part = part.part;
      parts[basename_n].len = part.len;
      { const char *c = validatepart(parts, basename_n, i); if (c) return c; }
    }
    break;
  case invalid_n:
    strerr_dief(100, part.part); 
  default:
    strerr_warn("unhandled ", part.name);
  }
  return NULL;
}


struct parsed *doparse(char *const path, Rules const rules, struct parsed *parsed) {
  char *pcpy = strdup(path) ;
  if (!pcpy) strerr_diefu2sys(111, "strdup failed to copy", path) ;
  {
    enum_addrtype addrtype = 0 ;
  }
  
#define NEXTPARTIS(_part) {						\
    if (parts_END == _part) parts[parts_END].parts_end.next = curpart;	\
    do {								\
      if (parts[curpart].part &&					\
	  (parts[invalid_n].part = validatepart(parts, curpart, i))) {	\
	curpart = invalid_n;						\
	break;								\
      }									\
      ++curpart;							\
      if (_part < curpart) curpart = _part ;				\
    } while ( _part > curpart) ;					\
    if (pn || invalid_n == curpart) continue ;				\
    break;								\
  }
#define NEXTPART NEXTPARTIS(parts[curpart].access.next)
  
  char *lastslash = NULL ;
  uint i, unwind = 1 ;
  enum_parts curpart = start_n ;
  for (i = 0; curpart < parts_END; i++, parts[curpart].part && parts[curpart].len++) {
    char p, UP, pn, pp;
    goto rewound;
  unwind:
    i -= unwind;
    parts[curpart].len = ( unwind >= parts[curpart].len )
      ? 0 : parts[curpart].len - unwind ;
    parts[curpart].len || ( parts[curpart].part = NULL ) ;
    unwind = 1;

  rewound:
    p = path[i] ;		/* current char */
    pn = path[i+1] ;		/* look ahead */
    pp = i ? pcpy[i-1] : 0 ;	/* look behind */

    if (!p) NEXTPARTIS(parts_END);
	      
      /* path componets reset everything. we might have wasted time
       * processing it, but probably not much longer than strrchr-ing
       * it first
       */
    if ('/' == p) {
      if (pn) {
	lastslash && (*lastslash='/') ;
	*(lastslash = &path[i]) = NUL ;
	goto reset;
      } else {
	if (!i) strerr_die(100, "should not be run from /") ;
	path[i] = pcpy[i] = NUL;
	goto unwind;
      }
    }
    if (invalid_n == curpart) {
      if (!p) strerr_die(100, "directory name was invalid: ", parts[invalid_n].part);
      else continue ; }/* looking for a / to reset everything	*/
    
    if (strchr(rules[NetPart], p)) {
      if (netspec_n > curpart) { /* we are now  */
	pcpy[i] = '/' ;
	NEXTPARTIS(netspec_n) ;
      }
      if (!parts[curpart].part) {
	NEXTPART;
      }
    }
    
      /*
       * Advance to the next netspec if the current isn't implemented.
       *
       * There is a risk that as functionality is added, it might
       * change how a spec is parsed.
       *
       * I *think* this should be backwards compatability safe as it
       * comes after manual net spec jumps, which are the only
       * "forwards" compatability feature that nbeeds to be considered,
       * given the no backtrack fail forward design which means each
       * netcomponent is incompatible with its predecessors
       * (except toport)
       *
       * This assumption will fail if a path includes an unimplemented
       * netspec feature, but forward compatibility of yet to be
       * implemented features would require implementing them, so the
       * program would fail in that case anyway.
       */
    for(;unimplemented & curpart;curpart++)
      strerr_warn("skipping ", parts[curpart].name);
    
    if (start_n == curpart) {	/* should be base_n */
      if (strchr(rules[EndCmd], p)) {
	pcpy[i] = NUL;
	NEXTPART;
      }
      if (!parts[curpart].part)
	parts[curpart].part = &path[i];
      continue;
    }

    if (exts_n == curpart) {
      if (strchr(rules[Separator], p)) {
	pcpy[i] = NUL ;
	NEXTPART ;
      }
      if (!parts[curpart].part) {
	parts[curpart].part = &pcpy[i];
	if (!parts[exts_n].exts.start)
	  parts[exts_n].exts.start = &pcpy[i];
      }
    }
    strerr_warn("continuring for ", parts[curpart].name); 
    continue;
 reset: /*damn, a subpath*/
    curpart = 0;
    for (int i = 0; part_NULL > i; ++i) {
      parts[i].part = NULL;
      parts[i].len = 0;
      /* NB. modifications to pcpy haven't been reset, but the path part
       * should be discarded anyway
       */
    }
    pcpy[i] = NUL;
    curpart = start_n;
    continue;
  } /* for */

#include <stdio.h>
  printf("we escaped at position %u: %d\n", i, curpart);
  for (enum_parts j=0; curpart >= j; j++)
    printf("parts[%u](%s) = '%s'[%u]\n", j, parts[j].name, parts[j].part, parts[j].len);
  
  for (struct parsed_field const *f = &parsed->fields[0]; f ;++f) {
    printf("asked for %s(%u)\n", f->name, f->part);
  };
  return parsed;
  
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
    /* /\* So we check the parts from last to first so we don;t waste	*\/ */
    /* /\* time on completed components					*\/ */
      /* if (pending == addrpart) */
      /* if (addrpart) { */
      /* 	if (strchr(rules[NetPart], p) */
      /* 	    ||  strchr(rules[Separator], p)) { */
      /* 	  if (!addrtype) { /\* haven't started *\/ */
      /* 	    pcpy[i] = '/' ; */
      /* 	    addrpart++; */
      /* 	    /\* OK, so we're just going to pretend it didn't happen	*\/ */
      /* 	    /\* later versions may use it to explicitly advance to the 	*\/ */
      /* 	    /\* next componet of a net spec.				*\/ */
      /* 	    continue; */
      /* 	} */
      /* 	pcpy[i] = '/' ; */

	
      /* 	goto unwind ; */
      /* } */
      /* if (strchr(rules[Separator], p)) { */
      /* 	if (!addrtype) {/\* don't know what we are yet *\/ */
      /* 	  addrtype = v4orv6; */
      /* 	  continue ; */
      /* 	} */
	
	

      /* } */
      /* if (strchr(addrchars[addrtype], UC(path[i]))) continue ; */
      /* for (enum AddrType t = v4; t; t = v4 == t ? v6 : 0) */
      /* 	if (!strchr(addrchars[t], UC(path[i]))) addrtype &= ~t; */
      /* if (!addrtype) */
      /* 	/\* uh-oh, this isn't an address. lets try again as a port *\/ */
      /* 	unwind = &pcpy[i] - addrpart ; */
      /* 	portpart = addrpart ; */
      /* 	addrpart = NULL ; */
      /* 	goto unwind ; */
      /* } */
      /* if (strchr(rules[NetPart], pcpy[i])) { */
      /* 	pcpy[i] = '/'; */
      /* 	if (toip == rules) { */
      /* 	  addrpart = &pcpy[i]+1; */
      /* 	  continue; */
      /* 	} */
      /* 	if (portpart && toip != rules) { */
      /* 	  strerr_warn("garbage at end of net spec '", &pcpy[i], "'"); */
      /* 	  pcpy[i] = rules[Separator][0]; */
      /* 	  goto unwind; */
      /* 	} */
      /* } */
int main(int argc, char const *const *argv) {
  char *path = NULL;
  {
    const char *av = argc && argv ? strrchr(argv[0], '/') : "/<undef>";
    av = av ? av+1 : argv[0] ? argv[0] : "<NULL>";
    if (strcmp("pwd-to-subpath", argv[0])) rules = &tosubpath ; 
    else if (strcmp("pwd-to-ip", argv[0])) rules = &toip ;
    else rules = &newstyle ;
  }
  if (*newstyle) {
    subgetopt l = SUBGETOPT_ZERO ;
    for (;;)
    {
      int opt = subgetopt_r(argc, (char const *const *)argv, "b:d:p:P:", &l) ;
      if (opt == -1) break ;
      switch (opt)
      {
      case 'd': newstyle[Separator] = l.arg ; break ;
      case 'b': newstyle[EndCmd] = l.arg ; break ;
      case 'p': newstyle[NetPart] = l.arg ; break ;
      case 'P': path = (char *)l.arg ; break ;
      default: dieusage() ;
      }
    }
    argc -= l.ind ; argv += l.ind ;
  }
  if (!path) {
    path = getcwd(NULL, 0) ;
    if (!path) strerr_diefu1sys(111, "getcwd failed") ;
    if (!*path) strerr_dief(111, "getcwd returned an empty path") ;
  }
  if ('/' != *path) {
    strerr_warn("getcwd gave a relative path: '", path, "'") ;
    path = realpath(path, NULL) ;
    if (!path) strerr_diefu1sys(111, "realpath failed") ;
    if (!*path) strerr_dief(111, "realpath returned an empty path") ;
    if ('/' != *path)
      strerr_warn("realpath gave a relative path: '", path, "'");
  }
  static struct parsed parsed = {NULL, {{base_n, "base"},{dir_n,"dirname"},{0}}};
  doparse(path, *rules, &parsed);
}


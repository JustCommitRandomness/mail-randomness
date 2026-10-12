#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

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

const char* validatepart(struct_parts (*parts)[], enum_parts p, uint i) {
  struct_parts part = (*parts)[p] ;
  switch (p) {

  case basename_n:
    break ;
  case base_n:
    {
      (*parts)[basename_n].part = part.part ;
      (*parts)[basename_n].len = part.len ;
      { const char *c = validatepart(parts, basename_n, i) ; if (c) return c ; }
    }
    break ;
  case invalid_n:
    strerr_dief(100, part.part) ;
  default:
    strerr_warn("unhandled ", part.name) ;
  }
  return NULL ;
}

int nextpartis(enum_parts *cp, enum_parts _part, struct_parts (*parts)[], int i, char n) {
  if (parts_END_n == _part) (*parts)[parts_END_n].parts_END.from = *cp ;
  do {
    if ((*parts)[*cp].part &&
	((*parts)[invalid_n].part
	 = validatepart(parts, *cp, i))) {
      *cp = invalid_n ;
	break ;
    }
    ++(*cp) ;
    if (_part < *cp) *cp = _part ;
  } while ( _part > *cp) ;
  if (n || invalid_n == *cp) return 1 ;
  return 0 ;
}


struct parsed *doparse(char *const path, Rules const rules, struct parsed *parsed) {
  char *pcpy = strdup(path) ;
  if (!pcpy) strerr_diefu2sys(111, "strdup failed to copy", path) ;
  {
    enum_addrtype addrtype = 0 ;
  }
  parts[dir_n].part = path ;

#define NEXTPARTIS(_part) { if(nextpartis(&curpart,_part,&parts,i,pn))continue ;break ;}
#define NEXTPART NEXTPARTIS(part->access.next)
#define SETCURPART(_part) part = &parts[curpart = _part]

  char *lastslash = NULL ;
  uint i, unwind = 1 ;
  enum_parts curpart ;
  struct_parts *part ;
  SETCURPART(start_n) ;
  for (i = 0 ; curpart < parts_END_n ; i++, part->part && part->len++) {
    char p, UP, pn, pp ;
    goto rewound ;
  unwind:
    i -= unwind ;
    parts->len = ( unwind >= part->len )
      ? 0 : part->len - unwind ;
    part->len || ( part->part = NULL ) ;
    unwind = 1 ;

  rewound:
    p = path[i] ;		/* current char */
    pn = path[i+1] ;		/* look ahead */
    pp = i ? pcpy[i-1] : NUL ;	/* look behind */

    if (!p) NEXTPARTIS(parts_END_n) ;

      /* path componets reset everything. we might have wasted time
       * processing it, but probably not much longer than strrchr-ing
       * it first
       */
    if ('/' == p) {
      if (pn) {
	char const *cmp ;
	char const *ref ;
	for ( cmp = &path[i+1], ref = parts[log_n].log.name
	       ; *ref && *ref == *cmp
	       ; cmp++, ref++, parts[log_n].len++) ;
	for ( ;'/' == *ref ; ref++) ;
	if (*cmp == *ref) {
	  parts[log_n].part = &path[i+1] ;
	  pn = NUL ;
	} else parts[log_n].len = 0 ;
      }
      if (pn) {
	lastslash && (*lastslash='/') ;
	*(lastslash = &path[i]) = NUL ;
	parts[dir_n].len = i ;
	goto reset ;
      } else {
	if (!i) strerr_die(100, "should not be run from /") ;
	path[i] = pcpy[i] = NUL ;
	goto unwind ;
      }
    }
    if (invalid_n == curpart) {
      if (!p) strerr_die(100, "directory name was invalid: ", parts[invalid_n].part) ;
      else continue ; }/* looking for a / to reset everything	*/

    if (strchr(rules[NetPart], p)) {
      if (netspec_n > curpart) { /* we are now  */
	pcpy[i] = '/' ;
	NEXTPARTIS(netspec_n) ;
      }
      if (!part->part) {
	NEXTPART ;
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
    {
      while (unimplemented & curpart) {
	strerr_warn("skipping ", part->name) ;
	SETCURPART(curpart+1) ;
      }
      part = &parts[curpart] ;
    }
    if (start_n == curpart) {	/* should be base_n */
      if (strchr(rules[EndCmd], p)) {
	pcpy[i] = NUL ;
	NEXTPART ;
      }
      if (!part->part)
	part->part = &path[i] ;
      continue ;
    }

    if (exts_n == curpart) {
      if (strchr(rules[Separator], p)) {
	pcpy[i] = NUL ;
	NEXTPART ;
      }
      if (!part->part) {
	part->part = &pcpy[i] ;
	if (!parts[exts_n].exts.start)
	  parts[exts_n].exts.start = &pcpy[i] ;
      }
    }
    strerr_warn("continuring for ", part->name) ;
    continue ;
 reset: /*damn, a subpath*/
    SETCURPART(start_n) ;
    for (int i = start_n ; part_NULL > i ; ++i) {
      parts[i].part = NULL ;
      parts[i].len = 0 ;
      /* NB. modifications to pcpy haven't been reset, but the path part
       * should be discarded anyway
       */
    }
    pcpy[i] = NUL ;
    curpart = start_n ;
    continue ;
  } /* for */
  if (parts_END_n != curpart)
    strerr_warn("Unexpected exit") ;
  else SETCURPART(part->parts_END.from) ;


  printf("we escaped at position %u: %d\n", i, curpart) ;
  for (enum_parts j=0 ; curpart >= j ; j++)
    printf("parts[%u](%s) = '%s'[%u]\n",
	   j, parts[j].name, parts[j].part, parts[j].len) ;
  {
    struct parsed_field *ret ;
    enum_parts id ;
    struct_parts const *part ; 	/* you cant touch this */
    for (ret = &parsed->fields[0] ; (id = ret->part) ; ++ret) {
      part = &parts[id] ;
      if (part->part) {
	if (ret->value) {
	  strncpy(ret->value, part->part, ret->len) ;
	  if (part->len <= ret->len) ret->len = part->len ;
	  else ret->value[ret->len - 1] = NUL ;
	} else {
	  ret->value = strdup(part->part) ;
	  ret->len = part->len ;
	}
	if (!ret->name) ret->name = part->name ;
      }
    }
  }
  return parsed ;
}

int main(int argc, char const *const *argv) {
  char *path = NULL ;
  {
    const char *av = argc && argv ? strrchr(argv[0], '/') : "/<undef>" ;
    av = av ? av+1 : argv[0] ? argv[0] : "<NULL>" ;
    if (strcmp("pwd-to-subpath", argv[0])) rules = &tosubpath ;
    else if (strcmp("pwd-to-ip", argv[0])) rules = &toip ;
    else rules = &newstyle ;
  }
  if (*newstyle) {
    subgetopt l = SUBGETOPT_ZERO ;
    for ( ; ; )
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
      strerr_warn("realpath gave a relative path: '", path, "'") ;
  }
  static struct parsed parsed = {NULL, {{base_n, "base"},{dir_n,"dirname"},{0}}} ;
  doparse(path, *rules, &parsed) ;
  for (struct parsed_field const *f = &parsed.fields[0] ; f->part ;++f) {
    printf("asked for %s(%u) got %u\n", f->name, f->part, f->len) ;
    puts(f->name) ; putchar('=') ; puts(f->value) ; putchar('\n') ;

  } ;


}


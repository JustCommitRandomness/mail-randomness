# mail-randomness

bits and pieces for my (q)mail setup

## thingies

### execline scripts

Cunningly stored in package/execline-scripts

these are generally focused around making polymorphic service run
scripts, or being said scripts 

#### pwd-to-subpath
- extracts the extension from the current directory name, or its parent 

#### pwd-to-ip
- extracts something loosley resembling an ipv4 address from the current
directory name, or its parent if the current directory is /log.
- Prints it to STDOUT for access via backtick.
- Services are likely to fail if the thing isn't an actual IP addrtess,
but that's for the administrator to get right instead of adding
heavy validation to a lightweight helper.

#### log-runner
- polymorphic run script for recording a service log.
- Uses pwd-to-subpath to file the logs away in a sane place in the log
filesystem

#### qmail-smtp-runner
- polymorphic run script for running qmail-smtp.
- loads defaults from /etc/qmail/ENV
- looks in a /etc/qmail subdir as specified by pwd-to-subpath for any tweaks

#### dnscache-runner
- polymorphic run script for running djb dnscache.
- loads defaults from /etc/dnscache/ENV
- looks in a /etc/dnscache subdir as specified by pwd-to-subpath for any tweaks,
- *then* grabs an IP using pwd-to-ip, overwritting any already configured
- *then*looks in /etc/dnscache/ENV.$IP for even more tweaks

#### svscanboot-probe
- detector for the environment a svscanboot script finds iteself in
- intended to help one write the correct svscan boot wrapper
- e.g. if one has `supervise /service/.svscanboot unknown yes` in
`/etc/ttys` one can drop that in /service/.svscanboot to see what 
init is giving you

#### svscanboot-wrapper

This starts an opinionated svscanboot, on the assmuptionis was
launched by init, based on /etc/ttys. it could be used in other contexts:
YMMV

- defaults PATH to /command only
- records the first parameter as SUPERVISE_TAG. 
	this will be fake-device name unless a parameter is specified 
- works out where it is from $0,  
	on the assmption it is in the service directory it is meant to supervise
	records this as SUPERVISE_DIR
- tries to envdir /etc/svscanboot/ENV for systemwide defaults.
- tries to envdir /etc/svscanboot/ENV.$1 for tag based overrides
- tries to envdir $0.ENV (e.g /service/.svscanboot.ENV) for overrides
- tries to envdir /etc/svscanboot/${SUPERVISE_DIR}/ENV for
	(possibly modified) directory overrides
- tries to envdir /etc/svscanboot/ENV-tag.SUPERVISE_TAG for
	(possibly modified) SUPERVISE_TAG based overrides
- tries to envdir ${SUPERVISE_DIR}/.ENV for
	(possibly modified) SUPERVISE_DIR directory alternate overrides
- tries to create ${LOGDIR}/svscanboot and chown it to LOGUSER
- launches /command/svscanboot -D ${LOGDIR}/svscanboot -l ${LOGUSER} ${SUPERVISE_DIR}

This is ridiculously overcomlicated as most systems will only run one
supervise process, however it should run only once per boot, so
the overhead shouldn;t amtter.

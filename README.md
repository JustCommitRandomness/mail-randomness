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

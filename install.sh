#!/bin/sh 

: ${PACKAGEDIR:=/package}
: ${COMMANDDIR:=/command}

if [ "$0" != "./install.sh" ] || [ ! -d package/execline-scripts ] 
then
  echo must be run as ./install.sh from the top directory
  exit 1
fi
packagemash="${PACKAGEDIR}/"$(basename `pwd`)"/"
if ! packagedir=`realpath -q "$packagemash"`
then
  cat <<-COWARDICE
	${packagemash} doesn't seem to exist, so I'm probably doing something wrong
	Yes, I know most installers make the directory, but better safe than sorry.
COWARDICE
  exit 1
fi
echo installing in $PACKAGEDIR and linking to $COMMANDDIR
for dir in execline-scripts awk sed
do
  if [ -d package/"${dir}" ]
  then
    echo "${dir}"/:
    (
      cd package/"${dir}"

      for tool in *
      do
        case "$tool" in
        *~)
          ;;
        *"#"*)
          ;;
        .*)
          ;;
        *)
          install -o 0 -g 0 -Cb "$tool" "$packagedir/$tool"
          ln -fs "$packagedir/$tool" "$COMMANDDIR/$tool"
          ls -l "$COMMANDDIR/$tool"
          ;;
        esac
      done
    )
  fi
done


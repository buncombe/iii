#!/bin/sh
# ----------------------------------------------------
# Nico Golde <nico@ngolde.de>
# License: do whatever you want with this code
# Purpose: locate new queries for the ii(1) IRC client
# ----------------------------------------------------

IRCPATH=irc
TMPFILE=$IRCPATH/queries.tmp

if [ ! -f $TMPFILE ]; then
	touch $TMPFILE
fi

echo "searching new query data"
for i in `find $IRCPATH -newer $TMPFILE -name 'out'`
do
	# If the file doesn't just contain server stuff.
	grep -v '\-!\-' $i  > /dev/null 2>&1
	if [ $? -ne 1 ]; then
		# Strip server, nickserv, chanserv and channel out files.
		echo $i | egrep -v -i "(nick|chan)serv|#|$IRCPATH\/[a-z0-9\.\-]+\/out$" > /dev/null 2>&1
		if [ $? -ne 1 ]; then
			printf "new data in: %s\n========================================================\n" "$i"
			tail -5 $i
		fi
	fi
done

touch $TMPFILE

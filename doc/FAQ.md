FAQ
===

1. Where is IRC command xy (ignore etc.)?
-----------------------------------------
ii(1) is for advanced users, please use standard tools like awk(1), sed(1) and
grep(1) instead. This can be done easily and is not going to bloat the code.

See also FAQ entry 3.

2. Where is a graphical interface?
----------------------------------
ii(1) follows the UNIX philosophy, which means that it is only file based. It
should be, though, easy to build different interfaces, as they only have to
handle the FIFOs and output files. Feel free to implement!

3. Which commands are supported?
--------------------------------
"/j" (join or msg), "/t" (topic), "/a" (away), "/n" (nick) and "/l" (leave).
The missing ones are obsolete or can be easily used by typing the IRC commands
itself (i.e. /WHO instead of /who).

4. How can I recognize queries?
-------------------------------
ii(1) itself doesn't support this, but the queries.sh script is an example of
how to view the new and changed files in your IRC directory. To get an instant
notice of a new file, other mechanisms like inotify, dnotify or kevent could be
used as well, but I was too lazy to try it out since the script is enough for
me.

5. What other fancy stuff can I do with ii(1)?
----------------------------------------------
It is very easy to write IRC bots using ii(1):

	tail -f \#/out | while read foo; do name=echo $foo | awk '{print $2}' \
	    | sed 's,<\\(.*\\)>,\\1,'; if 0 -eq expr $RANDOM % 10 \
	    then echo "$name: WHAT??" ; fi; done

These commands will just spam a channel, but think about using the client for
Nagios or generating of channel stats. Your imagination should be boundless.

6. How can I automatically reconnect to a server or network?
------------------------------------------------------------
As you might have guessed, ii(1) does not support this feature. The fact is,
however, that it easily can be done by scripting or with the wrapper(1) program
that is shipped with this package of ii(1).
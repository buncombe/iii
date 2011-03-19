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
ii(1) itself doesn't support this, but the query.sh script is an example of
how to view the new and changed files in your IRC directory. To get an instant
notice of a new file, other mechanisms like inotify, dnotify or kevent could be
used as well.

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
ii(1) does not support this feature. The fact is, however, that it easily can
be done by scripting or with the wrapper(1) program that is shipped with this
package of ii(1). See its manpage ([txt][wrapper_man_txt] or
[html][wrapper_man_html]) for more information about the application.

7. How to deal with different character encodings?
--------------------------------------------------
As an example, a channel might recommend its users to use the character
encoding ISO-8859-1 whilst the system that ii(1) is running on is using UTF-8.
This will work very well as long only characters in the range 32-126 are used,
since that range (called the G0 subset) maps to the same coded G0 subset of
UTF-8 (except that the 0x7F character is missing in ISO-8859-1). However,
problem arise when someone in the channel starts using characters in the G1
subset of ISO-8859-1 as this subset in UTF-8 only consists of continuation
bytes, start bytes and some bytes that must not appear in a valid UTF-8
sequence. The solution to this problem is to decode the ISO-8859-1 encoded
input into UTF-8 (1) and on write convert the UTF-8 data to ISO-8859-1 (2).

(1)

	use Encode;
	use strict;
	use warnings;
	
	while (<>) {
		$_ = encode("utf8", decode("iso-8859-1", $_,
		    Encode::FB_DEFAULT));
		print;
	}

Use it in combination with cat(1) by invoking `cat out | perl
./above_perl_script.pl`.

(2)

(This example assumes that vim(1) is used for writing to the channel.) Set the
file encoding to ISO-8859-1 (`:set fileencoding=iso-8859-1`), make sure that
the encoding is utf-8 (`:set encoding=utf-8`) and then simply write to the
channel (`:w>>in`).

[wrapper_man_txt]: http://buncombe.github.com/iii/wrapper.1.txt
[wrapper_man_html]: http://buncombe.github.com/iii/wrapper.1.html

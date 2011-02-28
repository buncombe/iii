ii - IRC it or IRC improved
===========================

Abstract
--------
An extract from the manpage ([txt][ii_man_txt] or [html][ii_man_html]):

> ii is a minimalistic FIFO and file system based IRC client. Upon connection,
> it creates an IRC directory prefix which contains the server directory (its
> name can be overridden by directory ). Beyond the path, inside the server
> directory, there are channel and nickname directories together with a FIFO
> file named "in" and a regular one called "out".  These two latter files do
> also occur inside each channel and nickname directory, which themselves are
> created upon joining a channel or opening of a private conversation.
> 
> The FIFO files "in" are used to communicate with the server, while the
> regular "out" files carry the server messages. If the data sent to an "in"
> file is not an IRC command, in other words not starting with a slash "/", it
> will be interpreted as a regular message (PRIVMSG) which later pops up in the
> "out" file in the same directory level.
> 
> The idea of this file system structure, is to be able to communicate with an
> IRC server using basic command line tools. For example, if the desired action
> is to join a channel, the command `echo /j #channel > in` will do the trick no
> matter which "in" FIFO file it is addressed to, as long as the file is in use
> by the current instance of ii.

For information about wrapper(1), consult its manpage ([txt][wrapper_man_txt]
or [html][wrapper_man_html]).

Installation
------------
Edit config.mk to match your local setup. ii(1) and wrapper(1) are installed
into /usr/local by default.

Afterwards, enter the following command to build and install ii(1) and
wrapper(1) (if necessary, as root):

	$ make clean install

Running ii(1)
-------------
Simply invoke the 'ii' command with the arguments of your choice. For
information about the available arguments, consult the manpage or pass ii(1)
the option "-h".

To make the use of ii(1) a bit more comfortable, use it in combination with the
multitail(1) program and, for example, with vim(1). Run vim in the server
directory and use key mappings like:

	map w1 :.w >> \#ii/in<cr>
	map w2 :.w >> \#wmii/in<cr>

in order to post to channels.

If you use the next editor line for a new posting, you can use ctrl-p for nick
completion if you have written the nick in the past.
Thanks to Matthias Kopfermann for this hint.

You can find an example of how this nested environment could look like on:
[http://nion.modprobe.de/blog/archives/440-Using-the-ii-irc-client.html][nion]

Configuration
-------------
No configuration is needed.

Changelog
---------
Since I missed the chance to add a proper changelog right from the beginning,
please have a look at the commit messages on
[http://hg.suckless.org/ii/shortlog][hg] - they are fairly descriptive on
releases prior to 1.2.

Contact
-------
If you want to contact the developers, just write an e-mail to
ii (at) modprobe (dot) de.

-- Nico Golde, Anselm R. Garbe

[nion]: http://nion.modprobe.de/blog/archives/440-Using-the-ii-irc-client.html
[hg]: http://hg.suckless.org/ii/shortlog
[ii_man_txt]: http://buncombe.github.com/iii/ii.1.txt
[ii_man_html]: http://buncombe.github.com/iii/ii.1.html
[wrapper_man_txt]: http://buncombe.github.com/iii/wrapper.1.txt
[wrapper_man_html]: http://buncombe.github.com/iii/wrapper.1.html

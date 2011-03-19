ii - IRC it or IRC improved
===========================

Introduction
------------
An extract from the manpage ([txt][ii_man_txt] or [html][ii_man_html]) that
explains what ii(1) is, how it works and why it was created:

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

This version of ii(1) is bundled into a package called iii, which, except a
modified ii(1), includes an application called wrapper(1) that can be used to
wrap ii(1) and to offer reconnection abilities. For more detailed information
about wrapper(1) and what the purpose of it is, consult its manpage
([txt][wrapper_man_txt] or [html][wrapper_man_html]).

Below are some changes to ii(1) listed that are not part of the [upstream
version][ii], but that this bundle provides:

* SASL PLAIN support.
* SSLv2/SSLv3/TLSv1 encryption.
* More detailed documentation.
* A [mdoc][mdoc] formatted manpage.
* Improved command management code.
* Better handling of the option flags.

Installation
------------
Edit config.mk to match your local setup. ii(1) and wrapper(1) are installed
into /usr/local by default. (Notice that it is also possible to adjust the
variables in config.mk by using environment variables at build time.)

Afterwards, enter the following command to build and install ii(1) and
wrapper(1). (If necessary, run it as root):

	$ make clean install

Running ii(1)
-------------
Simply invoke the 'ii' command with the arguments of your choice. For
information about the available flags, consult the manpage or pass ii(1) the
option "-h".

To make the use of ii(1) a bit more comfortable, use it in combination with the
multitail(1) program and, for example, with vim(1). Run vim(1) in the server
directory and use key-mappings like:

	map w1 :.w >> \#ii/in<cr>
	map w2 :.w >> \#wmii/in<cr>

in order to post to channels.

In vim(1) it is also possible to use ctrl-p (in insert mode) for nick
completion if the nick has been written in the past. Thanks to Matthias
Kopfermann for this hint.

For an example of how this nested environment could look like on, check this
blog post:
[http://nion.modprobe.de/blog/archives/440-Using-the-ii-irc-client.html][nion]

Configuration
-------------
No configuration is needed except optionally at compile time, whereas the user
is able to modify some macros and variables. See the config.mk file for more
details on this.

Changelog
---------
See the [CHANGES][changes] file that comes with this package of ii(1).

Notice that there is no explicit changelog available for ii(1) prior to version
1.2, but, in order to get an overview of what has changed since the start of
the development until version 1.2, have a look at the commit messages on
[http://hg.suckless.org/ii/shortlog][hg].

It is the same case with wrapper(1) and the iii package version of ii(1), as
they do not have their changes documented in a specific file. However, this
sort of information is available in the [commit messages][gh_commits].

Contact
-------
If you want to contact the developers, just write an e-mail to ii (at) modprobe
(dot) de. Alternatively, when the subject is specifically related to wrapper(1)
or changes that only concern the iii version of the program, use the
issue tracker at [https://github.com/buncombe/iii/issues][issues] (requires
registration) or send a message over IRC to buncombe on the [OFTC
network][oftc].

The upstream ii(1) project has also a dedicated IRC channel, #ii, on the OFTC
network.

See also
--------
There are some frequently asked questions with answers available in the file
[doc/FAQ.md][faq]. Have a look at it for more information about ii(1).

[nion]: http://nion.modprobe.de/blog/archives/440-Using-the-ii-irc-client.html
[hg]: http://hg.suckless.org/ii/shortlog
[ii_man_txt]: http://buncombe.github.com/iii/ii.1.txt
[ii_man_html]: http://buncombe.github.com/iii/ii.1.html
[wrapper_man_txt]: http://buncombe.github.com/iii/wrapper.1.txt
[wrapper_man_html]: http://buncombe.github.com/iii/wrapper.1.html
[issues]: https://github.com/buncombe/iii/issues
[ii]: http://tools.suckless.org/ii/
[mdoc]: https://secure.wikimedia.org/wikipedia/en/wiki/Mandoc
[oftc]: http://www.oftc.net/oftc/
[gh_commits]: https://github.com/buncombe/iii/commits/
[faq]: https://github.com/buncombe/iii/blob/master/doc/FAQ.md
[changes]: https://github.com/buncombe/iii/blob/master/CHANGES

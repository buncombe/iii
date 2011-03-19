JOINUSER
========

Description
-----------
By default, to PRIVMSG a user you need to `echo /j user message > in`, this
patch makes the "message" optional. It also displays "–!– yournick has joined
user" prior to any messages both when you `echo /j user > in` and when a user
messages you, and incorporates the autojoin patch, so you do not need to `echo
/j user > in` first in order to talk to someone who has already messaged you.

Authors
-------
* Robert Lowry (bobertlo) <robertwlowry@gmail.com>
* Evan Gates (emg) <evan.gates@gmail.com>

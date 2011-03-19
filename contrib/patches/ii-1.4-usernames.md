USERNAMES
=========

Description
-----------
Changes the handling of QUIT and NICK messages from other users so that the
output is written to the relevant channels rather than to the server "out"
file. This patch is rather bulky but it gets the job done.

Notes
-----
I have tested this patch against the other patches and they all apply, but for
some of them you must apply this patch first or there will be conflicts.

Author
------
* Robert Lowry (bobertlo) <robertwlowry@gmail.com>

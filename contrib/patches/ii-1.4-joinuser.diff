diff -r d93eaacde742 ii.c
--- a/ii.c	Fri Jun 25 10:55:05 2010 +0200
+++ b/ii.c	Tue Jul 13 09:29:23 2010 -0700
@@ -105,6 +105,7 @@
 	return open(infile, O_RDONLY | O_NONBLOCK, 0);
 }
 
+static void print_out(char *channel, char *buf); // needs to be declared
 static void add_channel(char *name) {
 	Channel *c;
 	int fd;
@@ -130,6 +131,12 @@
 	}
 	c->fd = fd;
 	c->name = strdup(name);
+
+	if(name[0] && !((name[0]=='#')||(name[0]=='&')||(name[0]=='+')||(name[0]=='!'))) {
+		char msg[128];
+		snprintf(msg, sizeof(msg), "-!- %s has joined %s", nick, name);
+		print_out(name, msg);
+	}
 }
 
 static void rm_channel(Channel *c) {
@@ -210,6 +217,7 @@
 	if(channel) snprintf(server, sizeof(server), "-!- %s", channel);
 	if(strstr(buf, server)) channel="";
 	create_filepath(outfile, sizeof(outfile), channel, "out");
+	if(channel && channel[0]) add_channel(channel);
 	if(!(out = fopen(outfile, "a"))) return;
 
 	strftime(buft, sizeof(buft), "%F %R", localtime(&t));
@@ -237,17 +245,14 @@
 		case 'j':
 			p = strchr(&buf[3], ' ');
 			if(p) *p = 0;
+			add_channel(&buf[3]);
 			if((buf[3]=='#')||(buf[3]=='&')||(buf[3]=='+')||(buf[3]=='!')){
 				if(p) snprintf(message, PIPE_BUF, "JOIN %s %s\r\n", &buf[3], p + 1); /* password protected channel */
 				else snprintf(message, PIPE_BUF, "JOIN %s\r\n", &buf[3]);
-				add_channel(&buf[3]);
 			}
-			else {
-				if(p){
-					add_channel(&buf[3]);
-					proc_channels_privmsg(&buf[3], p + 1);
-					return;
-				}
+			else if(p) {
+				proc_channels_privmsg(&buf[3], p + 1);
+				return;
 			}
 			break;
 		case 't':
@@ -357,6 +362,8 @@
 		argv[TOK_CHAN] = argv[TOK_TEXT];
 		snprintf(message, PIPE_BUF, "-!- %s(%s) has joined %s", argv[TOK_NICKSRV], argv[TOK_USER], argv[TOK_TEXT]);
 	} else if(!strncmp("PART", argv[TOK_CMD], 5)) {
+		if (!strcmp(nick, argv[TOK_NICKSRV]))
+			return;
 		snprintf(message, PIPE_BUF, "-!- %s(%s) has left %s", argv[TOK_NICKSRV], argv[TOK_USER], argv[TOK_CHAN]);
 	} else if(!strncmp("MODE", argv[TOK_CMD], 5))
 		snprintf(message, PIPE_BUF, "-!- %s changed mode/%s -> %s %s", argv[TOK_NICKSRV], argv[TOK_CMD + 1] ? argv[TOK_CMD + 1] : "" , argv[TOK_CMD + 2]? argv[TOK_CMD + 2] : "", argv[TOK_CMD + 3] ? argv[TOK_CMD + 3] : "");

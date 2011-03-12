/* (C)opyright MMV-MMVI Anselm R. Garbe <garbeam at gmail dot com>
 * (C)opyright MMV-MMXI Nico Golde <nico at ngolde dot de>
 * See LICENSE file for license details. */
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#ifdef USESSL
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#ifndef PIPE_BUF /* FreeBSD don't know PIPE_BUF */
#define PIPE_BUF 4096
#endif
#define PING_TIMEOUT 300
#define SERVER_PORT 6667
#define IS_CHANNEL(s) (((s)[0]=='#')||((s)[0]=='&')||((s)[0]=='+')||((s)[0]=='!'))
#ifdef USESSL
#define SSL_SERVER_PORT 6697
#define WRITE(conn, msg) (use_ssl ? SSL_write(irc->sslHandle, msg, strlen(msg)) : write(conn->irc, msg, strlen(msg)))
#define READ(fd, buf, s) (from_srv && use_ssl ? SSL_read(irc->sslHandle, buf, s) : read(fd, buf, s))
#else
#define WRITE(irc, msg) (write(irc, msg, strlen(msg)))
#define READ(fd, buf, s) (read(fd, buf, s))
#endif
enum { TOK_NICKSRV = 0, TOK_USER, TOK_CMD, TOK_CHAN, TOK_ARG, TOK_TEXT, TOK_LAST };

typedef struct Channel Channel;
struct Channel {
	int fd;
	char *name;
	Channel *next;
};
#ifdef USESSL
typedef struct {
	int irc;
	SSL *sslHandle;
	SSL_CTX *sslContext;
} conn;
#endif

#ifdef USESSL
static size_t use_ssl = 0;
static conn *irc;
static unsigned char fp[EVP_MAX_MD_SIZE];
static int fp_len = 0;
#else
static int irc;
#endif
static time_t last_response;
static Channel *channels = NULL;
static char *host = "irc.freenode.net";
static char nick[32] = "anonymous";	/* might change while running */
static char path[_POSIX_PATH_MAX];
static char message[PIPE_BUF]; /* message buf used for communication */

static void usage() {
	fprintf(stderr, "%s",
			"ii - irc it - " VERSION "\n"
			"(C)opyright MMV-MMVI Anselm R. Garbe\n"
			"(C)opyright MMV-MMXI Nico Golde\n"
			"usage: ii [-i <irc dir>] [-s <host>] [-p <port>]\n"
			"          [-n <nick>] [-k <password>] [-f <fullname>]\n"
#ifdef USESSL
			"          [-e] [-d <directory>]\n");
#else
			"          [-d <directory>]\n");
#endif
	exit(EXIT_FAILURE);
}

static char *striplower(char *s) {
	char *p = NULL;
	for(p = s; p && *p; p++) {
		if(*p == '/') *p = '_';
		*p = tolower(*p);
	}
	return s;
}

/* creates directories top-down, if necessary */
static void create_dirtree(const char *dir) {
	char tmp[256];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp),"%s",dir);
	len = strlen(tmp);
	if(tmp[len - 1] == '/')
		tmp[len - 1] = 0;
	for(p = tmp + 1; *p; p++)
		if(*p == '/') {
			*p = 0;
			mkdir(tmp, S_IRWXU);
			*p = '/';
		}
	mkdir(tmp, S_IRWXU);
}

static int get_filepath(char *filepath, size_t len, char *channel, char *file) {
	if(channel) {
		if(!snprintf(filepath, len, "%s/%s", path, channel))
			return 0;
		create_dirtree(filepath);
		return snprintf(filepath, len, "%s/%s/%s", path, channel, file);
	}
	return snprintf(filepath, len, "%s/%s", path, file);
}

static void create_filepath(char *filepath, size_t len, char *channel, char *suffix) {
	if(!get_filepath(filepath, len, striplower(channel), suffix)) {
		fprintf(stderr, "ii: path to irc directory too long\n");
		exit(EXIT_FAILURE);
	}
}

static int open_channel(char *name) {
	static char infile[256];
	create_filepath(infile, sizeof(infile), name, "in");
	if(access(infile, F_OK) == -1)
		mkfifo(infile, S_IRWXU);
	return open(infile, O_RDONLY | O_NONBLOCK, 0);
}

static void add_channel(char *cname) {
	Channel *c;
	int fd;
	char *name = striplower(cname);

	for(c = channels; c; c = c->next)
		if(!strcmp(name, c->name))
			return; /* already handled */

	fd = open_channel(name);
	if(fd == -1) {
		fprintf(stderr, "ii: exiting, cannot create in channel: %s\n", name);
		exit(EXIT_FAILURE);
	}
	c = calloc(1, sizeof(Channel));
	if(!c) {
		perror("ii: cannot allocate memory");
		exit(EXIT_FAILURE);
	}
	if(!channels) channels = c;
	else {
		c->next = channels;
		channels = c;
	}
	c->fd = fd;
	c->name = strdup(name);
}

static void rm_channel(Channel *c) {
	Channel *p;
	if(channels == c) channels = channels->next;
	else {
		for(p = channels; p && p->next != c; p = p->next);
		if(p->next == c)
			p->next = c->next;
	}
	free(c->name);
	free(c);
}

#ifdef USESSL
static char *base64_encode(const char *input, int length) {
	BIO *b64, *bmem;
        BUF_MEM *bptr;
	char *ret = NULL;

	if (!input)
		return ret;

	if (length < 1)
		length = strlen(input);

	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_new(BIO_f_base64());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, input, length);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);
	ret = strndup(bptr->data, bptr->length);
	BIO_free_all(b64);

	return ret;
}


void *mcat(void *s1, size_t n1, void *s2, size_t n2) {
	void *target = (char*)s1 + n1;

	memcpy(target,s2, n2);
	return s1;
}
#endif

static void login(char *key, char *fullname, size_t use_sasl) {
#ifdef USESSL
	char *auth, *ret;
	size_t size;

	if(key && use_sasl) {
		size = strlen(key) + strlen(nick) * 2 + 2;
		auth = calloc(size, 1);
		memcpy(auth, nick, strlen(nick));
		mcat(auth, strlen(nick) + 1, nick, strlen(nick));
		mcat(auth, strlen(nick) * 2 + 2, key, strlen(key)); /* auth = nick\0nick\0key (no trailing NUL) */
		ret = base64_encode(auth, size);
		snprintf(message, PIPE_BUF, "CAP REQ :sasl\r\nNICK %s\r\nUSER %s localhost * :%s\r\nAUTHENTICATE PLAIN\r\nAUTHENTICATE %s\r\nCAP END\r\n", nick, nick, fullname ? fullname : nick, ret);
		free(auth);
		free(ret);
#endif
	} else if(key && !use_sasl) snprintf(message, PIPE_BUF,
				"PASS %s\r\nNICK %s\r\nUSER %s localhost * :%s\r\n", key,
				nick, nick, fullname ? fullname : nick);
	else snprintf(message, PIPE_BUF, "NICK %s\r\nUSER %s localhost * :%s\r\n",
				nick, nick, fullname ? fullname : nick);
	WRITE(irc, message);	/* login */
}

#ifdef USESSL
static conn *ssl_connect(int fd) {
	conn *c = NULL;

	c = malloc(sizeof(conn));
	if(!c) {
		perror("ii: cannot allocate memory");
		exit(EXIT_FAILURE);
	}
	c->irc = fd;

	if (use_ssl) {
		c->sslHandle = NULL;
		c->sslContext = NULL;
		SSL_load_error_strings();
		SSL_library_init();
		c->sslContext = SSL_CTX_new(SSLv23_client_method());
		if(c->sslContext == NULL)
			ERR_print_errors_fp(stderr);
		c->sslHandle = SSL_new(c->sslContext);
		if(!SSL_set_fd(c->sslHandle, c->irc) || SSL_connect(c->sslHandle) != 1)
			ERR_print_errors_fp(stderr);
		if(!X509_digest(SSL_get_peer_certificate(c->sslHandle), EVP_md5(), fp, &fp_len))
			ERR_print_errors_fp(stderr);
	}

	return c;
}

static conn *tcpopen(unsigned short port) {
#else
static int tcpopen(unsigned short port) {
#endif
	int fd;
	struct sockaddr_in sin;
	struct hostent *hp = gethostbyname(host);

	memset(&sin, 0, sizeof(struct sockaddr_in));
	if(!hp) {
		perror("ii: cannot retrieve host information");
		exit(EXIT_FAILURE);
	}
	sin.sin_family = AF_INET;
	memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
	sin.sin_port = htons(port);
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("ii: cannot create socket");
		exit(EXIT_FAILURE);
	}
	if(connect(fd, (const struct sockaddr *) &sin, sizeof(sin)) < 0) {
		perror("ii: cannot connect to host");
		exit(EXIT_FAILURE);
	}

#ifdef USESSL
	return ssl_connect(fd);
#else
	return fd;
#endif
}

static size_t tokenize(char **result, size_t reslen, char *str, char delim) {
	char *p = NULL, *n = NULL;
	size_t i;

	if(!str)
		return 0;
	for(n = str; *n == ' '; n++);
	p = n;
	for(i = 0; *n != 0;) {
		if(i == reslen)
			return 0;
		if(i > TOK_CHAN - TOK_CMD && strtol(result[0], NULL, 10) > 0) delim=':'; /* workaround non-RFC compliant messages */
		if(*n == delim) {
			*n = 0;
			result[i++] = p;
			p = ++n;
		} else
			n++;
	}
	if(i<reslen && p < n && strlen(p))
		result[i++] = p;
	return i;				/* number of tokens */
}

static void print_out(char *channel, char *buf) {
	static char outfile[256], server[256], buft[18];
	FILE *out = NULL;
	time_t t = time(0);

	if(channel) snprintf(server, sizeof(server), "-!- %s", channel);
	if(strstr(buf, server)) channel="";
	create_filepath(outfile, sizeof(outfile), channel, "out");
	if(!(out = fopen(outfile, "a"))) return;
	if(channel && channel[0] != '\0') add_channel(channel);

	strftime(buft, sizeof(buft), "%F %R", localtime(&t));
	fprintf(out, "%s %s\n", buft, buf);
	fclose(out);
}

static void proc_channels_privmsg(char *channel, char *buf) {
	if(channel && channel[0] != '\0') {
		snprintf(message, PIPE_BUF, "<%s> %s", nick, buf);
		print_out(channel, message);
		snprintf(message, PIPE_BUF, "PRIVMSG %s :%s\r\n", channel, buf);
		WRITE(irc, message);
	}
}

static void proc_channels_input(Channel *c, char *buf) {
	char *p = NULL;

	if(!buf || *buf == '\0')
		return;

	if(*buf != '/') {
		proc_channels_privmsg(c->name, buf);
		return;
	}
	if(buf[1] == '\0')
		return;
	message[0] = '\0';
	if(buf[2] == ' ' || buf[2] == '\0') switch (buf[1]) {
		case 'j':
			if(buf[3] == ' ' || buf[3] == '\0') return;
			p = strchr(&buf[3], ' ');
			if(p) *p = 0;
			if(IS_CHANNEL(&buf[3])){
				if(p && strlen(p + 1)) snprintf(message, PIPE_BUF, "JOIN %s %s\r\n", &buf[3], p + 1); /* password protected channel */
				else snprintf(message, PIPE_BUF, "JOIN %s\r\n", &buf[3]);
				add_channel(&buf[3]);
			}
			else if(p && strlen(p + 1)) {
				add_channel(&buf[3]);
				proc_channels_privmsg(&buf[3], p + 1);
				return;
			}
			break;
		case 't':
			if(IS_CHANNEL(&buf[3])){
				p = strchr(&buf[3], ' ');
				if(p) *p = 0;
				if(p && strlen(p + 1)) snprintf(message, PIPE_BUF, "TOPIC %s :%s\r\n", &buf[3], p + 1);
				else snprintf(message, PIPE_BUF, "TOPIC %s\r\n", &buf[3]);
			}
			else {
				if(c->name[0] == '\0') return;
				if(strlen(buf)>3) snprintf(message, PIPE_BUF, "TOPIC %s :%s\r\n", c->name, &buf[3]);
				else snprintf(message, PIPE_BUF, "TOPIC %s\r\n", c->name);
			}
			break;
		case 'a':
			if(strlen(buf)>3){
				snprintf(message, PIPE_BUF, "-!- %s is away \"%s\"", nick, &buf[3]);
				print_out(c->name, message);
				snprintf(message, PIPE_BUF, "AWAY :%s\r\n", &buf[3]);
			} else snprintf(message, PIPE_BUF, "AWAY\r\n");
			break;
		case 'n':
			if(strlen(buf)>3){
				snprintf(nick, sizeof(nick),"%s", &buf[3]);
				snprintf(message, PIPE_BUF, "NICK %s\r\n", &buf[3]);
			}
			break;
		case 'l':
			if(c->name[0] == '\0') return;
			if(strlen(buf)>3) snprintf(message, PIPE_BUF, "PART %s :%s\r\n", c->name, &buf[3]);
			else snprintf(message, PIPE_BUF, "PART %s\r\n", c->name);
			if(IS_CHANNEL(c->name)) WRITE(irc, message);
			close(c->fd);
			rm_channel(c);
			return;
			break;
		default:
			snprintf(message, PIPE_BUF, "%s\r\n", &buf[1]);
			break;
	} else
		snprintf(message, PIPE_BUF, "%s\r\n", &buf[1]);

	if (message[0] != '\0')
		WRITE(irc, message);
}

static void proc_server_cmd(char *buf) {
	char *argv[TOK_LAST], *cmd = NULL, *p = NULL;
	int i;

	if(!buf || *buf=='\0')
		return;

	for(i = 0; i < TOK_LAST; i++)
		argv[i] = NULL;
	/* <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
	   <prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
	   <command>  ::= <letter> { <letter> } | <number> <number> <number>
	   <SPACE>    ::= ' ' { ' ' }
	   <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
	   <middle>   ::= <Any *non-empty* sequence of octets not including SPACE
	   or NUL or CR or LF, the first of which may not be ':'>
	   <trailing> ::= <Any, possibly *empty*, sequence of octets not including NUL or CR or LF>
	   <crlf>     ::= CR LF */

	if(buf[0] == ':') {		/* check prefix */
		if (!(p = strchr(buf, ' '))) return;
		*p = 0;
		for(++p; *p == ' '; p++);
		cmd = p;
		argv[TOK_NICKSRV] = &buf[1];
		if((p = strchr(buf, '!'))) {
			*p = 0;
			argv[TOK_USER] = ++p;
		}
	} else
		cmd = buf;

	/* remove CRLFs */
	for(p = cmd; p && *p != 0; p++)
		if(*p == '\r' || *p == '\n')
			*p = 0;

	if((p = strchr(cmd, ':'))) {
		*p = 0;
		argv[TOK_TEXT] = ++p;
	}

	tokenize(&argv[TOK_CMD], TOK_LAST - TOK_CMD, cmd, ' ');

	if(!argv[TOK_CMD] || !strncmp("PONG", argv[TOK_CMD], 5)) {
		return;
	} else if(!strncmp("PING", argv[TOK_CMD], 5)) {
		snprintf(message, PIPE_BUF, "PONG %s\r\n", argv[TOK_TEXT]);
		WRITE(irc, message);
		return;
	} else if(!argv[TOK_NICKSRV] || !argv[TOK_USER]) {	/* server command */
		snprintf(message, PIPE_BUF, "%s%s", argv[TOK_ARG] ? argv[TOK_ARG] : "", argv[TOK_TEXT] ? argv[TOK_TEXT] : "");
		print_out("", message);
		return;
	} else if(!strncmp("ERROR", argv[TOK_CMD], 6))
		snprintf(message, PIPE_BUF, "-!- error %s", argv[TOK_TEXT] ? argv[TOK_TEXT] : "unknown");
	else if(!strncmp("JOIN", argv[TOK_CMD], 5)) {
		if(argv[TOK_TEXT] != NULL){
			p = strchr(argv[TOK_TEXT], ' ');
			if(p)
				*p = 0;
		}
		argv[TOK_CHAN] = argv[TOK_TEXT];
		snprintf(message, PIPE_BUF, "-!- %s(%s) has joined %s", argv[TOK_NICKSRV], argv[TOK_USER], argv[TOK_TEXT]);
	} else if(!strncmp("PART", argv[TOK_CMD], 5)) {
		snprintf(message, PIPE_BUF, "-!- %s(%s) has left %s", argv[TOK_NICKSRV], argv[TOK_USER], argv[TOK_CHAN]);
	} else if(!strncmp("MODE", argv[TOK_CMD], 5))
		snprintf(message, PIPE_BUF, "-!- %s changed mode/%s -> %s %s", argv[TOK_NICKSRV], argv[TOK_CMD + 1] ? argv[TOK_CMD + 1] : "" , argv[TOK_CMD + 2]? argv[TOK_CMD + 2] : "", argv[TOK_CMD + 3] ? argv[TOK_CMD + 3] : "");
	else if(!strncmp("QUIT", argv[TOK_CMD], 5))
		snprintf(message, PIPE_BUF, "-!- %s(%s) has quit \"%s\"", argv[TOK_NICKSRV], argv[TOK_USER], argv[TOK_TEXT] ? argv[TOK_TEXT] : "");
	else if(!strncmp("NICK", argv[TOK_CMD], 5))
		snprintf(message, PIPE_BUF, "-!- %s changed nick to %s", argv[TOK_NICKSRV], argv[TOK_TEXT]);
	else if(!strncmp("TOPIC", argv[TOK_CMD], 6))
		snprintf(message, PIPE_BUF, "-!- %s changed topic to \"%s\"", argv[TOK_NICKSRV], argv[TOK_TEXT] ? argv[TOK_TEXT] : "");
	else if(!strncmp("KICK", argv[TOK_CMD], 5))
		snprintf(message, PIPE_BUF, "-!- %s kicked %s (\"%s\")", argv[TOK_NICKSRV], argv[TOK_ARG], argv[TOK_TEXT] ? argv[TOK_TEXT] : "");
	else if(!strncmp("NOTICE", argv[TOK_CMD], 7))
		snprintf(message, PIPE_BUF, "-!- \"%s\")", argv[TOK_TEXT] ? argv[TOK_TEXT] : "");
	else if(!strncmp("PRIVMSG", argv[TOK_CMD], 8))
		snprintf(message, PIPE_BUF, "<%s> %s", argv[TOK_NICKSRV], argv[TOK_TEXT] ? argv[TOK_TEXT] : "");
	if(!argv[TOK_CHAN] || !strncmp(argv[TOK_CHAN], nick, strlen(nick)))
		print_out(argv[TOK_NICKSRV], message);
	else
		print_out(argv[TOK_CHAN], message);
}

#ifdef USESSL
static int read_line(int fd, size_t res_len, char *buf, size_t from_srv) {
#else
static int read_line(int fd, size_t res_len, char *buf) {
#endif
	size_t i = 0;
	char c = 0;
	do {
		if(READ(fd, &c, sizeof(char)) != sizeof(char))
			return -1;
		buf[i++] = c;
	}
	while(c != '\n' && i < res_len);
	buf[i - 1] = 0;			/* eliminates '\n' */
#ifdef USESSL
	if(from_srv && use_ssl && SSL_pending(irc->sslHandle))
		return 1;
#endif
	return 0;
}

static void handle_channels_input(Channel *c) {
	static char buf[PIPE_BUF];
#ifdef USESSL
	if(read_line(c->fd, PIPE_BUF, buf, 0) == -1) {
#else
	if(read_line(c->fd, PIPE_BUF, buf) == -1) {
#endif
		close(c->fd);
		int fd = open_channel(c->name);
		if(fd != -1)
			c->fd = fd;
		else
			rm_channel(c);
		return;
	}
	proc_channels_input(c, buf);
}

static void handle_server_output() {
	static char buf[PIPE_BUF];
#ifdef USESSL
	int r;

	do {
		r = read_line(irc->irc, PIPE_BUF, buf, 1);
		if (r == -1) {
			perror("ii: remote host closed connection");
			exit(EXIT_FAILURE);
		} else
			proc_server_cmd(buf);
	} while(r != 0);
#else
	if(read_line(irc, PIPE_BUF, buf) == -1) {
		perror("ii: remote host closed connection");
		exit(EXIT_FAILURE);
	}
	proc_server_cmd(buf);
#endif
}

static void run() {
	Channel *c;
	int r, maxfd;
	fd_set rd;
	struct timeval tv;
	char ping_msg[17] = "PING localhost\r\n";

	for(;;) {
		FD_ZERO(&rd);
#ifdef USESSL
		maxfd = irc->irc;
		FD_SET(irc->irc, &rd);
#else
		maxfd = irc;
		FD_SET(irc, &rd);
#endif
		for(c = channels; c; c = c->next) {
			if(maxfd < c->fd)
				maxfd = c->fd;
			FD_SET(c->fd, &rd);
		}

		tv.tv_sec = 120;
		tv.tv_usec = 0;
		r = select(maxfd + 1, &rd, 0, 0, &tv);
		if(r < 0) {
			if(errno == EINTR)
				continue;
			perror("ii: error on select()");
			exit(EXIT_FAILURE);
		} else if(r == 0) {
			if(time(NULL) - last_response >= PING_TIMEOUT) {
				print_out("", "-!- ii shutting down: ping timeout");
				exit(EXIT_FAILURE);
			}
			WRITE(irc, ping_msg);
			continue;
		}
#ifdef USESSL
		if(FD_ISSET(irc->irc, &rd)) {
#else
		if(FD_ISSET(irc, &rd)) {
#endif
			handle_server_output();
			last_response = time(NULL);
		}
		for(c = channels; c; c = c->next)
			if(FD_ISSET(c->fd, &rd))
				handle_channels_input(c);
	}
}

int main(int argc, char *argv[]) {
	size_t use_sasl = 0;
	int i;
	unsigned short port = SERVER_PORT;
	char *key = NULL, *fullname = NULL, *dir = NULL;
#ifdef USESSL
	char prefix[_POSIX_PATH_MAX] = "irc", *pmsg = message + 17;

	while((i = getopt(argc, argv, "aei:s:p:n:k:f:d:")) != -1) {
#else
	char prefix[_POSIX_PATH_MAX] = "irc";

	while((i = getopt(argc, argv, "i:s:p:n:k:f:d:")) != -1) {
#endif
		switch (i) {
#ifdef USESSL
			case 'a': use_sasl = 1; break;
			case 'e': use_ssl = 1; break;
#endif
			case 'i': snprintf(prefix,sizeof(prefix),"%s", optarg); break;
			case 's': host = optarg; break;
			case 'p': port = strtol(optarg, NULL, 10); break;
			case 'n': snprintf(nick,sizeof(nick),"%s", optarg); break;
			case 'k': key = optarg; break;
			case 'f': fullname = optarg; break;
			case 'd': dir = optarg; break;
			default: usage(); break;
		}
	}
#ifdef USESSL
	if(use_ssl) port = port == SERVER_PORT ? SSL_SERVER_PORT : port;
#endif
	irc = tcpopen(port);
	if(!snprintf(path, sizeof(path), "%s/%s", prefix, dir ? dir : host)) {
		fprintf(stderr, "ii: path to irc directory too long\n");
		exit(EXIT_FAILURE);
	}
	create_dirtree(path);

	add_channel(""); /* master channel */
	login(key, fullname, use_sasl);
#ifdef USESSL
	if (use_ssl && fp_len) {
		snprintf(message, PIPE_BUF, "MD5 Fingerprint: ");
		for(i = 0; strlen(message) + (i > 0 ? 4 : 3) < PIPE_BUF && i < fp_len; i++) {
			snprintf(pmsg, PIPE_BUF, i > 0 ? ":%02X" : "%02X", fp[i]);
			if(i > 0) pmsg += 3;
			else pmsg += 2;
		}
		print_out("", message);
	}
#endif
	run();
	return 0;
}

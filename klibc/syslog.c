/*
 * syslog.c
 *
 * Issue syslog messages via the kernel printk queue.
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>

/* Maximum size for a kernel message */
#define BUFLEN 1024

/* Logging node */
#define LOGDEV "/dev/kmsg"

/* Max length of ID string */
#define MAXID 31

int __syslog_fd = -1;
static char id[MAXID+1];

void openlog(const char *ident, int option, int facility)
{
  (void)option; (void)facility;	/* Unused */
  
  if ( __syslog_fd == -1 )
    __syslog_fd = open(LOGDEV, O_WRONLY);
  
  strncpy(id, ident?ident:"", MAXID);
  id[MAXID] = '\0';		/* Make sure it's null-terminated */
}

void syslog(int prio, const char *format, ...)
{
  va_list ap;
  char buf[BUFLEN];
  int rv, len;
  int fd;

  if ( __syslog_fd == -1 )
    openlog(NULL, 0, 0);

  fd = __syslog_fd;
  if ( fd == -1 )
    fd = 2;			/* Failed to open log, write to stderr */

  buf[0] = '<';
  buf[1] = LOG_PRI(prio)+'0';
  buf[2] = '>';
  len = 3;

  if ( *id )
    len += sprintf(buf+3, "%s: ", id);
  
  va_start(ap, format);
  rv = vsnprintf(buf+len, BUFLEN-len, format, ap);
  va_end(ap);

  len += rv;
  if ( len > BUFLEN-1 ) len = BUFLEN-1;
  buf[len] = '\n';

  write(fd, buf, len+1);
}
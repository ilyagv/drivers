/*
 * llcsocket.c
 *   This one is heavily based on socket.c from the original CUPS
 *   distribution.
 *
 *   To compile use gcc -o llcsocket -lcups llcsocket.c
 *
 *   Syntax for the URLs:
 *
 *   llcsocket://<printername>%<interface>
 *
 *   The printer name must be present in /etc/ethers to be resolved.
 *
 * The Common UNIX Printing System(tm), ("CUPS(tm)"), is provided
 * under the GNU General Public License ("GPL") and GNU Library
 * General Public License ("LGPL"), Version 2, with exceptions for
 * Apple operating systems and the OpenSSL toolkit.
 *
 * See LICENSE.txt from the CUPS distribution for more information.
 *
 *   LLCSocket backend for the Common UNIX Printing System (CUPS).
 *
 *   Copyright 1997-2005 by Easy Software Products, all rights reserved.
 *   Copyright 2005 by Jochen Friedrich <jochen@scram.de>
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Easy Software Products and are protected by Federal
 *   copyright law.  Distribution and use rights are outlined in the file
 *   "LICENSE" which should have been included with this file.  If this
 *   file is missing or damaged please contact Easy Software Products
 *   at:
 *
 *       Attn: CUPS Licensing Information
 *       Easy Software Products
 *       44141 Airport View Drive, Suite 204
 *       Hollywood, Maryland 20636 USA
 *
 *       Voice: (301) 373-9600
 *       EMail: cups-info@cups.org
 *         WWW: http://www.cups.org
 *
 *   This file is subject to the Apple OS-Developed Software exception.
 *
 * Contents:
 *
 *   main() - Send a file to the printer or server.
 */

/*
 * Include necessary headers.
 */

#include <cups/cups.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <netinet/ether.h>

#define IFHWADDRLEN     6
#include <linux/llc.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/if_packet.h>

#define SIOCGIFHWADDR      0x8927
#define AF_LLC              26

/*
 * Local functions...
 */

void	print_backchannel(const unsigned char *buffer, int nbytes);
static int get_hwaddr (char *name, struct ether_addr *addr, int *type);


/*
 * 'main()' - Send a file to the printer or server.
 *
 * Usage:
 *
 *    printer-uri job-id user title copies options [file]
 */

static int
get_hwaddr (char *name, struct ether_addr *addr, int *type)
{
  struct ifreq ifr;
  int fd = socket (AF_INET, SOCK_DGRAM, 0);

  if (fd < 0)
  {
    perror ("ERROR: Unable to create AF_INET socket");
    return (1);
  }
  bcopy (name, &ifr.ifr_name, sizeof (ifr.ifr_name));

  /* find my own hardware address */
  if (ioctl (fd, SIOCGIFHWADDR, &ifr) < 0)
  {
    close (fd);
    fprintf(stderr, "ERROR: Unable to find interface \'%s\'\n",
            name);
    return (1);
  }
  close (fd);
  bcopy (&ifr.ifr_hwaddr.sa_data, addr, ETH_ALEN);
  *type = ifr.ifr_hwaddr.sa_family;
  return (0);
}

int				/* O - Exit status */
main(int  argc,			/* I - Number of command-line arguments (6 or 7) */
     char *argv[])		/* I - Command-line arguments */
{
  char		method[255],	/* Method in URI */
		hostname[1024],	/* Hostname */
  		*iface,		/* Interface name */
		username[255],	/* Username info (not used) */
		resource[1024];	/* Resource info (not used) */
  int		fp;		/* Print file */
  int		copies;		/* Number of copies to print */
  int		port;		/* Port number */
  int		delay;		/* Delay for retries... */
  int		fd;		/* AppSocket */
  int		error;		/* Error code (if any) */
  int		type;		/* ARPHRD */
  struct sockaddr_llc addr;	/* Socket address */
  struct sockaddr_llc laddr;	/* Socket address */
  struct ether_addr *eth;	/* Host address */
  int		wbytes;		/* Number of bytes written */
  int		nbytes;		/* Number of bytes read */
  size_t	tbytes;		/* Total number of bytes written */
  char		buffer[8192],	/* Output buffer */
		*bufptr;	/* Pointer into buffer */
  struct timeval timeout;	/* Timeout for select() */
  fd_set	input;		/* Input set for select() */
#if defined(HAVE_SIGACTION) && !defined(HAVE_SIGSET)
  struct sigaction action;	/* Actions for POSIX signals */
#endif /* HAVE_SIGACTION && !HAVE_SIGSET */


 /*
  * Make sure status messages are not buffered...
  */

  setbuf(stderr, NULL);

 /*
  * Ignore SIGPIPE signals...
  */

#ifdef HAVE_SIGSET
  sigset(SIGPIPE, SIG_IGN);
#elif defined(HAVE_SIGACTION)
  memset(&action, 0, sizeof(action));
  action.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &action, NULL);
#else
  signal(SIGPIPE, SIG_IGN);
#endif /* HAVE_SIGSET */

 /*
  * Check command-line...
  */

  if (argc == 1)
  {
    puts("network llcsocket \"Unknown\" \"LLCSocket/HP JetDirect\"");
    return (0);
  }
  else if (argc < 6 || argc > 7)
  {
    fprintf(stderr, "Usage: %s job-id user title copies options [file]\n",
            argv[0]);
    return (1);
  }

 /*
  * If we have 7 arguments, print the file named on the command-line.
  * Otherwise, send stdin instead...
  */

  if (argc == 6)
  {
    fp     = 0;
    copies = 1;
  }
  else
  {
   /*
    * Try to open the print file...
    */

    if ((fp = open(argv[6], O_RDONLY)) < 0)
    {
      perror("ERROR: unable to open print file");
      return (1);
    }

    copies = atoi(argv[4]);
  }

 /*
  * Extract the hostname and port number from the URI...
  */

  httpSeparate(argv[0], method, username, hostname, &port, resource);

  if (port == 0)
    port = 180;		/* Default to HP JetDirect/Tektronix PhaserShare */

 /*
  * Then try to connect to the remote host...
  */

  iface = index(hostname,'%');
  if (iface == NULL)
  {
    perror("ERROR: No interface specified");
    return (1);
  }
  iface[0] = 0;
  iface++;

  memset(&laddr, 0, sizeof(laddr));
  eth = (struct ether_addr *)laddr.sllc_mac;
  if (get_hwaddr (iface, eth, &type))
    return (1);

  laddr.sllc_family = AF_LLC;
  laddr.sllc_arphrd = type;
  laddr.sllc_sap = port;

  memset(&addr, 0, sizeof(addr));
  eth = (struct ether_addr *)addr.sllc_mac;

  if (ether_hostton(hostname, eth))
  {
    fprintf(stderr, "ERROR: Unable to locate printer in ethers \'%s\'\n",
            hostname);
    return (1);
  }

  fprintf(stderr, "INFO: Attempting to connect to host %s on SAP %d\n",
          hostname, port);

  addr.sllc_family = AF_LLC;
  addr.sllc_arphrd = type;
  addr.sllc_sap    = port;

  wbytes = 0;

  while (copies > 0)
  {
    for (delay = 5;;)
    {
      if ((fd = socket(AF_LLC, SOCK_STREAM, 0)) < 0)
      {
	perror("ERROR: Unable to create socket");
	return (1);
      }

      if (bind (fd, (struct sockaddr *)&laddr, sizeof (laddr)) < 0)
      {
	perror("ERROR: Unable to bind socket");
	return (1);
      }

      if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
      {
	error = errno;
	close(fd);
	fd = -1;

	if (error == ECONNREFUSED || error == EHOSTDOWN ||
            error == EHOSTUNREACH)
	{
	  fprintf(stderr, "INFO: Network host \'%s\' is busy; will retry in %d seconds...\n",
                  hostname, delay);
	  sleep(delay);

	  if (delay < 30)
	    delay += 5;
	}
	else
	{
	  perror("ERROR: Unable to connect to printer (retrying in 30 seconds)");
	  sleep(30);
	}
      }
      else
	break;
    }

   /*
    * Now that we are "connected" to the port, ignore SIGTERM so that we
    * can finish out any page data the driver sends (e.g. to eject the
    * current page...  Only ignore SIGTERM if we are printing data from
    * stdin (otherwise you can't cancel raw jobs...)
    */

    if (argc < 7)
    {
#ifdef HAVE_SIGSET /* Use System V signals over POSIX to avoid bugs */
      sigset(SIGTERM, SIG_IGN);
#elif defined(HAVE_SIGACTION)
      memset(&action, 0, sizeof(action));

      sigemptyset(&action.sa_mask);
      action.sa_handler = SIG_IGN;
      sigaction(SIGTERM, &action, NULL);
#else
      signal(SIGTERM, SIG_IGN);
#endif /* HAVE_SIGSET */
    }

   /*
    * Finally, send the print file...
    */

    copies --;

    if (fp != 0)
    {
      fputs("PAGE: 1 1\n", stderr);
      lseek(fp, 0, SEEK_SET);
    }

    fputs("INFO: Connected to host, sending print job...\n", stderr);

    tbytes = 0;
    while ((nbytes = read(fp, buffer, sizeof(buffer))) > 0)
    {
     /*
      * Write the print data to the printer...
      */

      tbytes += nbytes;
      bufptr = buffer;

      while (nbytes > 0)
      {
	if ((wbytes = send(fd, bufptr, nbytes, 0)) < 0)
	{
	  perror("ERROR: Unable to send print file to printer");
	  break;
	}

	nbytes -= wbytes;
	bufptr += wbytes;
      }

      if (wbytes < 0)
        break;

     /*
      * Check for possible data coming back from the printer...
      */

      timeout.tv_sec  = 0;
      timeout.tv_usec = 0;

      FD_ZERO(&input);
      FD_SET(fd, &input);
      if (select(fd + 1, &input, NULL, NULL, &timeout) > 0)
      {
       /*
	* Grab the data coming back and spit it out to stderr...
	*/

	if ((nbytes = recv(fd, buffer, sizeof(buffer), 0)) > 0)
	{
	  fprintf(stderr, "INFO: Received %d bytes of back-channel data!\n",
	          nbytes);
          print_backchannel((unsigned char *)buffer, nbytes);
        }
      }
      else if (argc > 6)
	fprintf(stderr, "INFO: Sending print file, %lu bytes...\n",
	        (unsigned long)tbytes);
    }

   /*
    * Shutdown the socket and wait for the other end to finish...
    */

    fputs("INFO: Print file sent, waiting for printer to finish...\n", stderr);

    shutdown(fd, 1);

    for (;;)
    {
     /*
      * Wait a maximum of 90 seconds for backchannel data or a closed
      * connection...
      */

      timeout.tv_sec  = 90;
      timeout.tv_usec = 0;

      FD_ZERO(&input);
      FD_SET(fd, &input);

#ifdef __hpux
      if (select(fd + 1, (int *)&input, NULL, NULL, &timeout) > 0)
#else
      if (select(fd + 1, &input, NULL, NULL, &timeout) > 0)
#endif /* __hpux */
      {
       /*
	* Grab the data coming back and spit it out to stderr...
	*/

	if ((nbytes = recv(fd, buffer, sizeof(buffer), 0)) > 0)
	{
	  fprintf(stderr, "INFO: Received %d bytes of back-channel data!\n",
	          nbytes);
          print_backchannel((unsigned char *)buffer, nbytes);
        }
	else
	  break;
      }
      else
        break;
    }

   /*
    * Close the socket connection...
    */

    close(fd);
  }

 /*
  * Close the input file and return...
  */

  if (fp != 0)
    close(fp);

  return (wbytes < 0);
}


/*
 * 'print_backchannel()' - Print the contents of a back-channel buffer.
 */

void
print_backchannel(const unsigned char *buffer,	/* I - Data buffer */
                  int                 nbytes)	/* I - Number of bytes */
{
  char	line[255],				/* Formatted line */
	*lineptr;				/* Pointer into line */


  for (lineptr = line; nbytes > 0; buffer ++, nbytes --)
  {
    if (*buffer < 0x20 || *buffer >= 0x7f)
    {
      snprintf(lineptr, sizeof(line) - (lineptr - line), "<%02X>", *buffer);
      lineptr += strlen(lineptr);
    }
    else
      *lineptr++ = *buffer;

    if ((lineptr - line) > 72)
    {
      *lineptr = '\0';
      fprintf(stderr, "DEBUG: DATA: %s\n", line);
      lineptr = line;
    }
  }

  if (lineptr > line)
  {
    *lineptr = '\0';
    fprintf(stderr, "DEBUG: DATA: %s\n", line);
  }
}


/*
 * End of llcsocket.c
 */

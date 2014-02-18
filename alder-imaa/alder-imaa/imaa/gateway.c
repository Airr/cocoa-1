#line 16357 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
#if !defined(_WIN32) && !defined(_WIN64) && !defined(WIN32) && !defined(WIN64)
#include <config.h>
#endif
#line 16122 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
#if !defined(_WIN32) && !defined(_WIN64) && !defined(WIN32) && !defined(WIN64)
#include <config.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#line 16144 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
#define PORT "3490" /* the port users will be connecting to */
#define bufsize 20
#define queuesize 5
#define true 1
#define false 0
#define BACKLOG 100 /* how many pending connections queue will hold */

#line 16140 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
void sigchld_handler (int s);
int DoService (int sd);

#line 16363 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
void imaStructuramaRemoterun (char *fname);
#line 15552 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
void * get_in_addr (struct sockaddr *sa);
int IMA_tcp_send (int sd, char *cmd);
int IMA_tcp_recv (int sd, char *cmd);
int sendall(int s, char *buf, int *len);
int recvall(int s, char *buf, int *len);

#line 15559 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
/* get sockaddr, IPv4 or IPv6: */
void *
get_in_addr (struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
    {
      return &(((struct sockaddr_in *) sa)->sin_addr);
    }

  return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int
sendall (int s, char *buf, int *len)
{
  int total = 0;                // how many bytes we've sent
  int bytesleft = *len;         // how many we have left to send
  int n;

  while (total < *len)
    {
      n = send (s, buf + total, bytesleft, 0);
      if (n == -1)
        {
          break;
        }
      total += n;
      bytesleft -= n;
    }

  *len = total;                 // return number actually sent here
  return n == -1 ? -1 : 0;      // return -1 on failure, 0 on success
}

int
recvall (int s, char *buf, int *len)
{
  int total = 0;                // how many bytes we've receive
  int bytesleft = *len;         // how many we have left to receive
  int n;

  while (total < *len)
    {
      n = recv (s, buf + total, bytesleft, 0);
      if (n == -1)
        {
          break;
        }
      total += n;
      bytesleft -= n;
    }

  *len = total;                 // return number actually sent here
  return n == -1 ? -1 : 0;      // return -1 on failure, 0 on success
}

int
IMA_tcp_send (int s, char *cmd)
{
  int len;
  /* int bytes_sent; */
  int lenbuf;
  char buf[bufsize];
  FILE *fp;
  int i;

  fp = fopen (cmd, "r");
  lenbuf = bufsize;
  while (lenbuf == bufsize)
    {
      lenbuf = 0;
      for (i = 0; i < bufsize; i++)
        {
          if (feof (fp))
            {
              break;
            }
          buf[i] = fgetc (fp);
          lenbuf++;
        }
      /* Send the length of buffer to send. */
      if (lenbuf != bufsize)
        {
          lenbuf--;
        }
      if (lenbuf > 0)
        {
          /* printf ("Sending: %d\n", lenbuf); */
          len = sizeof (int);
          if (sendall (s, (char *) &lenbuf, &len) == -1)
            {
              perror ("send");
              exit (1);
            }

          if (sendall (s, buf, &lenbuf) == -1)
            {
              perror ("send");
              exit (1);
            }
        }
    }
  len = sizeof (int);
  lenbuf = 999;
  if (sendall (s, (char *) &lenbuf, &len) == -1)
    {
      perror ("send");
      exit (1);
    }

  fclose (fp);
  fp = NULL;
  printf ("Sent: %s\n", cmd);
  return 0;
}


int
IMA_tcp_recv (int s, char *cmd)
{
  int len;
  int lenbuf;
  char buf[bufsize];
  FILE *fp;
  int i;

  fp = fopen (cmd, "w");
  lenbuf = bufsize;
  while (lenbuf == bufsize)
    {
      /* Receive the length of buffer. */
      len = sizeof (int);
      if (recvall (s, (char *) &lenbuf, &len) == -1)
        {
          perror ("recv");
          exit (1);
        }
      if (lenbuf == 999)
        {
          break;
        }
      /* printf ("Received: %d\n", lenbuf); */
      if (recvall (s, buf, &lenbuf) == -1)
        {
          perror ("recv");
          exit (1);
        }
      for (i = 0; i < lenbuf; i++)
        {
          fputc (buf[i], fp);
        }
    }
  if (lenbuf < bufsize)
    {
      len = sizeof (int);
      if (recvall (s, (char *) &lenbuf, &len) == -1)
        {
          perror ("recv");
          exit (1);
        }
      assert (lenbuf == 999);
    }

  fclose (fp);
  fp = NULL;
  printf ("Received: %s\n", cmd);
  return 0;

}
#line 16152 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
void
sigchld_handler (int s)
{
  while (waitpid (-1, NULL, WNOHANG) > 0);
}

#line 16159 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
int
main (void)
{
  /* listen on sock_fd, new connection on new_fd */
  int sockfd, new_fd;  
  struct addrinfo hints, *servinfo, *p;
  /* connector's address information */
  struct sockaddr_storage their_addr; 
  socklen_t sin_size;
  struct sigaction sa;
  int yes = 1;
  char s[INET6_ADDRSTRLEN];
  int rv;

  memset (&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;  /* use my IP */

  if ((rv = getaddrinfo (NULL, PORT, &hints, &servinfo)) != 0)
    {
      fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (rv));
      return 1;
    }

  /* Loop through all the results and bind to the first we can */
  for (p = servinfo; p != NULL; p = p->ai_next)
    {
      if ((sockfd = socket (p->ai_family, p->ai_socktype,
                            p->ai_protocol)) == -1)
        {
          perror ("server: socket");
          continue;
        }

      if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                      sizeof (int)) == -1)
        {
          perror ("setsockopt");
          exit (1);
        }

      if (bind (sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
          close (sockfd);
          perror ("server: bind");
          continue;
        }

      break;
    }

  if (p == NULL)
    {
      fprintf (stderr, "server: failed to bind\n");
      return 2;
    }

  freeaddrinfo (servinfo); /* all done with this structure */

  if (listen (sockfd, BACKLOG) == -1)
    {
      perror ("listen");
      exit (1);
    }

  sa.sa_handler = sigchld_handler; /* reap all dead processes */
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction (SIGCHLD, &sa, NULL) == -1)
    {
      perror ("sigaction");
      exit (1);
    }

  printf ("server: waiting for connections...\n");

  while (1)
    {
      /* Main accept() loop */
      sin_size = sizeof their_addr;
      new_fd = accept (sockfd, (struct sockaddr *) &their_addr, &sin_size);
      if (new_fd == -1)
        {
          perror ("accept");
          continue;
        }

      inet_ntop (their_addr.ss_family,
                 get_in_addr ((struct sockaddr *) &their_addr), s, sizeof s);
      printf ("server: got connection from %s\n", s);

      if (!fork ())
        {                       /* This is the child process */
          close (sockfd);       /* Child doesn't need the listener */

          DoService (new_fd);

          close (new_fd);
          exit (0);
        }
      close (new_fd);           /* parent doesn't need this */
    }

  return 0;
}

#line 16368 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
#undef RUNSTRUCTURAMA
#define GATEWAY
#line 16267 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
int
DoService (int sd)
{
#ifdef RUNSTRUCTURAMA
  FILE *fp;
#endif /* RUNSTRUCTURAMA */
  char cmd[500];
  int len;
  int lenbname;
  char *bname;

  /* 1. Receive the length of a string. */
  len = sizeof (int);
  if (recvall (sd, (char *) &lenbname, &len) == -1)
    {
      perror ("recv");
      exit (1);
    }
  /* 2. Receive the string. */
  bname = malloc ((lenbname + 1) * sizeof (char));
  /* printf ("Received: %d\n", lenbuf); */
  if (recvall (sd, bname, &lenbname) == -1)
    {
      perror ("recv");
      exit (1);
    }
  bname[lenbname] = '\0';

  /* 3. Receive structurama input. */
  sprintf (cmd, "%s.in", bname);
  IMA_tcp_recv (sd, cmd);
  /* 4. Receive assignment samples files. */
  sprintf (cmd, "%s.in.p", bname);
  IMA_tcp_recv (sd, cmd);

#ifdef RUNSTRUCTURAMA
  /* Prepare a batch file for structurama. */
  sprintf (cmd, "%s.bat", bname);
  fp = fopen (cmd, "w");
  fprintf (fp, "execute %s.in\nsum\nquit\n", bname);
  fclose (fp);
  fp = NULL;

  /* Run STRUCTURAMA. */
# if defined(DARWIN)
  sprintf (cmd, "%s/structurama_1.0 < %s.bat >> log.txt", STRUCTURAMA, bname);
#elif defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64) || defined(CYGWIN)
  sprintf (cmd, "%s/Structurama.exe < %s.bat >> log.txt", STRUCTURAMA, bname);
# endif
  system (cmd);

#else
  /* Connect to server */
  imaStructuramaRemoterun (bname);
#endif /* RUNSTRUCTURAMA */

  /* 5. Send result files back to the client. */
  sprintf (cmd, "%s.in.sum_assignments", bname);
  IMA_tcp_send (sd, cmd);
  sprintf (cmd, "%s.in.sum_dist.nex", bname);
  IMA_tcp_send (sd, cmd);
  sprintf (cmd, "%s.in.sum_pairs", bname);
  IMA_tcp_send (sd, cmd);

  /* 6. Remove all files. */
  sprintf (cmd, "%s.in", bname);
  unlink (cmd);
  sprintf (cmd, "%s.in.p", bname);
  unlink (cmd);
#ifdef RUNSTRUCTURAMA
  sprintf (cmd, "%s.bat", bname);
  unlink (cmd);
#endif /* RUNSTRUCTURAMA */
  sprintf (cmd, "%s.in.sum_assignments", bname);
  unlink (cmd);
  sprintf (cmd, "%s.in.sum_dist.nex", bname);
  unlink (cmd);
  sprintf (cmd, "%s.in.sum_pairs", bname);
  unlink (cmd);

  free (bname);
  bname = NULL;

  /* Wait for a next request. */
  return true;
}

#line 15973 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
void 
imaStructuramaRemoterun (char *fname)
{
  int lencmd;
  char *cmd;
  char *bname;
  int len;
  int lenbname;

  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];

  memset (&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  /* if ((rv = getaddrinfo ("172.17.34.101", PORT, &hints, &servinfo)) != 0) for
   * Hey Lab desktop3 */
  /* if ((rv = getaddrinfo ("172.17.34.102", PORT, &hints, &servinfo)) != 0) for
   * Hey Lab desktop4 */
  /* if ((rv = getaddrinfo ("10.0.0.1", PORT, &hints, &servinfo)) != 0) for
   * operon Genetics cluster. */

#ifdef GATEWAY
  if ((rv = getaddrinfo (SERVERADDRESS, PORT, &hints, &servinfo)) != 0)
    {
      fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (rv));
      return;
#else
  if ((rv = getaddrinfo (GATEWAYADDRESS, PORT, &hints, &servinfo)) != 0)
    {
      IM_errloc (AT, 
                 "The try of remote connection for structurama has failed: %s.",
                 gai_strerror (rv));
#endif /* GATEWAY */
    }

  /* Loop through all the results and connect to the first we can */
  for (p = servinfo; p != NULL; p = p->ai_next)
    {
      if ((sockfd = socket (p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
          perror ("client: socket");
          continue;
        }

      if (connect (sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
          close (sockfd);
          perror ("client: connect");
          continue;
        }
      break;
    }

  if (p == NULL)
    {
#ifdef GATEWAY
      fprintf (stderr, "client: failed to connect\n");
#else
      IM_errloc (AT, "IMa client: failed to connnect");
#endif /* GATEWAY */
    }

  inet_ntop (p->ai_family, 
             get_in_addr ((struct sockaddr *) p->ai_addr),
             s, sizeof s);
#ifdef GATEWAY
  printf ("client: connecting to %s\n", s);
#else
  printf ("IMa: connecting to %s\n", s);
#endif /* GATEWAY */

  freeaddrinfo (servinfo);	/* all done with this structure */

  lencmd = 65 + strlen (fname);
  bname = strrchr (fname, '/');
  if (bname == NULL)
    {
      bname = fname;
    }
  else
    {
      bname = bname + 1;
    }
  lenbname = strlen (bname);
  len = sizeof (int);
  /* 1. Send the length of a string. */
  if (sendall (sockfd, (char *)&lenbname, &len) == -1)
    {
#ifdef GATEWAY
      perror ("send");
#else
      IM_errloc (AT, "send the length of a lenbname: %d", lenbname);
#endif /* GATEWAY */
    }
  /* 2. Send the string. */
  if (sendall (sockfd, bname, &lenbname) == -1)
    {
#ifdef GATEWAY
      perror ("send");
#else
      IM_errloc (AT, "send the file name: %s", bname);
#endif /* GATEWAY */
    }

  cmd = malloc (lencmd * sizeof (char));
  /* 3. Send the input file. */
  sprintf (cmd, "%s.in", fname);
  IMA_tcp_send (sockfd, cmd); 
  /* 4. Send the assignment samples file. */
  sprintf (cmd, "%s.in.p", fname);
  IMA_tcp_send (sockfd, cmd); 
  /* 5. Receive the three output files. */
  sprintf (cmd, "%s.in.sum_assignments", fname);
  IMA_tcp_recv (sockfd, cmd); 
  sprintf (cmd, "%s.in.sum_dist.nex", fname);
  IMA_tcp_recv (sockfd, cmd); 
  sprintf (cmd, "%s.in.sum_pairs", fname);
  IMA_tcp_recv (sockfd, cmd); 

  printf ("Finished!\n");
  free (cmd);
  cmd = NULL;
  close (sockfd);
  return;
}





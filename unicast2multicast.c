/*

  unicast2multicast.c

  This program receives a unicast stream, and resends as a multicast stream

*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>

void showargs()
{
  fprintf(stderr, "unicast2multicast - unicast to multicast bridge\n\n");
  fprintf(stderr, "Syntax: -i incoming_ip:port -m multicast_group[:port] [-o outgoing_ip]\n");
}

int validate_multicast_addr(const char *mcstr)
{
  in_addr_t mcaddr;

  // Check for blank address
  if (mcstr[0]==0) return 0;

  // Convert address to number
  mcaddr=inet_network(mcstr);

  // Compare address to multicast ranges
  if ((mcaddr<inet_network("224.0.0.0")) || (mcaddr>inet_network("239.255.255.255")))
    return 0;

  return 1;
}

int main(int argc, char **argv)
{
  struct sockaddr_in inaddr;
  int insock;
  socklen_t inaddrlen;
  ssize_t incnt;
  char message[9000];

  struct sockaddr_in outaddr;
  int outsock;
  socklen_t outaddrlen;
  ssize_t outcnt;

  int argn=0;
  char ip_in[INET_ADDRSTRLEN+1]="";
  char ip_out[INET_ADDRSTRLEN+1]="";
  char multicast_addr[INET_ADDRSTRLEN+1]="";
  unsigned int port_in;
  unsigned int port_out;

  /*****************************************************************/
  /* Parse command line */

  // Check for no arguments
  if (argc==1)
  {
    showargs();
    return 1;
  }

  // Check arguments
  while (argn<argc)
  {
    if ((strcmp(argv[argn], "-i")==0) && ((argn+1)<argc))
    {
      char *colonpos;

      ++argn;

      colonpos=strstr(argv[argn], ":");
      if (colonpos!=NULL)
      {
        colonpos[0]=0;

        if (strlen(argv[argn])<INET_ADDRSTRLEN)
        {
          strcpy(ip_in, argv[argn]);

          if (sscanf(&colonpos[1], "%5u", &port_in)==0)
            port_in=0;
        }
        else
        {
          showargs();
          return 1;
        }
      }
      else
      {
        showargs();
        return 1;
      }
    }
    else
    if ((strcmp(argv[argn], "-m")==0) && ((argn+1)<argc))
    {
      char *colonpos;

      ++argn;

      colonpos=strstr(argv[argn], ":");
      if (colonpos!=NULL)
      {
        colonpos[0]=0;

        if (strlen(argv[argn])<INET_ADDRSTRLEN)
        {
          strcpy(multicast_addr, argv[argn]);

          if (sscanf(&colonpos[1], "%5u", &port_out)==0)
            port_out=0;
        }
        else
        {
          showargs();
          return 1;
        }
      }
      else
      {
        if (strlen(argv[argn])<INET_ADDRSTRLEN)
        {
          strcpy(multicast_addr, argv[argn]);
        }
        else
        {
          showargs();
          return 1;
        }
      }
    }
    else
    if ((strcmp(argv[argn], "-o")==0) && ((argn+1)<argc))
    {
      ++argn;

      if (strlen(argv[argn])<INET_ADDRSTRLEN)
      {
        strcpy(ip_out, argv[argn]);
      }
      else
      {
        showargs();
        return 1;
      }
    }

    ++argn;
  }

  /* validate arguments */
  if ((port_in==0) || (ip_in[0]==0) || (validate_multicast_addr(multicast_addr)==0))
  {
    showargs();
    return 1;
  }

  /*****************************************************************/
  /* set up input */

  insock=socket(AF_INET, SOCK_DGRAM, 0);
  if (insock<0)
  {
    perror("incoming socket");
    exit(1);
  }

  bzero((char *)&inaddr, sizeof(inaddr));
  inaddr.sin_family=AF_INET;
  inaddr.sin_addr.s_addr=htonl(INADDR_ANY);
  inaddr.sin_port=htons(port_in);
  inaddrlen=sizeof(inaddr);

  if (bind(insock, (struct sockaddr *) &inaddr, sizeof(inaddr))<0)
  {
    perror("incoming bind");
    exit(1);
  }

  /*****************************************************************/
  /* set up output */

  outsock=socket(AF_INET, SOCK_DGRAM, 0);
  if (outsock<0)
  {
    perror("outgoing socket");
    exit(1);
  }

  bzero((char *)&outaddr, sizeof(outaddr));
  outaddr.sin_family=AF_INET;
  outaddr.sin_addr.s_addr=inet_addr(multicast_addr);
  outaddr.sin_port=htons(port_out==0?port_in:port_out);
  outaddrlen=sizeof(outaddr);

  // Check for multicast output interface override
  if (ip_out[0]!=0)
  {
    struct in_addr iface;

    iface.s_addr=inet_addr(ip_out);
    if (setsockopt(outsock, IPPROTO_IP, IP_MULTICAST_IF, (char *)&iface, sizeof(iface))<0)
    {
      perror("setting multicast interface");
      exit(1);
     }
  }

  /*****************************************************************/
  /* main loop */

  while (1)
  {
    incnt=recvfrom(insock, message, sizeof(message), 0, (struct sockaddr *) &inaddr, &inaddrlen);

    if (incnt<0)
    {
      perror("recvfrom");
      exit(1);
    }
    else
      if (incnt==0)
      {
          break;
      }

    outcnt=sendto(outsock, message, incnt, 0, (struct sockaddr *) &outaddr, outaddrlen);
    if (outcnt<0)
    {
      perror("sendto");
      exit(1);
    }
  }

  return 0;
}


#include "config.h"
#include <sys/types.h>
#include <stdio.h>

#ifdef __linux__
# define _GNU_SOURCE
#ifndef __USE_POSIX199309
# define __USE_POSIX199309
#endif
#endif

#ifdef __SVR4
# define u_int32_t uint32_t
# define u_int16_t uint16_t
#endif

#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define COUNTMAX 8000
#define FTIME  0

#define	MAX_FAKE 100
#define	ERROR    -1
#define	TYPE_A   1
#define	TYPE_NS      0x2
#define	CLASS_IN     0x1
#define	TYPE_PTR 12
#define	CLASS_INET 1

#ifndef IPVERSION
  #define IPVERSION 4
#endif                  /* IPVERISON */

#ifndef IP_MAXPACKET
  #define IP_MAXPACKET 65535
#endif                  /* IP_MAXPACKET */

#define DNSHDRSIZE 12
struct udphdr {
    u_short source;     /* source port */
    u_short dest;       /* destination port */
    u_short len;        /* udp length */
    u_short check;      /* udp checksum */
};

#ifndef IPVERSION
  #define IPVERSION 4
#endif                  /* IPVERISON */

struct iphdr {
  u_char  ihl:4,        /* header length */
          version:4;    /* version */
  u_char  tos;          /* type of service */
  short   tot_len;      /* total length */
  u_short id;           /* identification */
  short   off;          /* fragment offset field */
#define IP_DF   0x4000  /* dont fragment flag */
#define IP_MF   0x2000  /* more fragments flag */
  u_char  ttl;          /* time to live */
  u_char  protocol;     /* protocol */
  u_short check;        /* checksum */
  struct  in_addr saddr, daddr;  /* source and dest address */
};

#ifndef IP_MAXPACKET
#define IP_MAXPACKET 65535
#endif                   /* IP_MAXPACKET */

#define   IPHDRSIZE     sizeof(struct iphdr)
#define   UDPHDRSIZE    sizeof(struct udphdr)


struct dnshdr {
    unsigned short int id;

    unsigned char  rd:1;           /* recursion desired */
    unsigned char  tc:1;           /* truncated message */
    unsigned char  aa:1;           /* authoritive answer */
    unsigned char  opcode:4;       /* purpose of message */
    unsigned char  qr:1;           /* response flag */

    unsigned char  rcode:4;        /* response code */
    unsigned char  unused:2;       /* unused bits */
    unsigned char  pr:1;           /* primary server required (non standard) */
    unsigned char  ra:1;           /* recursion available */

    unsigned short int que_num;
    unsigned short int rep_num;
    unsigned short int num_rr;
    unsigned short int num_rrsup;
};

int myrand(void);
unsigned short in_cksum(char *packet, int len);
int get_qname(const u_char *payload, size_t paylen, char* qname, u_int* id);
int spy_send_ipv4(int s, struct in_addr from, struct in_addr to,
	    unsigned sport, unsigned dport,
	    const u_char *dnspkt, unsigned dnslen);
int build_reponse(u_char *buf, unsigned size, unsigned *len,
	      uint16_t txid, uint16_t flags,
	      const char *q_name, const char *q_ip,
	      const char *domain, const char *auth_name, const char *auth_ip);
int format_domain(u_char *buf, unsigned size, unsigned *len, const char *name);
int format_qr(u_char *buf, unsigned size, unsigned *len, const char *name,
		uint16_t type, uint16_t class);
int format_rr(u_char *buf, unsigned size, unsigned *len, const char *name,
		uint16_t type, uint16_t class, uint32_t ttl, const char *data);



int get_qname(const u_char *payload, size_t paylen, char* qname, u_int* id)
{
	ns_msg msg;
	if (ns_initparse(payload, paylen, &msg) < 0) {
		fputs(strerror(errno), stderr);
		return 0;
	}

	int rrmax;
	ns_rr rr;

	rrmax = ns_msg_count(msg, ns_s_qd); //0
	if (rrmax != 1) {
		fputs(" 0", stderr);
		return 0;
	}
	//rrmax should be 1
	if (ns_parserr(&msg, ns_s_qd, 0, &rr)) {
		fputs(strerror(errno), stderr);
		return 0;
	}

	if (ns_rr_type(rr) != 1)
		return 0;

	*id = ns_msg_id(msg);
	strcpy(qname, ns_rr_name(rr));
	return 1;
}

int spy_answer(int s, struct in_addr from, struct in_addr to,
	    unsigned sport, unsigned dport, uint16_t txid,
	    const char *q_name, const char *q_ip)
{
	u_char dnsdata[1024];
	bzero(dnsdata, sizeof(dnsdata));
	unsigned len = 0;
	build_reponse(dnsdata, sizeof(dnsdata), &len,
		      txid, 1, q_name, q_ip, 0,0,0);
	spy_send_ipv4(s, from, to, sport, dport, dnsdata, len);
	return 1;
}

int spy_send_ipv4(int s, struct in_addr from, struct in_addr to,
	    unsigned sport, unsigned dport,
	    const u_char *dnspkt, unsigned dnslen)
{
	struct sockaddr_in sin;
    struct   iphdr   *ip;
    struct   udphdr  *udp;
    unsigned char    *data;
	unsigned char    packet[4024];

	ip     = (struct iphdr     *)packet;
	udp    = (struct udphdr    *)(packet+IPHDRSIZE);
	data   = (unsigned char    *)(packet+IPHDRSIZE+UDPHDRSIZE);
    memset(packet,0,sizeof(packet));

    udp->source  = htons(sport);
    udp->dest    = htons(dport);
    udp->len     = htons(UDPHDRSIZE + dnslen);
    udp->check   = 0;

    memcpy(data, dnspkt, dnslen);

    ip->saddr	 = from;
    ip->daddr	 = to;
    ip->version  = 4;             /*ip version*/
    ip->ihl      = 5;
    ip->ttl      = 245;
    ip->id       = random()%5985;
    ip->protocol = IPPROTO_UDP;   /*protocol type*/
    ip->tot_len  = htons(IPHDRSIZE + UDPHDRSIZE + dnslen);
    ip->check    = 0;
    ip->check    = in_cksum((char *)packet,IPHDRSIZE);

    sin.sin_family=AF_INET;
    sin.sin_addr = to;
    sin.sin_port=udp->dest;

    /* scratch IP package for debug
    printf ("src_port=%x,dst_port=%x,tot_len=%x, datasize =%d\n",sport,dport,ip->tot_len,datasize);
    for (itmp=0; itmp< IPHDRSIZE+UDPHDRSIZE;itmp++) printf("%2x ",packet[itmp]);
    for (itmp; itmp< IPHDRSIZE+UDPHDRSIZE+datasize;itmp++) printf("%2c ",packet[itmp]);
    printf("|||\n");
    */

    return(sendto(s, packet, IPHDRSIZE+UDPHDRSIZE+dnslen,
    		0, (struct sockaddr*)&sin, sizeof(struct sockaddr)));

}

int build_reponse(u_char *buf, unsigned size, unsigned *len,
	      uint16_t txid, uint16_t flags,
	      const char *q_name, const char *q_ip,
	      const char *domain, const char *auth_name, const char *auth_ip)
{
	u_char *out = buf;
	u_char *end = buf + size;
	u_char rec[256];
	unsigned l_rec;
	uint32_t ttl = 24 * 3600;

	struct dnshdr *dns;
	dns = (struct dnshdr *) buf;
	dns->id = htons(txid);
	dns->qr = 1; /* qr = 0: question packet   */
	dns->rd = 1;
	dns->ra = 1;
	dns->que_num = htons(1);
	dns->rep_num = htons(1);

	*len += sizeof(*dns); //12

	// queries
	format_qr(rec, sizeof(rec), &l_rec, q_name, TYPE_A, CLASS_IN);
	if (out + *len + l_rec > end) {
		fprintf(stderr, "dns_response overflow");
		return 0;

	}
	memcpy(out + *len, rec, l_rec);
	*len += l_rec;

	// answers
	format_rr(rec, sizeof(rec), &l_rec, q_name, TYPE_A, CLASS_IN, ttl, q_ip);
	if (out + *len + l_rec > end) {
		fprintf(stderr, "dns_response overflow");
		return 0;

	}
	memcpy(out + *len, rec, l_rec);

	*len += l_rec;
/*
	// authoritative nameservers
	format_rr(rec, sizeof(rec), &l_rec, domain, TYPE_NS, CLASS_IN, ttl,
			auth_name);
	if (out + *len + l_rec > end)
		fprintf(stderr, "dns_response overflow"), exit(1);
	memcpy(out + *len, rec, l_rec);
	*len += l_rec;

	// additional records
	format_rr(rec, sizeof(rec), &l_rec, auth_name, TYPE_A, CLASS_IN, ttl,
			auth_ip);
	if (out + *len + l_rec > end)
		fprintf(stderr, "dns_response overflow"), exit(1);
	memcpy(out + *len, rec, l_rec);
	*len += l_rec;
*/
	return 1;


}
void nameformat(const char *name, char *QS)
{
	char *bungle, *x;
	char elem[128];

	*QS = 0;
	bungle = malloc(strlen(name) + 3);
	strcpy(bungle, name);
	x = strtok(bungle, ".");
	while (x != NULL) {
		if (snprintf(elem, 128, "%c%s", strlen(x), x) == 128) {
			puts("String overflow.");
			exit(1);
		}
		strcat(QS, elem);
		x = strtok(NULL, ".");
	}
	free(bungle);
}
int format_domain(u_char *buf, unsigned size, unsigned *len, const char *name)
{
	unsigned bufi, i, j;
	bufi = i = j = 0;
	while (name[i]) {
		if (name[i] == '.') {
			if (bufi + 1 + (i - j) > size) {
				fprintf(stderr, "format_domain overflow\n");
				return 0;
			}
			buf[bufi++] = i - j;
			memcpy(buf + bufi, name + j, i - j);
			bufi += (i - j);
			j = i + 1;
		}
		i++;
	}
	if (bufi + 1 + 2 + 2 > size) {
		fprintf(stderr, "format_domain overflow\n");
		return 0;
	}
	buf[bufi++] = i - j;
	memcpy(buf + bufi, name + j, i - j);
	bufi += (i - j);

	buf[bufi++] = 0;
	*len = bufi;

	return 1;
}

int format_qr(u_char *buf, unsigned size, unsigned *len, const char *name, uint16_t type, uint16_t class)
{
	uint16_t tmp;
	// name
	if (!format_domain(buf, size, len, name))
		return 0;
//	nameformat(name, (char*)buf);
//	*len = strlen((char*)buf);
	// type
	tmp = htons(type);
	memcpy(buf + *len, &tmp, sizeof(tmp));
	*len += sizeof(tmp);
	// class
	tmp = htons(class);
	memcpy(buf + *len, &tmp, sizeof(tmp));
	*len += sizeof(tmp);
	return 1;
}

int format_rr(u_char *buf, unsigned size, unsigned *len, const char *name, uint16_t type, uint16_t class, uint32_t ttl, const char *data)
{
   if(!format_qr(buf, size, len, name, type, class))
	   return 0;
   // ttl
   ttl = htonl(ttl);
   memcpy(buf + *len, &ttl, sizeof (ttl));
   *len += sizeof (ttl);
   // data length + data
   uint16_t dlen;
   struct in_addr addr;

   switch (type)
   {
      case TYPE_A:
         dlen = sizeof (addr);
         break;
      case TYPE_NS:
         dlen = strlen(data) + 1;
         break;
      default:
         fprintf(stderr, "format_rr: unknown type %02x", type);
         return 0;
   }
   dlen = htons(dlen);
   memcpy(buf + *len, &dlen, sizeof (dlen));
   *len += sizeof (dlen);
   // data
   unsigned len2;
   switch (type)
   {
      case TYPE_A:
         if (inet_pton(AF_INET, data, (void*)&addr) < 0)
         {
        	 fprintf(stderr, "invalid destination IP: %s", data);
        	 return 0;
         }
         memcpy(buf + *len, &addr, sizeof (addr));
         *len += sizeof (addr);
         break;
      case TYPE_NS:
         format_domain(buf + *len, size - *len, &len2, data);
         *len += len2;
         break;
      default:
         fprintf(stderr, "format_rr: unknown type %02x", type);
         return 0;
   }
   return 1;
}

int myrand(void)
{
    int j;
    j=1+(int) (150.0*rand()/(RAND_MAX+1.0));
    return(j);
}

unsigned short in_cksum(char *packet, int len)
{
   register int nleft = len;
   register u_short *w = (u_short *)packet;
   register int sum = 0;
   u_short answer = 0;

   /*
   * Our algorithm is simple, using a 32 bit accumulator (sum), we add
   * sequential 16 bit words to it, and at the end, fold back all the
   * carry bits from the top 16 bits into the lower 16 bits.
   */
   while (nleft > 1)
   {
       sum += *w++;
       nleft -= 2;
   }

   /* mop up an odd byte, if necessary */

   if (nleft == 1)
   {
       *(u_char *)(&answer) = *(u_char *)w ;
       sum += answer;
   }

   /* add back carry outs from top 16 bits to low 16 bits */
   sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
   sum += (sum >> 16);                 /* add carry */
   answer = ~sum;                      /* truncate to 16 bits */
   return(answer);
}

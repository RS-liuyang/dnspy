
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "radix_tree.h"

int main(void)
{
	u_char* CIDR1= "192.168.0.0/32";

	in_cidr_t cidr1;
	intptr_t result;
	result = ptocidr(CIDR1, &cidr1);
	fprintf(stderr, "result is %d\n",result);

	cidr1.addr = ntohl(cidr1.addr);
	cidr1.mask = ntohl(cidr1.mask);
	fprintf(stderr, "host addr is %u\n", (uint32_t)cidr1.addr);
	fprintf(stderr, "host mask is %u\n", (uint32_t)cidr1.mask);

	radix_tree_t *tree = radix_tree_create();

	result = radix32tree_insert(tree, cidr1.addr, cidr1.mask, 5);
	fprintf(stderr, "insert result is %d\n",result);

	const char* ip1="192.168.0.1";
	struct in_addr		a4;
	uint32_t	hip1;
	inet_pton(AF_INET, ip1, &a4);
	hip1 = ntohl(a4.s_addr);

	result = radix32tree_find(tree, hip1);
	fprintf(stderr, "find result is %d\n",result);

	//struct in_addr		a4;
	//if(inet_pton(AF_INET, IP1, &a4) > 0)
	//	fprintf(stderr, "inet_pton %s \n", IP1);

	return 1;
}

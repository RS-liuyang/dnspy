#ifndef SEND_H
#define SEND_H

int get_qname(const u_char *payload, size_t paylen, char* qname, u_int* id);
int spy_answer(int s, struct in_addr from, struct in_addr to,
	    unsigned sport, unsigned dport, uint16_t txid,
	    const char *q_name, const char *q_ip);


#endif

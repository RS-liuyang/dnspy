
#include <string.h>
#include "dmip.h"

struct _DMIP{
	char	domain[128];
	int		num;
	int		cur;
	char*	ip[40];
};

DMIP* new_dmip(const char* domain)
{
	DMIP* mydmip = (DMIP*)malloc(sizeof(DMIP));
	memset(mydmip, 0, sizeof(DMIP));
	strcpy(mydmip->domain, domain);
	return mydmip;
}

void add_dmip(DMIP* mydmip, const char* ip)
{
	mydmip->ip[mydmip->num++] = strdup(ip);
}

const char* get_dmip(DMIP* mydmip)
{
	if(mydmip->num == 1)
	{
		mydmip->cur = 1;
		return mydmip->ip[0];
	}

	int cur = mydmip->cur;
	if(cur >= mydmip->num)
		cur = 0;
	mydmip->cur=cur+1;
	return mydmip->ip[cur];
}

char* get_dmname(DMIP* mydmip)
{
	return mydmip->domain;
}

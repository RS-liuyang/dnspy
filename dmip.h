
#ifndef DMIP_H
#define DMIP_H

typedef struct _DMIP DMIP;

DMIP* new_dmip(const char* domain);

void add_dmip(DMIP* mydmip, const char* ip);

const char* get_dmip(DMIP* mydmip);

char* get_dmname(DMIP* mydmip);

#endif

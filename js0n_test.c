#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "js0n.h"

int main(int argc, char **argv)
{
	unsigned char buff[1024], *json = NULL;
	int len, lastlen=0, ret;
	unsigned short *res;
	FILE *f;
	int loops;
	
	if((f = fopen(argv[1],"r")) == NULL)
	{
		printf("uhoh opening %s\n",argv[1]);
		exit(1);
	}
	while((len = fread(buff,1,1024,f)) > 0)
	{
		json = realloc(json,lastlen+len);
		memcpy(json+lastlen,buff,len);
		lastlen+=len;
	}
	fclose(f);
	loops = 1000000;
	while (loops-- > 0) {
		//printf("got[%.*s]\n",lastlen,json);
		res = malloc(lastlen); // way more than enough
		memset(res,0,lastlen);
		ret = js0n(json,lastlen,res);
		if (0 && ret)
			exit(0);
		free(res);
	}
	return 0;
}


#include <stdio.h>
#include "pnox.h"

struct input {
	char message[32];
} *input;

struct output {
	char message[32];
} *output;

int test1000(tHandle, mHandle)
T_HAND *tHandle;
M_HAND *mHandle;
{
	int num;

	input = (struct input *)mHandle->rcvb;
	output = (struct output *)mHandle->sndb;

	memcpy(output->message, input->message, sizeof(output->message));
	mHandle->sndl = sizeof(output->message);
	return(0);
}

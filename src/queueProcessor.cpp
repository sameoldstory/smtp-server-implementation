#include "queueProcessor.h"
#include <stdio.h>

QueueProcessor::QueueProcessor(int seconds): timeout(seconds)
{

}

void QueueProcessor::Run()
{
	printf("QueueProcessor::Run\n");
}

int QueueProcessor::GetTimeout()
{
	return timeout;
}
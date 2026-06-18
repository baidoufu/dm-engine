#include "StdAfx.h"
#include ".\eventprocessor.h"

CEventProcessor::CEventProcessor(VOID) : xListHost<CEventProcessor>::xListNode(this)
{
}

CEventProcessor::~CEventProcessor(VOID)
{
}
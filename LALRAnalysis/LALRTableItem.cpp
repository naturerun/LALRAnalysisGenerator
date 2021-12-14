#include "pch.h"
#include "LALRTableItem.h"

LALRTableItem::LALRTableItem(const LALRTableItem& copy)
{
	ActionType = copy.ActionType;
	switch (copy.ActionType)
	{
	case ACCEPT:
	case ERROR: new (&NULLLable) string(copy.NULLLable); break;
	case MOVE: LALRStateNumber = copy.LALRStateNumber; break;
	case REDUCTION: production = copy.production; break;

	}
}
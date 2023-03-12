#include "PrjStat.h"

NEOPAD_STAT::NEOPAD_STAT()
{
	memset(arr, 0, sizeof(arr));
}

int NEOPAD_STAT::CalcStatistics(DocItem* node)
{
	int total = 1;
	switch (node->status) {
	case ETreeStatus::TS_READY:
		x100++;
		break;
	case ETreeStatus::TS_ALMOST:
		x99++;
		break;
	case ETreeStatus::TS_75:
		x75++;
		break;
	case ETreeStatus::TS_50:
		x50++;
		break;
	case ETreeStatus::TS_25:
		x25++;
		break;
	case ETreeStatus::TS_UNREADY:
		x0++;
		break;
	case ETreeStatus::TS_LOCKED:
		xLock++;
		break;
	default:
		xUnd++;
		break;
	}
	ELangStatus ls = node->GetLangStatus(1);
	switch (ls) {
	case ELangStatus::LS_NONE:		// no document
		tNone++;
		break;
	case ELangStatus::LS_OK:		// document is newer than the original
		tOk++;
		break;
	case ELangStatus::LS_OLD:		// original is newer than document - this document needs updating
		tOld++;
		break;
	case ELangStatus::LS_QOK:		// the document is newer than the original, one of the dates is unreliable
		tQok++;
		break;
	case ELangStatus::LS_QOLD:
		tQold++;
		break;
	default:
		tUnd++;
		break;
	}

	for (auto tpos : node->children)
	{
		total += CalcStatistics(tpos);
	}
	return total;
}
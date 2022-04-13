#pragma once
#include <list>
#include <QString>
#include <functional>
#include "PrjItem.h"


class PrjTree
{
public:
	PrjTree();
	~PrjTree();
	MTPOS GetRoot();
	MTPOS AddRoot();
	void  RemoveAll();

    void ForEach(const std::function<void(MTPOS)> &fn);

	MTPOS AddCTail(MTPOS pos, MT_ITEM *item = nullptr);
	MTPOS AddAfter(MTPOS pos, MT_ITEM *item = nullptr);
	MTPOS GetPrevSibling(MTPOS pos);
	MTPOS GetNextSibling(MTPOS pos);

	static bool  IsMovePossible(MTPOS tpItem, MTPOS tpNewPar);
	static bool  IsExchangePossible(MTPOS pos1, MTPOS pos2);

	void  Exchange(MTPOS pos1, MTPOS pos2);
protected:
    void ForEachLevel(MTPOS node, const std::function<void(MTPOS)> &fn);
protected:
	MTPOS m_root = nullptr;
};


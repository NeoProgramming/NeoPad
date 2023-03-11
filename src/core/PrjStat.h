#pragma once

#include "DocItem.h"

// statistics
struct NEOPAD_STAT 
{
	union {
		struct {
			int x0;		// under construction
			int x25;	
			int x50;
			int x75;
			int x99;	// almost ready
			int x100;	// ready
			int xLock;	// locked
			int xUnd;	// undefined (default)

			int tNone;
			int tOk;
			int tOld;
			int tQok;
			int tQold;
			int tUnd;	// undefined (default)
		};
		int arr[14];
	};

	NEOPAD_STAT();
	int     CalcStatistics(DocItem* node);
};

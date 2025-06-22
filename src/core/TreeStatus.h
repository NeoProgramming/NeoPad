#pragma once

// statuses (pictures) of the main items in the tree
enum class ETreeStatus
{
	TS_UNKNOWN,		// unknown what ...
	TS_READY,		// regular document
	TS_ALMOST,
	TS_75,
	TS_50,
	TS_25,
	TS_UNREADY,		// unfinished document
	TS_LOCKED,		// ready but blocked for publication
	TS_IMPORTANT,
	TS_FOLDER,

	TS_ITEMS_COUNT
};

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
	TS_PINNED,
	TS_QUICK,
	TS_FOLDER,
	TS_EMPTY,

	TS_ITEMS_COUNT
};

// statuses (pictures) of transfers in the tree
enum class ELangStatus
{
	LS_NONE,	// no translation
	LS_OK,		// translation is newer than the original
	LS_OLD,		// the original is newer than the translation - you need to translate
	LS_QOK,     // the translation is newer than the original, one of the dates is unreliable
	LS_QOLD,    // the original is newer than the translation, one of the dates is unreliable
	LS_ITEMS_COUNT
};


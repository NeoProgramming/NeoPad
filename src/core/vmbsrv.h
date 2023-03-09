#pragma once

// maximal bases count
#define BCNT	2

const char AppTitle[] = "NeoPad";


struct NeopadBook
{
	QString suffix;
	QString title;
	QString rpath;			// path relative to NPBase
	QString csspath;
	QString load_prefix;	// for Jekyll
	QString save_prefix;	// for Jekyll
	bool path_is_unique;	// unique path requiring file operations
};

namespace MBA
{
	const char extVmbase[] = ".vmbase";
	const char extHtml[] = ".html";

	const char node[] = "node";

	const char id[] = "id";
	const char type[] = "type";
	const char title[] = "title";
	const char status[] = "s";
};

enum class ImageAction : int {
	Href,
	Copy,
	Move,
	//HardLink,
	//SymLink,
	Embed,
	Extract
};


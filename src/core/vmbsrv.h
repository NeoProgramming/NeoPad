#pragma once

// maximal bases count
#define BCNT	2

const char AppTitle[] = "NeoPad";


struct NeopadBase
{
	QString suffix;
	QString title;
	QString rpath;			// path relative to NPBase
	QString csspath;
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


#pragma once

const char AppTitle[] = "NeoPad";

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

void CopyLink(const char *uid);

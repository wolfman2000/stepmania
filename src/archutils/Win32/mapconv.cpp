//	mapconv - symbolic debugging info generator for VirtualDub

#include <vector>
#include <algorithm>
#include <string>
#include <cctype>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int constexpr MAX_FNAMBUF{ 0x0FFFFFFF };
int constexpr MAX_SEGMENTS{ 64 };
int constexpr MAX_GROUPS{ 64 };

struct RVAEnt {
	long rva;
	std::string line;
};

struct RVASorter {
	bool operator()(RVAEnt const& e1, RVAEnt const& e2) {
		return e1.rva < e2.rva;
	}
};

#if 0

std::vector<RVAEnt> rvabuf;

char fnambuf[MAX_FNAMBUF];
char *fnamptr = fnambuf;

long segbuf[MAX_SEGMENTS][2];
int segcnt=0;
int seggrp[MAX_SEGMENTS];
long grpstart[MAX_GROUPS];

char line[8192];
long codeseg_flags = 0;
FILE *f, *fo;

char *strtack(char *s, const char *t, const char *s_max) {
	while(s < s_max && (*s = *t))
		++s, ++t;

	if (s == s_max)
		return nullptr;

	return s+1;
}

bool readline() {
	if (!fgets(line, sizeof line, f))
		return false;

	int l = strlen(line);

	if (l>0 && line[l-1]=='\n')
		line[l-1]=0;

	return true;
}

bool findline(const char *searchstr) {
	while(readline()) {
		if (strstr(line, searchstr))
			return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////

/* dbghelp UnDecorateSymbolName() doesn't handle anonymous namespaces,
 * which look like "?A0x30dd143a".  Remove "@?A0x????????"; we don't
 * want to see "<anonymous namespace>::" in crash dump output, anyway. */
void RemoveAnonymousNamespaces( char *p )
{
	while( p = strstr( p, "@?A" ) )
	{
		int skip = 0, i;
		if( strlen(p) < 13 )
			break;

		for( i = 5; i < 13; ++i )
			if( !isxdigit(p[i]) )
				skip = 1;
		if( p[3] != '0' || p[4] != 'x' )
			skip = 1;
		if( skip )
		{
			++p;
			continue;
		}

		memmove( p, p+13, strlen(p+13)+1 );
	}

}

void parsename(long rva, char *func_name) {
	RemoveAnonymousNamespaces( func_name );
	 
	fnamptr = strtack(fnamptr, func_name, fnambuf+MAX_FNAMBUF);
	if(!fnamptr)
		throw "Too many func names; increase MAX_FNAMBUF.";
}

struct RVASorter {
	bool operator()(const RVAEnt& e1, const RVAEnt& e2) {
		return e1.rva < e2.rva;
	}
};

int main(int argc, char **argv) {
	int i;
	long load_addr;

	if (argc<3) {
		printf("mapconv <listing-file> <output-name>\n");
		return 0;
	}

	// TODO: Choose a better default for the vdi file.
	int ver = 20151002;

	if (!(f=fopen(argv[1], "r"))) {
		printf("can't open listing file \"%s\"\n", argv[1]);
		return 20;
	}

	if (!(fo=fopen(argv[2], "wb"))) {
		printf("can't open output file \"%s\"\n", argv[2]);
		return 20;
	}

	// Begin parsing file

	try {
		line[0] = 0;

//		printf("Looking for segment list.\n");

		if (!findline("Start         Length"))
			throw "can't find segment list";

//		printf("Reading in segment list.\n");

		while(readline()) {
			long grp, start, len;

			if (3!=sscanf(line, "%lx:%lx %lx", &grp, &start, &len))
				break;

			if (strstr(line+49, "CODE")) {
//				printf("%04x:%08lx %08lx type code\n", grp, start, len);

				codeseg_flags |= 1<<grp;

				segbuf[segcnt][0] = start;
				segbuf[segcnt][1] = len;
				seggrp[segcnt] = grp;
				++segcnt;
			}
		}

//		printf("Looking for public symbol list.\n");

		if (!findline("Publics by Value"))
			throw "Can't find public symbol list.";

		readline();

//		printf("Found public symbol list.\n");

		while(readline()) {
			long grp, start, rva;
			char symname[2048];
			int i;

			if (4!=sscanf(line, "%lx:%lx %s %lx", &grp, &start, symname, &rva))
				break;

			if (!(codeseg_flags & (1<<grp)) && strcmp(symname, "___ImageBase") )
				continue;

			RVAEnt entry = { rva, strdup(line) };

			rvabuf.push_back(entry);

//			parsename(rva,symname);
		}

//		printf("Looking for static symbol list.\n");

		if (!findline("Static symbols"))
			printf("WARNING: No static symbols found!\n");
		else {
			readline();

			while(readline()) {
				long grp, start, rva;
				char symname[4096];

				if (4!=sscanf(line, "%lx:%lx %s %lx", &grp, &start, symname, &rva))
					break;

				if (!(codeseg_flags & (1<<grp)))
					continue;

				RVAEnt entry = { rva, strdup(line) };

				rvabuf.push_back(entry);

	//			parsename(rva,symname);
			}
		}

//		printf("Sorting RVA entries...\n");

		std::sort(rvabuf.begin(), rvabuf.end(), RVASorter());

//		printf("Processing RVA entries...\n");

		for(i=0; i<rvabuf.size(); i++) {
			long grp, start, rva;
			char symname[4096];

			sscanf(rvabuf[i].line, "%lx:%lx %s %lx", &grp, &start, symname, &rva);

			grpstart[grp] = rva - start;

			parsename(rva, symname);
		}
		
//		printf("Processing segment entries...\n");

		for(i=0; i<segcnt; i++) {
			segbuf[i][0] += grpstart[seggrp[i]];
//			printf("\t#%-2d  %08lx-%08lx\n", i+1, segbuf[i][0], segbuf[i][0]+segbuf[i][1]-1);
		}
/*
		printf("Raw statistics:\n");
		printf("\tRVA bytes:        %ld\n", rvabuf.size()*4);
		printf("\tFunc name bytes:  %ld\n", fnamptr - fnambuf);

		printf("\nPacking RVA data..."); fflush(stdout);
*/
		std::vector<RVAEnt>::iterator itRVA = rvabuf.begin(), itRVAEnd = rvabuf.end();
		std::vector<char> rvaout;
		long firstrva = (*itRVA++).rva;
		long lastrva = firstrva;

		for(; itRVA != itRVAEnd; ++itRVA) {
			long rvadiff = (*itRVA).rva - lastrva;

			lastrva += rvadiff;

			if (rvadiff & 0xF0000000) rvaout.push_back((char)(0x80 | ((rvadiff>>28) & 0x7F)));
			if (rvadiff & 0xFFE00000) rvaout.push_back((char)(0x80 | ((rvadiff>>21) & 0x7F)));
			if (rvadiff & 0xFFFFC000) rvaout.push_back((char)(0x80 | ((rvadiff>>14) & 0x7F)));
			if (rvadiff & 0xFFFFFF80) rvaout.push_back((char)(0x80 | ((rvadiff>> 7) & 0x7F)));
			rvaout.push_back((char)(rvadiff & 0x7F));
		}

//		printf("%ld bytes\n", rvaout.size());

		// dump data

		static const char header[64]="symbolic debug information\r\n\x1A";

		fwrite(header, 64, 1, fo);

		long t;

		t = ver;
		fwrite(&t, 4, 1, fo);

		t = rvaout.size() + 4;
		fwrite(&t, 4, 1, fo);

		t = fnamptr - fnambuf;
		fwrite(&t, 4, 1, fo);

		t = segcnt;
		fwrite(&t, 4, 1, fo);

		fwrite(&firstrva, 4, 1, fo);
		fwrite(&rvaout[0], rvaout.size(), 1, fo);
		fwrite(fnambuf, fnamptr - fnambuf, 1, fo);
		fwrite(segbuf, segcnt*8, 1, fo);

		// really all done

		if (fclose(fo))
			throw "output file close failed";
		
	} catch(const char *s) {
		fprintf(stderr, "%s: %s\n", argv[1], s);
	}

	fclose(f);

	return 0;
}

#else

#include <fstream>
#include <iostream>
#include <tuple>
#include <array>

class MapConverter
{
public:
	MapConverter(std::string const &map, std::string const &vdi): 
		mapFile{map, std::ios::in}, vdiFile{vdi, std::ios::out | std::ios::binary},
		mapLine{}, codeSegFlags{ 0 }, segBuf{}, rvaBuffers{}, grpStarts{}
	{}

	~MapConverter()
	{
		mapFile.close();
		vdiFile.close();
	}

	bool CanRead() const
	{
		return mapFile.good();
	}

	bool CanWrite() const
	{
		return vdiFile.good();
	}
	void ReadSegmentList()
	{
		if (!TryFindLine("Start         Length"))
		{
			throw "Cannot find the segment list!";
		}

		long grp{ 0 };
		long start{ 0 };
		long len{ 0 };
		while (TryReadLine())
		{
			if (3 != std::sscanf(mapLine.c_str(), "%lx:%lx %lx", &grp, &start, &len))
			{
				break;
			}

			if (mapLine.find("CODE", 49) != std::string::npos)
			{
				codeSegFlags |= 1 << grp;
				// Consider a class or struct here.
				segBuf.push_back(std::make_tuple(start, len, grp));
			}
		}
	}
	void ReadPublicSymbols()
	{
		if (!TryFindLine("Publics by Value"))
		{
			throw "Cannot find the public symbol list!";
		}

		long grp{ 0 };
		long start{ 0 };
		long rva{ 0 };
		char symbolName[2048];

		while (TryReadLine())
		{
			if (4 != std::sscanf(mapLine.c_str(), "%lx:%lx %s %lx", &grp, &start, symbolName, &rva))
			{
				break;
			}

			if (!(codeSegFlags & (1 << grp)) && std::strcmp(symbolName, "___ImageBase"))
			{
				continue;
			}

			RVAEnt entry{ rva, mapLine };
			rvaBuffers.push_back(entry);
		}
	}

	void ReadStaticSymbols()
	{
		if (!TryFindLine("Static symbols"))
		{
			std::cout << "WARNING: No static symbols found!\n";
			return;
		}

		TryReadLine(); // Blank line we don't care about.
		long grp{ 0 };
		long start{ 0 };
		long rva{ 0 };
		char symbolName[4096];
		while (TryReadLine())
		{
			if (4 != std::sscanf(mapLine.c_str(), "%lx:%lx %s %lx", &grp, &start, symbolName, &rva))
			{
				break;
			}
			// TODO: Move this logic to a separate function.
			if (!(codeSegFlags & (1 << grp)))
			{
				continue;
			}

			RVAEnt entry{ rva, mapLine };
			rvaBuffers.push_back(entry);
		}
	}
	void ProcessRvaEntries()
	{
		std::sort(rvaBuffers.begin(), rvaBuffers.end(), RVASorter());

		long grp{ 0 };
		long start{ 0 };
		long rva{ 0 };
		char symbolName[4096];

		for (auto &item : rvaBuffers)
		{
			// Re-read the line data again.
			std::sscanf(item.line.c_str(), "%lx:%lx %s %lx", &grp, &start, symbolName, &rva);

			grpStarts[grp] = rva - start;
		}
	}

	void WriteBinary()
	{
		std::string header = "symbolic debug information\r\n\x1A";
		vdiFile.write(header.c_str(), header.size());
		char constexpr nullByte{ '\0' };
		for (int i{ 0 }; i < 34; ++i)
		{
			vdiFile.write(&nullByte, 1);
		}
	}
private:
	std::string RemoveAnonymousNamespace(std::string ns)
	{
		while (ns.find("@?A") != std::string::npos)
		{
			int skip{ 0 };
			// already eliminated them all.
			if (ns.size() < 13)
			{
				break;
			}

			for (int i{ 5 }; i < 13; ++i)
			{
				if (!std::isxdigit(ns[i]))
				{
					skip = 1;
				}
			}

			if (ns[3] != '0' || ns[4] != 'x')
			{
				skip = 1;
			}
			if (skip)
			{
				ns = ns.substr(1);
			}
			else
			{
				ns = ns.substr(13);
			}
		}
		return ns;
	}
	bool TryReadLine()
	{
		if (mapFile.eof())
		{
			return false;
		}
		std::getline(mapFile, mapLine);
		return true;
	}
	bool TryFindLine(std::string const &target)
	{
		while (TryReadLine())
		{
			if (mapLine.find(target) != std::string::npos)
			{
				return true;
			}
		}
		// no more left.
		return false;
	}

	std::ifstream mapFile;
	std::ofstream vdiFile;
	std::string mutable mapLine;
	long codeSegFlags;
	std::vector<std::tuple<long, long, long>> segBuf;
	std::vector<RVAEnt> rvaBuffers;
	std::array<long, MAX_GROUPS> grpStarts;
};

int main(int argc, char **argv) {
	int i{ 0 };
	long load_addr{ 0 };

	if (argc < 3) {
		printf("mapconv <listing-file> <output-name>\n");
		return 10;
	}

	// TODO: Replace with vertub's version_date.
	int ver = 20151002;

	MapConverter converter{ argv[1], argv[2] };
	if (!converter.CanRead())
	{
		std::cout << "Can't open listing file \"" << argv[1] << "\"\n";
		return 20;
	}
	if (!converter.CanWrite())
	{
		std::cout << "Can't open output file \"" << argv[2] << "\"\n";
		return 25;
	}
	try {
		converter.ReadSegmentList();
		converter.ReadPublicSymbols();
		converter.ReadStaticSymbols();
		converter.WriteBinary();
	}
	catch (std::string const &s)
	{
		std::cerr << argv[1] << ": " << s << std::endl;
	}

	return 0;
}

#endif

/*
 * (c) 2002 Avery Lee
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __stepmania__LunaAnnouncerManager__
#define __stepmania__LunaAnnouncerManager__

#include "LuaBinding.h"
#include "AnnouncerManager.h"

/** @brief Allow Lua to have access to the AnnouncerManager. */
class LunaAnnouncerManager: public Luna<AnnouncerManager>
{
public:
	static int DoesAnnouncerExist( T* p, lua_State *L ) { lua_pushboolean(L, p->DoesAnnouncerExist( SArg(1) )); return 1; }
	static int GetAnnouncerNames( T* p, lua_State *L )
	{
		vector<RString> vAnnouncers;
		p->GetAnnouncerNames( vAnnouncers );
		LuaHelpers::CreateTableFromArray(vAnnouncers, L);
		return 1;
	}
	static int GetCurrentAnnouncer( T* p, lua_State *L )
	{
		RString s = p->GetCurAnnouncerName();
		if( s.empty() )
			return 0;
		lua_pushstring(L, s );
		return 1;
	}
	static int SetCurrentAnnouncer( T* p, lua_State *L )
	{
		RString s = SArg(1);
		// only bother switching if the announcer exists. -aj
		if(p->DoesAnnouncerExist(s))
			p->SwitchAnnouncer(s);
		return 0;
	}
	
    LUNA_FILE_ALLOW_GLOBAL(LunaAnnouncerManager, ANNOUNCER, ANNOUNCER);
	
	LUNA_FILE_ADD_PUSH(AnnouncerManager);
};

#endif /* defined(__stepmania__LunaAnnouncerManager__) */

/*
 * (c) 2001-2014 Chris Danford
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


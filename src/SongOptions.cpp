#include "global.h"
#include "SongOptions.h"
#include "RageUtil.h"
#include "GameState.h"
#include "CommonMetrics.h"

using std::vector;

static const char *AutosyncTypeNames[] = {
	"Off",
	"Song",
	"Machine",
	"Tempo",
};
XToString( AutosyncType );
XToLocalizedString( AutosyncType );
LuaXType( AutosyncType );

static const char *SoundEffectTypeNames[] = {
	"Off",
	"Speed",
	"Pitch",
};
XToString( SoundEffectType );
XToLocalizedString( SoundEffectType );
LuaXType( SoundEffectType );

void SongOptions::Init()
{
	m_bAssistClap = false;
	m_bAssistMetronome = false;
	m_fMusicRate = 1.0f;
	m_SpeedfMusicRate = 1.0f;
	m_fHaste = 0.0f;
	m_SpeedfHaste = 1.0f;
	m_AutosyncType = AutosyncType_Off;
	m_SoundEffectType = SoundEffectType_Off;
	m_bStaticBackground = false;
	m_bRandomBGOnly = false;
	m_bSaveScore = true;
	m_bSaveReplay = false; // don't save replays by default?
}

void SongOptions::Approach( const SongOptions& other, float fDeltaSeconds )
{
#define APPROACH( opt ) \
	fapproach( m_ ## opt, other.m_ ## opt, fDeltaSeconds * other.m_Speed ## opt );
#define DO_COPY( x ) \
	x = other.x;

	APPROACH( fMusicRate );
	APPROACH( fHaste );
	DO_COPY( m_bAssistClap );
	DO_COPY( m_bAssistMetronome );
	DO_COPY( m_AutosyncType );
	DO_COPY( m_SoundEffectType );
	DO_COPY( m_bStaticBackground );
	DO_COPY( m_bRandomBGOnly );
	DO_COPY( m_bSaveScore );
	DO_COPY( m_bSaveReplay );
#undef APPROACH
#undef DO_COPY
}

static void AddPart( vector<std::string> &AddTo, float level, RString name )
{
	if( level == 0 )
	{
		return;
	}
	const RString LevelStr = (level == 1)? RString(""): fmt::sprintf( "%ld%% ", std::lrint(level*100) );

	AddTo.push_back( LevelStr + name );
}

void SongOptions::GetMods( vector<std::string> &AddTo ) const
{
	if( m_fMusicRate != 1 )
	{
		RString s = fmt::sprintf( "%2.2f", m_fMusicRate );
		if( s[s.size()-1] == '0' )
		{
			s.erase( s.size()-1 );
		}
		AddTo.push_back( s + "xMusic" );
	}

	AddPart( AddTo, m_fHaste,	"Haste" );

	switch( m_AutosyncType )
	{
	case AutosyncType_Off:	                                	break;
	case AutosyncType_Song:	AddTo.push_back("AutosyncSong");	break;
	case AutosyncType_Machine:	AddTo.push_back("AutosyncMachine");	break;
	case AutosyncType_Tempo:	AddTo.push_back("AutosyncTempo");	break;
	default:
		FAIL_M(fmt::sprintf("Invalid autosync type: %i", m_AutosyncType));
	}

	switch( m_SoundEffectType )
	{
	case SoundEffectType_Off:	                                	break;
	case SoundEffectType_Speed:	AddTo.push_back("EffectSpeed");		break;
	case SoundEffectType_Pitch:	AddTo.push_back("EffectPitch");		break;
	default:
		FAIL_M(fmt::sprintf("Invalid sound effect type: %i", m_SoundEffectType));
	}

	if( m_bAssistClap )
	{	AddTo.push_back( "Clap" );
	}
	if( m_bAssistMetronome )
	{
		AddTo.push_back( "Metronome" );
	}
	if( m_bStaticBackground )
	{
		AddTo.push_back( "StaticBG" );
	}
	if( m_bRandomBGOnly )
	{
		AddTo.push_back( "RandomBG" );
	}
}

void SongOptions::GetLocalizedMods( vector<std::string> &v ) const
{
	GetMods( v );
	for (auto &s: v)
	{
		s = CommonMetrics::LocalizeOptionItem( s, true );
	}
}

std::string SongOptions::GetString() const
{
	vector<std::string> v;
	GetMods( v );
	return Rage::join( ", ", v );
}

std::string SongOptions::GetLocalizedString() const
{
	vector<std::string> v;
	GetLocalizedMods( v );
	return Rage::join( ", ", v );
}


/* Options are added to the current settings; call Init() beforehand if
 * you don't want this. */
void SongOptions::FromString( const RString &sMultipleMods )
{
	RString sTemp = sMultipleMods;
	vector<RString> vs;
	split( sTemp, ",", vs, true );
	RString sThrowAway;
	for (auto &s: vs)
	{
		FromOneModString( s, sThrowAway );
	}
}

bool SongOptions::FromOneModString( const RString &sOneMod, RString &sErrorOut )
{
	RString sBit = Rage::trim(Rage::make_lower(sOneMod));

	Regex mult("^([0-9]+(\\.[0-9]+)?)xmusic$");
	vector<RString> matches;
	if( mult.Compare(sBit, matches) )
	{
		m_fMusicRate = StringToFloat( matches[0] );
		return true;
	}

	matches.clear();

	vector<RString> asParts;
	split( sBit, " ", asParts, true );
	bool on = true;
	if( asParts.size() > 1 )
	{
		sBit = asParts[1];
		if( asParts[0] == "no" )
			on = false;
	}

	if( sBit == "clap" )				m_bAssistClap = on;
	else if( sBit == "metronome" )				m_bAssistMetronome = on;
	else if( sBit == "autosync" || sBit == "autosyncsong" )	m_AutosyncType = on ? AutosyncType_Song : AutosyncType_Off;
	else if( sBit == "autosyncmachine" )			m_AutosyncType = on ? AutosyncType_Machine : AutosyncType_Off;
	else if( sBit == "autosynctempo" )			m_AutosyncType = on ? AutosyncType_Tempo : AutosyncType_Off;
	else if( sBit == "effect" && !on )			m_SoundEffectType = SoundEffectType_Off;
	else if( sBit == "effectspeed" )			m_SoundEffectType = on ? SoundEffectType_Speed : SoundEffectType_Off;
	else if( sBit == "effectpitch" )			m_SoundEffectType = on ? SoundEffectType_Pitch : SoundEffectType_Off;
	else if( sBit == "staticbg" )				m_bStaticBackground = on;
	else if( sBit == "randombg" )				m_bRandomBGOnly = on;
	else if( sBit == "savescore" )				m_bSaveScore = on;
	else if( sBit == "savereplay" )			m_bSaveReplay = on;
	else if( sBit == "haste" )				m_fHaste = on? 1.0f:0.0f;
	else
		return false;

	return true;
}

bool SongOptions::operator==( const SongOptions &other ) const
{
#define COMPARE(x) { if( x != other.x ) return false; }
	COMPARE( m_fMusicRate );
	COMPARE( m_fHaste );
	COMPARE( m_bAssistClap );
	COMPARE( m_bAssistMetronome );
	COMPARE( m_AutosyncType );
	COMPARE( m_SoundEffectType );
	COMPARE( m_bStaticBackground );
	COMPARE( m_bRandomBGOnly );
	COMPARE( m_bSaveScore );
	COMPARE( m_bSaveReplay );
#undef COMPARE
	return true;
}

// lua start
#include "LuaBinding.h"
#include "OptionsBinding.h"

/** @brief Allow Lua to have access to SongOptions. */
class LunaSongOptions: public Luna<SongOptions>
{
public:

	ENUM_INTERFACE(AutosyncSetting, AutosyncType, AutosyncType);
	//ENUM_INTERFACE(SoundEffectSetting, SoundEffectType, SoundEffectType);
	// Broken, SoundEffectType_Speed disables rate mod, other settings have no effect. -Kyz
	BOOL_INTERFACE(AssistClap, AssistClap);
	BOOL_INTERFACE(AssistMetronome, AssistMetronome);
	BOOL_INTERFACE(StaticBackground, StaticBackground);
	BOOL_INTERFACE(RandomBGOnly, RandomBGOnly);
	BOOL_INTERFACE(SaveScore, SaveScore);
	BOOL_INTERFACE(SaveReplay, SaveReplay);
	FLOAT_INTERFACE(MusicRate, MusicRate, (v > 0.0f && v <= 3.0f)); // Greater than 3 seems to crash frequently, haven't investigated why. -Kyz
	FLOAT_INTERFACE(Haste, Haste, (v >= -1.0f && v <= 1.0f));

	LunaSongOptions()
	{
		ADD_METHOD(AutosyncSetting);
		//ADD_METHOD(SoundEffectSetting);
		ADD_METHOD(AssistClap);
		ADD_METHOD(AssistMetronome);
		ADD_METHOD(StaticBackground);
		ADD_METHOD(RandomBGOnly);
		ADD_METHOD(SaveScore);
		ADD_METHOD(SaveReplay);
		ADD_METHOD(MusicRate);
		ADD_METHOD(Haste);
	}
};

LUA_REGISTER_CLASS( SongOptions )
// lua end

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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

#ifndef __stepmania__LunaGameState__
#define __stepmania__LunaGameState__

#include "GameState.h"
#include "LunaSteps.h"

#include "PlayerState.h"
#include "Song.h"
#include "Style.h"
#include "Course.h"
#include "MemoryCardManager.h"
#include "Profile.h"
#include "Game.h"
#include "NoteSkinManager.h"
#include "CharacterManager.h"
#include "Character.h"

/** @brief Allow Lua to have access to the GameState. */
class LunaGameState: public Luna<GameState>
{
public:
	DEFINE_METHOD( IsPlayerEnabled,			IsPlayerEnabled(Enum::Check<PlayerNumber>(L, 1)) )
	DEFINE_METHOD( IsHumanPlayer,			IsHumanPlayer(Enum::Check<PlayerNumber>(L, 1)) )
	DEFINE_METHOD( GetPlayerDisplayName,		GetPlayerDisplayName(Enum::Check<PlayerNumber>(L, 1)) )
	DEFINE_METHOD( GetMasterPlayerNumber,		GetMasterPlayerNumber() )
	DEFINE_METHOD( GetMultiplayer,			m_bMultiplayer )
	static int SetMultiplayer( T* p, lua_State *L )
	{
		p->m_bMultiplayer = BArg(1);
		return 0;
	}
	DEFINE_METHOD( InStepEditor,			m_bInStepEditor );
	DEFINE_METHOD( GetNumMultiplayerNoteFields,	m_iNumMultiplayerNoteFields )
	DEFINE_METHOD( ShowW1,				ShowW1() )
    
	static int SetNumMultiplayerNoteFields( T* p, lua_State *L )
	{
		p->m_iNumMultiplayerNoteFields = IArg(1);
		return 0;
	}
	static int GetPlayerState( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		p->m_pPlayerState[pn]->PushSelf(L);
		return 1;
	}
	static int GetMultiPlayerState( T* p, lua_State *L )
	{
		MultiPlayer mp = Enum::Check<MultiPlayer>(L, 1);
		p->m_pMultiPlayerState[mp]->PushSelf(L);
		return 1;
	}
	static int ApplyGameCommand( T* p, lua_State *L )
	{
		PlayerNumber pn = PLAYER_INVALID;
		if( lua_gettop(L) >= 2 && !lua_isnil(L,2) ) {
			// Legacy behavior: if an old-style numerical argument
			// is given, decrement it before trying to parse
			if( lua_isnumber(L,2) ) {
				int arg = (int) lua_tonumber( L, 2 );
				arg--;
				LuaHelpers::Push( L, arg );
				lua_replace( L, -2 );
			}
			pn = Enum::Check<PlayerNumber>(L, 2);
		}
		p->ApplyGameCommand(SArg(1),pn);
		return 0;
	}
	static int GetCurrentSong( T* p, lua_State *L )			{ if(p->m_pCurSong) p->m_pCurSong->PushSelf(L); else lua_pushnil(L); return 1; }
	static int SetCurrentSong( T* p, lua_State *L )
	{
		if( lua_isnil(L,1) ) { p->m_pCurSong.Set( NULL ); }
		else { Song *pS = Luna<Song>::check( L, 1, true ); p->m_pCurSong.Set( pS ); }
		return 0;
	}
	static int GetCurrentSteps( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		Steps *pSteps = p->m_pCurSteps[pn];
		if( pSteps ) {
			LunaSteps::PushSelf(L, pSteps);
		}
		else {
			lua_pushnil(L);
		}
		return 1;
	}
	static int SetCurrentSteps( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		if( lua_isnil(L,2) )	{ p->m_pCurSteps[pn].Set( NULL ); }
		else					{ Steps *pS = Luna<Steps>::check(L,2); p->m_pCurSteps[pn].Set( pS ); }
		ASSERT(p->m_pCurSteps[pn]->m_StepsType == p->m_pCurStyle->m_StepsType);
        
		// Why Broadcast again?  This is double-broadcasting. -Chris
		MESSAGEMAN->Broadcast( (MessageID)(Message_CurrentStepsP1Changed+pn) );
		return 0;
	}
	static int GetCurrentCourse( T* p, lua_State *L )		{ if(p->m_pCurCourse) p->m_pCurCourse->PushSelf(L); else lua_pushnil(L); return 1; }
	static int SetCurrentCourse( T* p, lua_State *L )
	{
		if( lua_isnil(L,1) ) { p->m_pCurCourse.Set( NULL ); }
		else { Course *pC = Luna<Course>::check(L,1); p->m_pCurCourse.Set( pC ); }
		return 0;
	}
	static int GetCurrentTrail( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		Trail *pTrail = p->m_pCurTrail[pn];
		if( pTrail ) { pTrail->PushSelf(L); }
		else		 { lua_pushnil(L); }
		return 1;
	}
	static int SetCurrentTrail( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		if( lua_isnil(L,2) )	{ p->m_pCurTrail[pn].Set( NULL ); }
		else					{ Trail *pS = Luna<Trail>::check(L,2); p->m_pCurTrail[pn].Set( pS ); }
		MESSAGEMAN->Broadcast( (MessageID)(Message_CurrentTrailP1Changed+pn) );
		return 0;
	}
	static int GetPreferredSong( T* p, lua_State *L )		{ if(p->m_pPreferredSong) p->m_pPreferredSong->PushSelf(L); else lua_pushnil(L); return 1; }
	static int SetPreferredSong( T* p, lua_State *L )
	{
		if( lua_isnil(L,1) ) { p->m_pPreferredSong = NULL; }
		else { Song *pS = Luna<Song>::check(L,1); p->m_pPreferredSong = pS; }
		return 0;
	}
	static int SetTemporaryEventMode( T* p, lua_State *L )	{ p->m_bTemporaryEventMode = BArg(1); return 0; }
	static int Env( T* p, lua_State *L )	{ p->m_Environment->PushSelf(L); return 1; }
	static int GetEditSourceSteps( T* p, lua_State *L )
	{
		Steps *pSteps = p->m_pEditSourceSteps;
		if( pSteps ) {
			LunaSteps::PushSelf(L, pSteps);
		}
		else {
			lua_pushnil(L);
		}
		return 1;
	}
	static int SetPreferredDifficulty( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>( L, 1 );
		Difficulty dc = Enum::Check<Difficulty>( L, 2 );
		p->m_PreferredDifficulty[pn].Set( dc );
		return 0;
	}
	DEFINE_METHOD( GetPreferredDifficulty,		m_PreferredDifficulty[Enum::Check<PlayerNumber>(L, 1)] )
	DEFINE_METHOD( AnyPlayerHasRankingFeats,	AnyPlayerHasRankingFeats() )
	DEFINE_METHOD( IsCourseMode,			IsCourseMode() )
	DEFINE_METHOD( IsBattleMode,			IsBattleMode() )
	DEFINE_METHOD( IsDemonstration,			m_bDemonstrationOrJukebox )
	DEFINE_METHOD( GetPlayMode,			m_PlayMode )
	DEFINE_METHOD( GetSortOrder,			m_SortOrder )
	DEFINE_METHOD( GetCurrentStageIndex,		m_iCurrentStageIndex )
	DEFINE_METHOD( IsGoalComplete,			IsGoalComplete(Enum::Check<PlayerNumber>(L, 1)) )
	DEFINE_METHOD( PlayerIsUsingModifier,		PlayerIsUsingModifier(Enum::Check<PlayerNumber>(L, 1), SArg(2)) )
	DEFINE_METHOD( GetCourseSongIndex,		GetCourseSongIndex() )
	DEFINE_METHOD( GetLoadingCourseSongIndex,	GetLoadingCourseSongIndex() )
	DEFINE_METHOD( GetSmallestNumStagesLeftForAnyHumanPlayer, GetSmallestNumStagesLeftForAnyHumanPlayer() )
	DEFINE_METHOD( IsAnExtraStage,			IsAnExtraStage() )
	DEFINE_METHOD( IsExtraStage,			IsExtraStage() )
	DEFINE_METHOD( IsExtraStage2,			IsExtraStage2() )
	DEFINE_METHOD( GetCurrentStage,			GetCurrentStage() )
	DEFINE_METHOD( HasEarnedExtraStage,		HasEarnedExtraStage() )
	DEFINE_METHOD( GetEarnedExtraStage,		GetEarnedExtraStage() )
	DEFINE_METHOD( GetEasiestStepsDifficulty,	GetEasiestStepsDifficulty() )
	DEFINE_METHOD( GetHardestStepsDifficulty,	GetHardestStepsDifficulty() )
	DEFINE_METHOD( IsEventMode,			IsEventMode() )
	DEFINE_METHOD( GetNumPlayersEnabled,		GetNumPlayersEnabled() )
	/*DEFINE_METHOD( GetSongBeat,			m_Position.m_fSongBeat )
     DEFINE_METHOD( GetSongBeatVisible,		m_Position.m_fSongBeatVisible )
     DEFINE_METHOD( GetSongBPS,			m_Position.m_fCurBPS )
     DEFINE_METHOD( GetSongFreeze,			m_Position.m_bFreeze )
     DEFINE_METHOD( GetSongDelay,			m_Position.m_bDelay )*/
	static int GetSongPosition( T* p, lua_State *L )
	{
		p->m_Position.PushSelf(L);
		return 1;
	}
	DEFINE_METHOD( GetGameplayLeadIn,		m_bGameplayLeadIn )
	DEFINE_METHOD( GetCoins,			m_iCoins )
	DEFINE_METHOD( IsSideJoined,			m_bSideIsJoined[Enum::Check<PlayerNumber>(L, 1)] )
	DEFINE_METHOD( GetCoinsNeededToJoin,		GetCoinsNeededToJoin() )
	DEFINE_METHOD( EnoughCreditsToJoin,		EnoughCreditsToJoin() )
	DEFINE_METHOD( PlayersCanJoin,			PlayersCanJoin() )
	DEFINE_METHOD( GetNumSidesJoined,		GetNumSidesJoined() )
	DEFINE_METHOD( GetCoinMode,			GetCoinMode() )
	DEFINE_METHOD( GetPremium,			GetPremium() )
	DEFINE_METHOD( GetSongOptionsString,		m_SongOptions.GetCurrent().GetString() )
	static int GetSongOptions( T* p, lua_State *L )
	{
		ModsLevel m = Enum::Check<ModsLevel>( L, 1 );
		RString s = p->m_SongOptions.Get(m).GetString();
		LuaHelpers::Push( L, s );
		return 1;
	}
	static int GetDefaultSongOptions( T* p, lua_State *L )
	{
		SongOptions so;
		p->GetDefaultSongOptions( so );
		lua_pushstring(L, so.GetString());
		return 1;
	}
	static int ApplyStageModifiers( T* p, lua_State *L )
	{
		p->ApplyStageModifiers( Enum::Check<PlayerNumber>(L, 1), SArg(2) );
		return 0;
	}
	static int ApplyPreferredModifiers( T* p, lua_State *L )
	{
		p->ApplyPreferredModifiers( Enum::Check<PlayerNumber>(L, 1), SArg(2) );
		return 0;
	}
	static int ClearStageModifiersIllegalForCourse( T* p, lua_State *L )
	{
		p->ClearStageModifiersIllegalForCourse();
		return 0;
	}
	static int SetSongOptions( T* p, lua_State *L )
	{
		ModsLevel m = Enum::Check<ModsLevel>( L, 1 );
        
		SongOptions so;
        
		so.FromString( SArg(2) );
		p->m_SongOptions.Assign( m, so );
		return 0;
	}
	static int GetStageResult( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		LuaHelpers::Push( L, p->GetStageResult(pn) );
		return 1;
	}
	static int IsWinner( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		lua_pushboolean(L, p->GetStageResult(pn)==RESULT_WIN); return 1;
	}
	static int IsDraw( T* p, lua_State *L )
	{
		lua_pushboolean(L, p->GetStageResult(PLAYER_1)==RESULT_DRAW); return 1;
	}
	static int GetCurrentGame( T* p, lua_State *L )			{ const_cast<Game*>(p->GetCurrentGame())->PushSelf( L ); return 1; }
	//static int SetCurrentGame( T* p, lua_State *L )			{ p->SetCurrentGame( GAMEMAN->StringToGame( SArg(1) ) ); return 0; }
	DEFINE_METHOD( GetEditCourseEntryIndex,		m_iEditCourseEntryIndex )
	DEFINE_METHOD( GetEditLocalProfileID,		m_sEditLocalProfileID.Get() )
	static int GetEditLocalProfile( T* p, lua_State *L )
	{
		Profile *pProfile = p->GetEditLocalProfile();
		if( pProfile )
			pProfile->PushSelf(L);
		else
			lua_pushnil( L );
		return 1;
	}
    
	static int GetCurrentStepsCredits( T* t, lua_State *L )
	{
		const Song* pSong = t->m_pCurSong;
		if( pSong == NULL )
			return 0;
        
		// use a vector and not a set so that ordering is maintained
		vector<const Steps*> vpStepsToShow;
		FOREACH_HumanPlayer( p )
		{
			const Steps* pSteps = GAMESTATE->m_pCurSteps[p];
			if( pSteps == NULL )
				return 0;
			bool bAlreadyAdded = find( vpStepsToShow.begin(), vpStepsToShow.end(), pSteps ) != vpStepsToShow.end();
			if( !bAlreadyAdded )
				vpStepsToShow.push_back( pSteps );
		}
        
		for( unsigned i=0; i<vpStepsToShow.size(); i++ )
		{
			const Steps* pSteps = vpStepsToShow[i];
			RString sDifficulty = CustomDifficultyToLocalizedString( GetCustomDifficulty( pSteps->m_StepsType, pSteps->GetDifficulty(), CourseType_Invalid ) );
            
			lua_pushstring( L, sDifficulty );
			lua_pushstring( L, pSteps->GetDescription() );
		}
        
		return vpStepsToShow.size()*2;
	}
    
	static int SetPreferredSongGroup( T* p, lua_State *L ) { p->m_sPreferredSongGroup.Set( SArg(1) ); return 0; }
	DEFINE_METHOD( GetPreferredSongGroup, m_sPreferredSongGroup.Get() );
	static int GetHumanPlayers( T* p, lua_State *L )
	{
		vector<PlayerNumber> vHP;
		FOREACH_HumanPlayer( pn )
        vHP.push_back( pn );
		LuaHelpers::CreateTableFromArray( vHP, L );
		return 1;
	}
    static int GetEnabledPlayers(T* , lua_State *L )
    {
        vector<PlayerNumber> vEP;
        FOREACH_EnabledPlayer( pn )
        vEP.push_back( pn );
        LuaHelpers::CreateTableFromArray( vEP, L );
        return 1;
    }
	static int GetCurrentStyle( T* p, lua_State *L )
	{
		Style *pStyle = const_cast<Style *> (p->GetCurrentStyle());
		LuaHelpers::Push( L, pStyle );
		return 1;
	}
	static int IsAnyHumanPlayerUsingMemoryCard( T* , lua_State *L )
	{
		bool bUsingMemoryCard = false;
		FOREACH_HumanPlayer( pn )
		{
			if( MEMCARDMAN->GetCardState(pn) == MemoryCardState_Ready )
				bUsingMemoryCard = true;
		}
		lua_pushboolean(L, bUsingMemoryCard );
		return 1;
	}
	static int GetNumStagesForCurrentSongAndStepsOrCourse( T* , lua_State *L )
	{
		lua_pushnumber(L, GAMESTATE->GetNumStagesForCurrentSongAndStepsOrCourse() );
		return 1;
	}
	static int GetNumStagesLeft( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		lua_pushnumber(L, p->GetNumStagesLeft(pn));
		return 1;
	}
	static int GetGameSeed( T* p, lua_State *L )			{ LuaHelpers::Push( L, p->m_iGameSeed ); return 1; }
	static int GetStageSeed( T* p, lua_State *L )			{ LuaHelpers::Push( L, p->m_iStageSeed ); return 1; }
	static int SaveLocalData( T* p, lua_State *L )			{ p->SaveLocalData(); return 0; }
    
	static int SetJukeboxUsesModifiers( T* p, lua_State *L )
	{
		p->m_bJukeboxUsesModifiers = BArg(1); return 0;
	}
	static int Reset( T* p, lua_State *L )				{ p->Reset(); return 0; }
	static int JoinPlayer( T* p, lua_State *L )				{ p->JoinPlayer(Enum::Check<PlayerNumber>(L, 1)); return 0; }
	static int UnjoinPlayer( T* p, lua_State *L )				{ p->UnjoinPlayer(Enum::Check<PlayerNumber>(L, 1)); return 0; }
	static int JoinInput( T* p, lua_State *L )
	{
		lua_pushboolean(L, p->JoinInput(Enum::Check<PlayerNumber>(L, 1)));
		return 1;
	}
	static int GetSongPercent( T* p, lua_State *L )				{ lua_pushnumber(L, p->GetSongPercent(FArg(1))); return 1; }
	DEFINE_METHOD( GetCurMusicSeconds,	m_Position.m_fMusicSeconds )
    
	DEFINE_METHOD( GetWorkoutGoalComplete,		m_bWorkoutGoalComplete )
	static int GetCharacter( T* p, lua_State *L )				{ p->m_pCurCharacters[Enum::Check<PlayerNumber>(L, 1)]->PushSelf(L); return 1; }
	static int SetCharacter( T* p, lua_State *L ){
		Character* c = CHARMAN->GetCharacterFromID(SArg(2));
		if (c)
			p->m_pCurCharacters[Enum::Check<PlayerNumber>(L, 1)] = c;
		return 0;
	}
	static int GetExpandedSectionName( T* p, lua_State *L )				{ lua_pushstring(L, p->sExpandedSectionName); return 1; }
	static int AddStageToPlayer( T* p, lua_State *L )				{ p->AddStageToPlayer(Enum::Check<PlayerNumber>(L, 1)); return 0; }
	static int CurrentOptionsDisqualifyPlayer( T* p, lua_State *L )	{ lua_pushboolean(L, p->CurrentOptionsDisqualifyPlayer(Enum::Check<PlayerNumber>(L, 1))); return 1; }
	
	static int ResetPlayerOptions( T* p, lua_State *L )
	{
		p->ResetPlayerOptions(Enum::Check<PlayerNumber>(L, 1));
		return 0;
	}
	
	static int RefreshNoteSkinData( T* p, lua_State *L )
	{
		NOTESKIN->RefreshNoteSkinData(p->m_pCurGame);
		return 0;
	}
	
	static int Dopefish( T* p, lua_State *L )
	{
		lua_pushboolean(L, p->m_bDopefish);
		return 1;
	}
    
	LunaGameState()
	{
		ADD_METHOD( IsPlayerEnabled );
		ADD_METHOD( IsHumanPlayer );
		ADD_METHOD( GetPlayerDisplayName );
		ADD_METHOD( GetMasterPlayerNumber );
		ADD_METHOD( GetMultiplayer );
		ADD_METHOD( SetMultiplayer );
		ADD_METHOD( InStepEditor );
		ADD_METHOD( GetNumMultiplayerNoteFields );
		ADD_METHOD( SetNumMultiplayerNoteFields );
		ADD_METHOD( ShowW1 );
		ADD_METHOD( GetPlayerState );
		ADD_METHOD( GetMultiPlayerState );
		ADD_METHOD( ApplyGameCommand );
		ADD_METHOD( GetCurrentSong );
		ADD_METHOD( SetCurrentSong );
		ADD_METHOD( GetCurrentSteps );
		ADD_METHOD( SetCurrentSteps );
		ADD_METHOD( GetCurrentCourse );
		ADD_METHOD( SetCurrentCourse );
		ADD_METHOD( GetCurrentTrail );
		ADD_METHOD( SetCurrentTrail );
		ADD_METHOD( SetPreferredSong );
		ADD_METHOD( GetPreferredSong );
		ADD_METHOD( SetTemporaryEventMode );
		ADD_METHOD( Env );
		ADD_METHOD( GetEditSourceSteps );
		ADD_METHOD( SetPreferredDifficulty );
		ADD_METHOD( GetPreferredDifficulty );
		ADD_METHOD( AnyPlayerHasRankingFeats );
		ADD_METHOD( IsCourseMode );
		ADD_METHOD( IsBattleMode );
		ADD_METHOD( IsDemonstration );
		ADD_METHOD( GetPlayMode );
		ADD_METHOD( GetSortOrder );
		ADD_METHOD( GetCurrentStageIndex );
		ADD_METHOD( IsGoalComplete );
		ADD_METHOD( PlayerIsUsingModifier );
		ADD_METHOD( GetCourseSongIndex );
		ADD_METHOD( GetLoadingCourseSongIndex );
		ADD_METHOD( GetSmallestNumStagesLeftForAnyHumanPlayer );
		ADD_METHOD( IsAnExtraStage );
		ADD_METHOD( IsExtraStage );
		ADD_METHOD( IsExtraStage2 );
		ADD_METHOD( GetCurrentStage );
		ADD_METHOD( HasEarnedExtraStage );
		ADD_METHOD( GetEarnedExtraStage );
		ADD_METHOD( GetEasiestStepsDifficulty );
		ADD_METHOD( GetHardestStepsDifficulty );
		ADD_METHOD( IsEventMode );
		ADD_METHOD( GetNumPlayersEnabled );
		/*ADD_METHOD( GetSongBeat );
         ADD_METHOD( GetSongBeatVisible );
         ADD_METHOD( GetSongBPS );
         ADD_METHOD( GetSongFreeze );
         ADD_METHOD( GetSongDelay );*/
		ADD_METHOD( GetSongPosition );
		ADD_METHOD( GetGameplayLeadIn );
		ADD_METHOD( GetCoins );
		ADD_METHOD( IsSideJoined );
		ADD_METHOD( GetCoinsNeededToJoin );
		ADD_METHOD( EnoughCreditsToJoin );
		ADD_METHOD( PlayersCanJoin );
		ADD_METHOD( GetNumSidesJoined );
		ADD_METHOD( GetCoinMode );
		ADD_METHOD( GetPremium );
		ADD_METHOD( GetSongOptionsString );
		ADD_METHOD( GetSongOptions );
		ADD_METHOD( GetDefaultSongOptions );
		ADD_METHOD( ApplyPreferredModifiers );
		ADD_METHOD( ApplyStageModifiers );
		ADD_METHOD( ClearStageModifiersIllegalForCourse );
		ADD_METHOD( SetSongOptions );
		ADD_METHOD( GetStageResult );
		ADD_METHOD( IsWinner );
		ADD_METHOD( IsDraw );
		ADD_METHOD( GetCurrentGame );
		//ADD_METHOD( SetCurrentGame );
		ADD_METHOD( GetEditCourseEntryIndex );
		ADD_METHOD( GetEditLocalProfileID );
		ADD_METHOD( GetEditLocalProfile );
		ADD_METHOD( GetCurrentStepsCredits );
		ADD_METHOD( SetPreferredSongGroup );
		ADD_METHOD( GetPreferredSongGroup );
		ADD_METHOD( GetHumanPlayers );
		ADD_METHOD( GetEnabledPlayers );
		ADD_METHOD( GetCurrentStyle );
		ADD_METHOD( IsAnyHumanPlayerUsingMemoryCard );
		ADD_METHOD( GetNumStagesForCurrentSongAndStepsOrCourse );
		ADD_METHOD( GetNumStagesLeft );
		ADD_METHOD( GetGameSeed );
		ADD_METHOD( GetStageSeed );
		ADD_METHOD( SaveLocalData );
		ADD_METHOD( SetJukeboxUsesModifiers );
		ADD_METHOD( GetWorkoutGoalComplete );
		ADD_METHOD( Reset );
		ADD_METHOD( JoinPlayer );
		ADD_METHOD( UnjoinPlayer );
		ADD_METHOD( JoinInput );
		ADD_METHOD( GetSongPercent );
		ADD_METHOD( GetCurMusicSeconds );
		ADD_METHOD( GetCharacter );
		ADD_METHOD( SetCharacter );
		ADD_METHOD( GetExpandedSectionName );
		ADD_METHOD( AddStageToPlayer );
		ADD_METHOD( CurrentOptionsDisqualifyPlayer );
		ADD_METHOD( ResetPlayerOptions );
		ADD_METHOD( RefreshNoteSkinData );
		ADD_METHOD( Dopefish );
	}
    
    LUNA_FILE_ALLOW_GLOBAL(LunaGameState, GAMESTATE, GAMESTATE);
    
    LUNA_FILE_ADD_PUSH(GameState);
};


#endif /* defined(__stepmania__LunaGameState__) */

/*
 * (c) 2001-2014 Chris Danford, Glenn Maynard, Chris Gomez
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

/**
 * @file cl_game.h
 * @brief Shared game type headers
 */

/*
Copyright (C) 2002-2007 UFO: Alien Invasion team.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef CL_GAME_H
#define CL_GAME_H

#define GAME_NONE			0
#define GAME_SINGLEPLAYER	(1 << 0)
#define GAME_MULTIPLAYER	(1 << 1)
#define GAME_CAMPAIGN		(GAME_SINGLEPLAYER | (1 << 2))
#define GAME_SKIRMISH		(GAME_SINGLEPLAYER | (1 << 3))

#define GAME_MAX			GAME_SKIRMISH

#define GAME_IsSingleplayer()	(cls.gametype & GAME_SINGLEPLAYER)
#define GAME_IsMultiplayer()	(cls.gametype == GAME_MULTIPLAYER)
#define GAME_IsSkirmish()		(cls.gametype == GAME_SKIRMISH)
#define GAME_IsCampaign()		(cls.gametype == GAME_CAMPAIGN)

void GAME_InitStartup(void);
void GAME_SetMode(int gametype);
void GAME_RestartMode(int gametype);
void GAME_Init(qboolean load);
qboolean GAME_ItemIsUseable(const objDef_t *od);
void GAME_HandleResults(struct dbuffer *msg, int winner, int *numSpawned, int *numAlive, int numKilled[][MAX_TEAMS], int numStunned[][MAX_TEAMS]);
void GAME_SpawnSoldiers(void);
int GAME_GetCurrentTeam(void);
equipDef_t *GAME_GetEquipmentDefinition(void);
void GAME_CharacterCvars(const character_t *chr);

#include "cl_game_campaign.h"
#include "cl_game_skirmish.h"
#include "cl_game_multiplayer.h"

#endif

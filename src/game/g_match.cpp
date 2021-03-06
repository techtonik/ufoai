/**
 * @file
 * @brief Match related functions
 */

/*
All original material Copyright (C) 2002-2011 UFO: Alien Invasion.

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

#include "g_match.h"
#include "g_local.h"
#include "g_actor.h"
#include "g_ai.h"
#include "g_edicts.h"
#include "g_trigger.h"
#include "g_utils.h"
#include "g_vis.h"

/**
 * @brief Determines the amount of XP earned by a given soldier for a given skill, based on the soldier's performance in the last mission.
 * @param[in] skill The skill for which to fetch the maximum amount of XP.
 * @param[in] ent Pointer to the character you want to get the earned experience for
 * @sa G_UpdateCharacterExperience
 * @sa G_GetMaxExperiencePerMission
 */
static int G_GetEarnedExperience (abilityskills_t skill, edict_t *ent)
{
	character_t *chr = &ent->chr;

	int experience = 0;
	int i;

	switch (skill) {
	case ABILITY_POWER: {
		const float weight = chr->scoreMission->carriedWeight / level.actualRound;
		const float penalty = GET_ENCUMBRANCE_PENALTY(weight, chr->score.skills[ABILITY_POWER]);
		experience = 150 * (weight / chr->score.skills[ABILITY_POWER]) / penalty;
		break;
	}
	case ABILITY_SPEED:
		experience += chr->scoreMission->movedNormal / 2 + chr->scoreMission->movedCrouched;
		/* skip skills < ABILITY_NUM_TYPES, they are abilities not real skills */
		for (i = ABILITY_NUM_TYPES; i < SKILL_NUM_TYPES; i++)
			experience += (chr->scoreMission->firedTUs[i] + chr->scoreMission->firedSplashTUs[i]) / 10;
		break;
	case ABILITY_ACCURACY:
		/* skip skills < ABILITY_NUM_TYPES, they are abilities not real skills */
		for (i = ABILITY_NUM_TYPES; i < SKILL_NUM_TYPES; i++)
			if (i == SKILL_SNIPER)
				experience += 30 * (chr->scoreMission->hits[i][KILLED_ENEMIES] + chr->scoreMission->hitsSplash[i][KILLED_ENEMIES]);
			else
				experience += 20 * (chr->scoreMission->hits[i][KILLED_ENEMIES] + chr->scoreMission->hitsSplash[i][KILLED_ENEMIES]);
		break;
	case ABILITY_MIND:
		experience = 50 + 200 * chr->scoreMission->kills[KILLED_ENEMIES];
		break;
	case SKILL_CLOSE:
		experience = 150 * (chr->scoreMission->hits[skill][KILLED_ENEMIES] + chr->scoreMission->hitsSplash[skill][KILLED_ENEMIES]);
		break;
	case SKILL_HEAVY:
		experience = 200 * (chr->scoreMission->hits[skill][KILLED_ENEMIES] + chr->scoreMission->hitsSplash[skill][KILLED_ENEMIES]);
		break;
	case SKILL_ASSAULT:
		experience = 100 * (chr->scoreMission->hits[skill][KILLED_ENEMIES] + chr->scoreMission->hitsSplash[skill][KILLED_ENEMIES]);
		break;
	case SKILL_SNIPER:
		experience = 200 * (chr->scoreMission->hits[skill][KILLED_ENEMIES] + chr->scoreMission->hitsSplash[skill][KILLED_ENEMIES]);
		break;
	case SKILL_EXPLOSIVE:
		experience = 200 * (chr->scoreMission->hits[skill][KILLED_ENEMIES] + chr->scoreMission->hitsSplash[skill][KILLED_ENEMIES]);
		break;
	default:
		break;
	}

	return experience;
}

/**
 * @brief Determines the maximum amount of XP per skill that can be gained from any one mission.
 * @param[in] skill The skill for which to fetch the maximum amount of XP.
 * @sa G_UpdateCharacterExperience
 * @sa G_GetEarnedExperience
 * @note Explanation of the values here:
 * There is a maximum speed at which skills may rise over the course of 100 missions (the predicted career length of a veteran soldier).
 * For example, POWER will, at best, rise 20 points over 100 missions. If the soldier gets max XP every time.
 * Because the increase is given as experience^0.6, that means that the maximum XP cap x per mission is given as
 * log 20 / log x = 0.6
 * log x = log 20 / 0.6
 * x = 10 ^ (log 20 / 0.6)
 * x = 214
 * The division by 100 happens in G_UpdateCharacterExperience
 */
static int G_CharacterGetMaxExperiencePerMission (const abilityskills_t skill)
{
	switch (skill) {
	case ABILITY_POWER:
		return 375;
	case ABILITY_SPEED:
		return 91;
	case ABILITY_ACCURACY:
		return 290;
	case ABILITY_MIND:
		return 450;
	case SKILL_CLOSE:
		return 680;
	case SKILL_HEAVY:
		return 680;
	case SKILL_ASSAULT:
		return 680;
	case SKILL_SNIPER:
		return 680;
	case SKILL_EXPLOSIVE:
		return 680;
	case SKILL_NUM_TYPES: /* This is health. */
		return 2154;
	case SKILL_PILOTING:
	case SKILL_TARGETING:
	case SKILL_EVADING:
		return 0;
	default:
		gi.Error("G_GetMaxExperiencePerMission: invalid skill type\n");
	}
}

/**
 * @brief Updates character experience after a mission.
 * @param[in,out] ent Pointer to the character that should get the experience updated
 * @sa CP_UpdateCharacterStats
 * @sa G_UpdateCharacterScore
 * @sa G_UpdateHitScore
 */
static void G_UpdateCharacterExperience (edict_t *ent)
{
	character_t *chr = &ent->chr;

	int i;
	unsigned int maxXP, gainedXP, totalGainedXP;

	/* Robots/UGVs do not get skill-upgrades. */
	if (chr->teamDef->robot)
		return;

	totalGainedXP = 0;
	for (i = 0; i < SKILL_NUM_TYPES; i++) {
		const abilityskills_t skill = (abilityskills_t)i;
		maxXP = G_CharacterGetMaxExperiencePerMission(skill);
		gainedXP = G_GetEarnedExperience(skill, ent);

		gainedXP = std::min(gainedXP, maxXP);
		chr->score.experience[i] += gainedXP;
		totalGainedXP += gainedXP;
	}

	/* Health isn't part of abilityskills_t, so it needs to be handled separately. */
	assert(i == SKILL_NUM_TYPES);	/**< We need to get sure that index for health-experience is correct. */
	maxXP = G_CharacterGetMaxExperiencePerMission((abilityskills_t)i);
	gainedXP = std::min(maxXP, totalGainedXP / 2);
	chr->score.experience[i] += gainedXP;
}

/**
 * @brief Triggers the end of the game. Will be executed in the next server (or game) frame.
 * @param[in] team The winning team
 * @param[in] timeGap Second to wait before really ending the game. Useful if you want to allow a last view on the scene
 */
void G_MatchEndTrigger (int team, int timeGap)
{
	bool foundNextMap = false;
	edict_t *ent = NULL;

	while ((ent = G_EdictsGetTriggerNextMaps(ent)) != NULL) {
		if (ent->team == team) {
			ent->think = Think_NextMapTrigger;
			ent->nextthink = 1;
			foundNextMap = true;
		}
	}

	if (!foundNextMap) {
		const int realTimeGap = timeGap > 0 ? level.time + timeGap : 1;
		level.winningTeam = team;
		level.intermissionTime = realTimeGap;
	}
}

/**
 * @brief Sends character stats like assigned missions and kills back to client
 * @param[in] ent The edict to send the data for
 * @note first short is the ucn to allow the client to identify the character
 * @note parsed in GAME_CP_Results
 * @sa GAME_SendCurrentTeamSpawningInfo
 * @note you also have to update the pascal string size in G_MatchSendResults if you change the buffer here
 */
static void G_SendCharacterData (const edict_t* ent)
{
	int k;

	assert(ent);

	/* write character number */
	gi.WriteShort(ent->chr.ucn);

	gi.WriteShort(ent->HP);
	gi.WriteByte(ent->STUN);
	gi.WriteByte(ent->morale);

	for (k = 0; k < BODYPART_MAXTYPE; ++k)
		gi.WriteByte(ent->chr.wounds.woundLevel[k] + ent->chr.wounds.treatmentLevel[k]);

	/** Scores @sa inv_shared.h:chrScoreGlobal_t */
	for (k = 0; k < SKILL_NUM_TYPES + 1; k++)
		gi.WriteLong(ent->chr.score.experience[k]);
	for (k = 0; k < KILLED_NUM_TYPES; k++)
		gi.WriteShort(ent->chr.score.kills[k]);
	for (k = 0; k < KILLED_NUM_TYPES; k++)
		gi.WriteShort(ent->chr.score.stuns[k]);
	gi.WriteShort(ent->chr.score.assignedMissions);
}

/**
 * @brief Handles the end of a match
 * @param[in] team The winning team number
 * @param[in] nextmap Is there a follow-up map within the same match ?
 * @sa G_RunFrame
 * @sa CL_ParseResults
 */
static void G_MatchSendResults (int team, bool nextmap)
{
	edict_t *ent, *attacker;
	int i, j = 0;

	attacker = NULL;
	ent = NULL;
	/* Calculate new scores/skills for the soldiers. */
	while ((ent = G_EdictsGetNextLivingActor(ent))) {
		if (!G_IsAI(ent))
			G_UpdateCharacterExperience(ent);
		else if (ent->team == team)
			attacker = ent;
	}

	/* if aliens won, make sure every soldier that is not in the rescue zone dies */
	if (team == TEAM_ALIEN) {
		ent = NULL;
		while ((ent = G_EdictsGetNextLivingActor(ent)))
			if (ent->team != team && !G_ActorIsInRescueZone(ent)) {
				ent->HP = 0;
				G_ActorDieOrStun(ent, attacker);
			}
	}

	G_VisMakeEverythingVisible();

	/* send results */
	G_EventAdd(PM_ALL, EV_RESULTS, -1);
	gi.WriteByte(MAX_TEAMS);
	gi.WriteByte(team);
	gi.WriteByte(nextmap);

	for (i = 0; i < MAX_TEAMS; i++) {
		gi.WriteByte(level.num_spawned[i]);
		gi.WriteByte(level.num_alive[i]);
	}

	for (i = 0; i <= MAX_TEAMS; i++)
		for (j = 0; j < MAX_TEAMS; j++)
			gi.WriteByte(level.num_kills[i][j]);

	for (i = 0; i <= MAX_TEAMS; i++)
		for (j = 0; j < MAX_TEAMS; j++)
			gi.WriteByte(level.num_stuns[i][j]);

	/* how many actors */
	j = 0;
	ent = NULL;
	while ((ent = G_EdictsGetNextActor(ent)))
		if (!G_IsAI(ent))
			j++;

	/* number of soldiers */
	gi.WriteByte(j);

	if (j) {
		ent = NULL;
		while ((ent = G_EdictsGetNextActor(ent))) {
			if (!G_IsAI(ent)) {
				G_SendCharacterData(ent);
			}
		}
	}

	G_EventEnd();
}

/**
 * @brief Checks whether a match is over.
 * @return @c true if this match is over, @c false otherwise
 */
bool G_MatchDoEnd (void)
{
	/* check for intermission */
	if (level.intermissionTime > 0.0 && level.time > level.intermissionTime) {
		G_PrintStats("End of game - Team %i is the winner", level.winningTeam);
		G_MatchSendResults(level.winningTeam, level.nextMapSwitch);

		/* now we cleanup the AI */
		AIL_Cleanup();

		if (level.mapEndCommand != NULL) {
			gi.AddCommandString(level.mapEndCommand);
		}

		level.intermissionTime = 0.0;
		level.winningTeam = 0;
		return true;
	}

	return false;
}

/**
 * @brief Checks whether there are still actors to fight with left. If none
 * are the match end will be triggered.
 * @sa G_MatchEndTrigger
 */
void G_MatchEndCheck (void)
{
	int activeTeams;
	int i, last;

	if (level.intermissionTime > 0.0) /* already decided */
		return;

	if (!level.numplayers) {
		G_MatchEndTrigger(0, 0);
		return;
	}

	/** @todo count from 0 to get the civilians for objectives */
	for (i = 1, activeTeams = 0, last = 0; i < MAX_TEAMS; i++) {
		edict_t *ent = NULL;
		/* search for living but not stunned actors - there must at least be one actor
		 * that is still able to attack or defend himself */
		while ((ent = G_EdictsGetNextLivingActorOfTeam(ent, i)) != NULL) {
			if (!G_IsStunned(ent)) {
				last = i;
				activeTeams++;
				break;
			}
		}
	}

	/** @todo < 2 does not work when we count civilians */
	/* prepare for sending results */
	if (activeTeams < 2) {
		const int timeGap = (level.activeTeam == TEAM_ALIEN ? 10.0 : 3.0);
		G_MatchEndTrigger(activeTeams == 1 ? last : 0, timeGap);
	}
}

/**
 * @brief Checks whether the game is running (active team and no intermission time)
 * @return @c true if there is an active team for the current turn and the end of the game
 * was not yet triggered
 * @sa G_MatchEndTrigger
 */
bool G_MatchIsRunning (void)
{
	if (level.intermissionTime > 0.0)
		return false;
	return (level.activeTeam != TEAM_NO_ACTIVE);
}

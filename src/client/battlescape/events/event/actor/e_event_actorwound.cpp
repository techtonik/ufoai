/**
 * @file
 */

/*
Copyright (C) 2002-2012 UFO: Alien Invasion.

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

#include "../../../../client.h"
#include "../../../cl_actor.h"
#include "../../../cl_hud.h"
#include "e_event_actorwound.h"

/**
 * @brief Parses the actor wound stats that come from the netchannel
 * @sa CL_ParseEvent
 * @sa G_SendStats
 */
void CL_ActorWound (const eventRegister_t *self, dbuffer *msg)
{
	le_t *le;
	int entnum, bodyPart, wounds, treatment;

	NET_ReadFormat(msg, self->formatString, &entnum, &bodyPart, &wounds, &treatment);

	le = LE_Get(entnum);
	if (!le)
		LE_NotFoundError(entnum);

	switch (le->type) {
	case ET_ACTORHIDDEN:
	case ET_ACTOR:
	case ET_ACTOR2x2:
		break;
	default:
		Com_Printf("CL_ActorWound: LE (%i) not an actor (type: %i)\n", entnum, le->type);
		return;
	}

	if (le->wounds.woundLevel[bodyPart] < wounds && wounds > le->maxHP *
			le->teamDef->bodyTemplate->woundThreshold(bodyPart)) {
		const character_t *chr = CL_ActorGetChr(le);
		char tmpbuf[128];
		Com_sprintf(tmpbuf, lengthof(tmpbuf), _("%s has been wounded"), chr->name);
		HUD_DisplayMessage(tmpbuf);
	}
	le->wounds.woundLevel[bodyPart] = wounds;
	le->wounds.treatmentLevel[bodyPart] = treatment;
}

/**
 * @file g_actor.c
 */

/*
Copyright (C) 2002-2009 UFO: Alien Invasion.

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

#include "g_local.h"

/**
 * @brief Checks whether the given edict is a living actor
 * @param[in] ent The edict to perform the check for
 * @sa LE_IsLivingActor
 */
qboolean G_IsLivingActor (const edict_t *ent)
{
	return G_IsActor(ent) && !G_IsDead(ent);
}

/**
 * @brief Make the actor use (as in open/close) a door edict
 * @note Will also check whether the door is still reachable (this might have
 * changed due to the rotation) after the usage
 * @param actor The actor that is using the door
 * @param door The door that should be opened/closed
 * @todo Check other actors that might be close to the door, too
 */
void G_ActorUseDoor (edict_t *actor, edict_t *door)
{
	edict_t *closeActor = NULL;

	G_ClientUseEdict(G_PLAYER_FROM_ENT(actor), actor, door);

	while ((closeActor = G_FindRadius(closeActor, door->origin, UNIT_SIZE * 3, ET_ACTOR))) {
		/* check whether the door is still reachable (this might have
		 * changed due to the rotation) or whether a actor can reach it now */
		if (!G_TouchTriggers(closeActor))
			G_ActorSetClientAction(closeActor, NULL);
	}
}

/**
 * @brief Handles the client actions (interaction with the world)
 * @param actor The actors' edict
 * @param ent The edict that can be used by the actor
 */
void G_ActorSetClientAction (edict_t *actor, edict_t *ent)
{
	actor->clientAction = ent;
	if (ent == NULL) {
		G_EventResetClientAction(actor);
	} else {
		G_EventSetClientAction(actor);
	}
}

/**
 * @brief Calculates the amount of all currently reserved TUs
 * @param ent The actor to calculate the reserved TUs for
 * @return The amount of reserved TUs for the given actor edict
 */
int G_ActorGetReservedTUs (const edict_t *ent)
{
	const chrReservations_t *res = &ent->chr.reservedTus;
	return res->reaction + res->shot + res->crouch;
}

/**
 * @brief Calculates the amount of usable TUs. This is without the reserved TUs.
 * @param ent The actor to calculate the amount of usable TUs for
 * @return The amount of usable TUs for the given actor edict
 */
int G_ActorUsableTUs (const edict_t *ent)
{
	if (!ent)
		return 0;

	return ent->TU - G_ActorGetReservedTUs(ent);
}

/**
 * @brief Calculates the amount of TUs that are needed for the current selected reaction fire mode.
 * @note It's assumed that there is a sane fire mode selected for reaction fire
 * @param ent The actors edict
 * @return The amount of TUs that are needed for the current selected reaction fire mode.
 */
int G_ActorGetTUForReactionFire (const edict_t *ent)
{
	const invList_t *invlistWeapon;
	const chrFiremodeSettings_t *fm = &ent->chr.RFmode;
	const fireDef_t *fd;

	invlistWeapon = ACTOR_GET_INV(ent, fm->hand);
	assert(invlistWeapon);
	assert(invlistWeapon->item.t);

	fd = FIRESH_FiredefForWeapon(&invlistWeapon->item);
	assert(fd);
	return fd[fm->fmIdx].time;
}

void G_ActorReserveTUs (edict_t *ent, int resReaction, int resShot, int resCrouch)
{
	if (ent->TU >= resReaction + resShot + resCrouch) {
		ent->chr.reservedTus.reaction = resReaction;
		ent->chr.reservedTus.shot = resShot;
		ent->chr.reservedTus.crouch = resCrouch;
	}

	G_EventActorSendReservations(ent);
}

/**
 * @brief Searches an actor by a unique character number
 * @param[in] ucn The unique character number
 * @param[in] team The team to get the actor with the ucn from
 * @return The actor edict if found, otherwise @c NULL
 */
edict_t *G_GetActorByUCN (const int ucn, const int team)
{
	edict_t *ent = NULL;

	while ((ent = G_EdictsGetNextActor(ent)))
		if (team == ent->team && ent->chr.ucn == ucn)
			return ent;

	return NULL;
}

/**
 * @brief Turns an actor around
 * @param[in] ent the actor (edict) we are talking about
 * @param[in] dir the direction to turn the edict into, might be an action
 * @return Bitmask of visible (VIS_*) values
 * @sa G_CheckVisTeamAll
 */
int G_ActorDoTurn (edict_t * ent, byte dir)
{
	float angleDiv;
	const byte *rot;
	int i, num;
	int status;

	assert(ent->dir < CORE_DIRECTIONS);
	assert(dir < PATHFINDING_DIRECTIONS);

	/*
	 * If dir is at least CORE_DIRECTIONS but less than FLYING_DIRECTIONS,
	 * then the direction is an action.
	 */
	/** @todo If performing an action, ensure the actor is facing the direction
	 *  needed to perform the action if needed (climbing ladders).
	 */
	if (dir >= CORE_DIRECTIONS && dir < FLYING_DIRECTIONS)
		return 0;

	/* Clamp dir between 0 and CORE_DIRECTIONS - 1. */
	dir &= (CORE_DIRECTIONS - 1);
	assert(dir < CORE_DIRECTIONS);

	/* Return if no rotation needs to be done. */
	if (ent->dir == dir)
		return 0;

	/* calculate angle difference */
	angleDiv = directionAngles[dir] - directionAngles[ent->dir];
	if (angleDiv > 180.0)
		angleDiv -= 360.0;
	if (angleDiv < -180.0)
		angleDiv += 360.0;

	/* prepare rotation - decide whether the actor turns around the left
	 * shoulder or the right - this is needed the get the rotation vector
	 * that is used below to check in each of the rotation steps
	 * (1/8, 22.5 degree) whether something became visible while turning */
	if (angleDiv > 0) {
		const int angleStep = (360.0 / CORE_DIRECTIONS);
		rot = dvleft;
		num = (angleDiv + angleStep / 2) / angleStep;
	} else {
		const int angleStep = (360.0 / CORE_DIRECTIONS);
		rot = dvright;
		num = (-angleDiv + angleStep / 2) / angleStep;
	}

	/* do rotation and vis checks */
	status = 0;

	/* check every angle (1/8 steps - on the way to the end direction) in the rotation
	 * whether something becomes visible and stop before reaching the final direction
	 * if this happened */
	for (i = 0; i < num; i++) {
		ent->dir = rot[ent->dir];
		assert(ent->dir < CORE_DIRECTIONS);
		status |= G_CheckVisTeamAll(ent->team, qfalse, ent);
	}

	if (status & VIS_STOP) {
		/* send the turn */
		G_EventActorTurn(ent);
	}

	return status;
}

/**
 * @brief Sets correct bounding box for actor (state dependent).
 * @param[in] ent Pointer to entity for which bounding box is being set.
 * @note Also re-links the actor edict - because the server must know about the
 * changed bounding box for tracing to work.
 */
void G_ActorSetMaxs (edict_t* ent)
{
	if (G_IsCrouched(ent))
		VectorSet(ent->maxs, PLAYER_WIDTH, PLAYER_WIDTH, PLAYER_CROUCH);
	else
		VectorSet(ent->maxs, PLAYER_WIDTH, PLAYER_WIDTH, PLAYER_STAND);

	/* Link it. */
	gi.LinkEdict(ent);
}

/**
 * @brief Reports and handles death or stun of an actor.
 * @param[in] ent Pointer to an entity being killed or stunned actor.
 * @param[in] state Dead or stunned?
 * @param[in] attacker Pointer to attacker - it must be notified about state of victim.
 * @todo Discuss whether stunned actor should really drop everything to floor. Maybe
 * it should drop only what he has in hands? Stunned actor can wake later during mission.
 * @todo Renameme - stunned actor is not dead actor.
 */
void G_ActorDie (edict_t * ent, int state, edict_t *attacker)
{
	assert(ent);

	Com_DPrintf(DEBUG_GAME, "G_ActorDie: kill actor on team %i\n", ent->team);
	switch (state) {
	case STATE_DEAD:
		ent->state |= (1 + rand() % MAX_DEATH);
		if (attacker != NULL)
			level.num_kills[attacker->team][ent->team]++;
		break;
	case STATE_STUN:
		/**< @todo Is there a reason this is reset? We _may_ need that in the future somehow.
		 * @sa CL_ActorDie */
		ent->STUN = 0;
		ent->state = state;
		if (attacker != NULL)
			level.num_stuns[attacker->team][ent->team]++;
		break;
	default:
		Com_DPrintf(DEBUG_GAME, "G_ActorDie: unknown state %i\n", state);
		break;
	}
	VectorSet(ent->maxs, PLAYER_WIDTH, PLAYER_WIDTH, PLAYER_DEAD);
	gi.LinkEdict(ent);

	level.num_alive[ent->team]--;
	/* send death */
	G_EventActorDie(ent, attacker);

	/* handle inventory - drop everything to floor edict (but not the armour) */
	G_InventoryToFloor(ent);

	/* check if the player appears/perishes, seen from other teams */
	G_CheckVis(ent, qtrue);

	/* check if the attacker appears/perishes, seen from other teams */
	if (attacker)
		G_CheckVis(attacker, qtrue);

	/* calc new vis for this player */
	G_CheckVisTeamAll(ent->team, qfalse, attacker);
}

/**
 * @brief Moves an item inside an inventory. Floors are handled special.
 * @param[in] ent The pointer to the selected/used edict/soldier.
 * @param[in] from The container (-id) the item should be moved from.
 * @param[in] fItem The item you want to move.
 * @param[in] to The container (-id) the item should be moved to.
 * @param[in] tx x position where you want the item to go in the destination container
 * @param[in] ty y position where you want the item to go in the destination container
 * @param[in] checkaction Set this to qtrue if you want to check for TUs, otherwise qfalse.
 * @sa event PA_INVMOVE
 * @sa AI_ActorThink
 */
void G_ActorInvMove (edict_t *ent, const invDef_t * from, invList_t *fItem, const invDef_t * to, int tx,
		int ty, qboolean checkaction)
{
	player_t *player;
	edict_t *floor;
	qboolean newFloor;
	invList_t *ic;
	item_t item;
	int mask;
	inventory_action_t ia;
	invList_t fItemBackup;
	int fx, fy;
	int originalTU, reservedTU = 0;

	player = G_PLAYER_FROM_ENT(ent);

	assert(fItem);
	assert(fItem->item.t);

	/* Store the location/item of 'from' BEFORE actually moving items with I_MoveInInventory. */
	fItemBackup = *fItem;

	/* Get first used bit in item. */
	INVSH_GetFirstShapePosition(fItem, &fx, &fy);
	fx += fItem->x;
	fy += fItem->y;

	/* Check if action is possible */
	/* TUs are 1 here - but this is only a dummy - the real TU check is done in the inventory functions below */
	if (checkaction && !G_ActionCheck(player, ent, 1))
		return;

	/* "get floor ready" - searching for existing floor-edict */
	floor = G_GetFloorItems(ent);
	if (INV_IsFloorDef(to) && !floor) {
		/* We are moving to the floor, but no existing edict for this floor-tile found -> create new one */
		floor = G_SpawnFloor(ent->pos);
		newFloor = qtrue;
	} else if (INV_IsFloorDef(from) && !floor) {
		/* We are moving from the floor, but no existing edict for this floor-tile found -> this should never be the case. */
		gi.dprintf("G_ClientInvMove: No source-floor found.\n");
		return;
	} else {
		/* There already exists an edict for this floor-tile. */
		newFloor = qfalse;
	}

	/* search for space */
	if (tx == NONE) {
		ic = INVSH_SearchInInventory(&ent->i, from, fItem->x, fItem->y);
		if (ic)
			INVSH_FindSpace(&ent->i, &ic->item, to, &tx, &ty, fItem);
		if (tx == NONE)
			return;
	}

	/** @todo what if we don't have enough TUs after subtracting the reserved ones? */
	/* Because I_MoveInInventory don't know anything about character_t and it updates ent->TU,
	 * we need to save original ent->TU for the sake of checking TU reservations. */
	originalTU = ent->TU;
	reservedTU = G_ActorGetReservedTUs(ent);
	/* Temporary decrease ent->TU to make I_MoveInInventory do what expected. */
	ent->TU -= reservedTU;
	/* Try to actually move the item and check the return value after restoring valid ent->TU. */
	ia = game.i.MoveInInventory(&game.i, &ent->i, from, fItem, to, tx, ty, checkaction ? &ent->TU : NULL, &ic);
	/* Now restore the original ent->TU and decrease it for TU used for inventory move. */
	ent->TU = originalTU - (originalTU - reservedTU - ent->TU);

	switch (ia) {
	case IA_NONE:
		/* No action possible - abort */
		return;
	case IA_NOTIME:
		G_ClientPrintf(player, PRINT_HUD, _("Can't perform action - not enough TUs!\n"));
		return;
	case IA_NORELOAD:
		G_ClientPrintf(player, PRINT_HUD,
				_("Can't perform action - weapon already fully loaded with the same ammunition!\n"));
		return;
	default:
		/* Continue below. */
		break;
	}

	/* successful inventory change; remove the item in clients */
	if (INV_IsFloorDef(from)) {
		/* We removed an item from the floor - check how the client
		 * needs to be updated. */
		assert(!newFloor);
		if (FLOOR(ent)) {
			/* There is still something on the floor. */
			FLOOR(floor) = FLOOR(ent);
			G_EventInventoryDelete(floor, G_VisToPM(floor->visflags), from, fx, fy);
		} else {
			/* Floor is empty, remove the edict (from server + client) if we are
			 * not moving to it. */
			if (!INV_IsFloorDef(to)) {
				G_EventPerish(floor);
				G_FreeEdict(floor);
			}
		}
	} else {
		G_EventInventoryDelete(ent, G_TeamToPM(ent->team), from, fx, fy);
	}

	/* send tu's */
	G_SendStats(ent);

	assert(ic);
	item = ic->item;

	if (ia == IA_RELOAD || ia == IA_RELOAD_SWAP) {
		/* reload */
		if (INV_IsFloorDef(to))
			mask = G_VisToPM(floor->visflags);
		else
			mask = G_TeamToPM(ent->team);

		G_EventInventoryReload(INV_IsFloorDef(to) ? floor : ent, mask, &item, to, ic);

		if (ia == IA_RELOAD) {
			gi.EndEvents();
			return;
		} else { /* ia == IA_RELOAD_SWAP */
			item = fItemBackup.item;
			to = from;
			tx = fItemBackup.x;
			ty = fItemBackup.y;
		}
	}

	/* We moved an item to the floor - check how the client needs to be updated. */
	if (INV_IsFloorDef(to)) {
		/* we have to link the temp floor container to the new floor edict or add
		 * the item to an already existing floor edict - the floor container that
		 * is already linked might be from a different entity */
		FLOOR(floor) = FLOOR(ent);
		/* A new container was created for the floor. */
		if (newFloor) {
			/* Send item info to the clients */
			G_CheckVis(floor, qtrue);
		} else {
			G_EventInventoryAdd(floor, G_VisToPM(floor->visflags), 1);
			G_WriteItem(item, to, tx, ty);
		}
	} else {
		G_EventInventoryAdd(ent, G_TeamToPM(ent->team), 1);
		G_WriteItem(item, to, tx, ty);
	}

	G_UpdateReactionFire(ent, ent->chr.RFmode.fmIdx, ent->chr.RFmode.hand, ent->chr.RFmode.weapon);

	/* Other players receive weapon info only. */
	mask = G_VisToPM(ent->visflags) & ~G_TeamToPM(ent->team);
	if (mask) {
		if (INV_IsRightDef(from) || INV_IsLeftDef(from)) {
			G_EventInventoryDelete(ent, mask, from, fx, fy);
		}
		if (INV_IsRightDef(to) || INV_IsLeftDef(to)) {
			G_EventInventoryAdd(ent, mask, 1);
			G_WriteItem(item, to, tx, ty);
		}
	}
	gi.EndEvents();
}

/**
 * @brief Reload weapon with actor.
 * @param[in] ent Pointer to an actor reloading weapon.
 * @param[in] invDef Reloading weapon in right or left hand.
 * @sa AI_ActorThink
 */
void G_ActorReload (edict_t* ent, const invDef_t *invDef)
{
	invList_t *ic;
	invList_t *icFinal;
	objDef_t *weapon;
	int tu;
	containerIndex_t containerID;
	invDef_t *bestContainer;

	/* search for clips and select the one that is available easily */
	icFinal = NULL;
	tu = 100;
	bestContainer = NULL;

	if (CONTAINER(ent, invDef->id)) {
		weapon = CONTAINER(ent, invDef->id)->item.t;
	} else if (INV_IsLeftDef(invDef) && RIGHT(ent)->item.t->holdTwoHanded) {
		/* Check for two-handed weapon */
		invDef = INVDEF(gi.csi->idRight);
		weapon = CONTAINER(ent, invDef->id)->item.t;
	} else
		return;

	assert(weapon);

	/* LordHavoc: Check if item is researched when in singleplayer? I don't think this is really a
	 * cheat issue as in singleplayer there is no way to inject fake client commands in the virtual
	 * network buffer, and in multiplayer everything is researched */

	/* also try the temp containers */
	for (containerID = 0; containerID < gi.csi->numIDs; containerID++) {
		if (INVDEF(containerID)->out < tu) {
			/* Once we've found at least one clip, there's no point
			 * searching other containers if it would take longer
			 * to retrieve the ammo from them than the one
			 * we've already found. */
			for (ic = CONTAINER(ent, containerID); ic; ic = ic->next)
				if (INVSH_LoadableInWeapon(ic->item.t, weapon)) {
					icFinal = ic;
					bestContainer = INVDEF(containerID);
					tu = bestContainer->out;
					break;
				}
		}
	}

	/* send request */
	if (bestContainer)
		G_ActorInvMove(ent, bestContainer, icFinal, invDef, 0, 0, qtrue);
}

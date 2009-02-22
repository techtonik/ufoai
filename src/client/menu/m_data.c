/**
 * @file m_data.c
 */

/*
Copyright (C) 1997-2008 UFO:AI Team

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

#include "m_main.h"
#include "m_internal.h"

/**
 * @brief
 */
static const char *const menutextid_names[] = {
	"", /**< NULL value */
	"TEXT_STANDARD",
	"TEXT_LIST",
	"TEXT_UFOPEDIA",
	"TEXT_BUILDINGS",
	"TEXT_BUILDING_INFO",
	"TEXT_RESEARCH",
	"TEXT_POPUP",
	"TEXT_POPUP_INFO",
	"TEXT_AIRCRAFT_LIST",
	"TEXT_AIRCRAFT_INFO",
	"TEXT_MESSAGESYSTEM",
	"TEXT_CAMPAIGN_LIST",
	"TEXT_MULTISELECTION",
	"TEXT_PRODUCTION_LIST",
	"TEXT_PRODUCTION_AMOUNT",
	"TEXT_PRODUCTION_INFO",
	"TEXT_EMPLOYEE",
	"TEXT_MOUSECURSOR_RIGHT",
	"TEXT_PRODUCTION_QUEUED",
	"TEXT_STATS_BASESUMMARY",
	"TEXT_STATS_MISSION",
	"TEXT_STATS_BASES",
	"TEXT_STATS_NATIONS",
	"TEXT_STATS_EMPLOYEES",
	"TEXT_STATS_COSTS",
	"TEXT_STATS_INSTALLATIONS",
	"TEXT_STATS_7",
	"TEXT_BASE_LIST",
	"TEXT_BASE_INFO",
	"TEXT_TRANSFER_LIST",
	"TEXT_MOUSECURSOR_PLAYERNAMES",
	"TEXT_CARGO_LIST",
	"TEXT_UFOPEDIA_MAILHEADER",
	"TEXT_UFOPEDIA_MAIL",
	"TEXT_MARKET_NAMES",
	"TEXT_MARKET_STORAGE",
	"TEXT_MARKET_MARKET",
	"TEXT_MARKET_PRICES",
	"TEXT_CHAT_WINDOW",
	"TEXT_AIREQUIP_1",
	"TEXT_AIREQUIP_2",
	"TEXT_BASEDEFENCE_LIST",
	"TEXT_TIPOFTHEDAY",
	"TEXT_GENERIC",
	"TEXT_XVI",
	"TEXT_MOUSECURSOR_TOP",
	"TEXT_MOUSECURSOR_BOTTOM",
	"TEXT_MOUSECURSOR_LEFT",
	"TEXT_MESSAGEOPTIONS",

	"OPTION_LANGUAGES",
	"OPTION_JOYSTICKS",
	"OPTION_VIDEO_RESOLUTIONS",

	"LINESTRIP_FUNDING",
	"LINESTRIP_COLOR"
};
CASSERT(lengthof(menutextid_names) == MAX_MENUTEXTS);

/**
 * @brief Return a dataId by name
 * @return A dataId if data found, else -1
 */
int MN_GetDataIDByName (const char* name)
{
	int num;
	for (num = 0; num < MAX_MENUTEXTS; num++)
		if (!Q_strcmp(name, menutextid_names[num]))
			return num;

	return -1;
}

/**
 * @brief link a text to a menu text id
 * @note The menu doesn't manage the text memory, only save a pointer
 */
void MN_RegisterText (int textId, const char *text)
{
	mn.sharedData[textId].type = MN_SHARED_TEXT;
	mn.sharedData[textId].data.text = text;
	mn.sharedData[textId].versionId++;
}

/**
 * @brief link a text to a menu text id
 * @note The menu doesn't manage the text memory, only save a pointer
 */
void MN_RegisterLinkedListText (int textId, linkedList_t *text)
{
	mn.sharedData[textId].type = MN_SHARED_LINKEDLISTTEXT;
	mn.sharedData[textId].data.linkedListText = text;
	mn.sharedData[textId].versionId++;
}

const char *MN_GetText (int textId)
{
	if (mn.sharedData[textId].type != MN_SHARED_TEXT)
		return NULL;
	return mn.sharedData[textId].data.text;
}

int MN_GetDataVersion (int textId)
{
	return mn.sharedData[textId].versionId;
}

/**
 * @brief Reset a shared data. Type became NONE and value became NULL
 */
void MN_ResetData (int dataId)
{
	assert(dataId < MAX_MENUTEXTS);
	assert(dataId >= 0);

	if (mn.sharedData[dataId].type == MN_SHARED_LINKEDLISTTEXT)
		LIST_Delete(&mn.sharedData[dataId].data.linkedListText);

	mn.sharedData[dataId].type = MN_SHARED_NONE;
	mn.sharedData[dataId].data.text = NULL;
	mn.sharedData[dataId].versionId++;
}

/**
 * @brief Remove the higher element (in alphabet) from a list
 */
static menuOption_t *MN_OptionNodeRemoveHigherOption (menuOption_t **option)
{
	menuOption_t *prev = NULL;
	menuOption_t *prevfind = NULL;
	menuOption_t *search = (*option)->next;
	char *label = (*option)->label;

	/* search the smaller element */
	while (search) {
		if (Q_strcmp(label, search->label) < 0) {
			prevfind = prev;
			label = search->label;
		}
		prev = search;
		search = search->next;
	}

	/* remove the first element */
	if (prevfind == NULL) {
		menuOption_t *tmp = *option;
		*option = (*option)->next;
		return tmp;
	} else {
		menuOption_t *tmp = prevfind->next;
		prevfind->next = tmp->next;
		return tmp;
	}
}

/**
 * @brief Sort options by alphabet
 */
void MN_SortOptions (menuOption_t **first)
{
	menuOption_t *option;

	/* unlink the unsorted list */
	option = *first;
	if (option == NULL)
		return;
	*first = NULL;

	/* construct a sorted list */
	while (option) {
		menuOption_t *element;
		element = MN_OptionNodeRemoveHigherOption(&option);
		element->next = *first;
		*first = element;
	}
}

void MN_RegisterOption (int dataId, menuOption_t *option)
{
	mn.sharedData[dataId].type = MN_SHARED_OPTION;
	mn.sharedData[dataId].data.option = option;
	mn.sharedData[dataId].versionId++;
}

void MN_RegisterLineStrip(int dataId, lineStrip_t *lineStrip)
{
	mn.sharedData[dataId].type = MN_SHARED_LINESTRIP;
	mn.sharedData[dataId].data.lineStrip = lineStrip;
	mn.sharedData[dataId].versionId++;
}

menuOption_t *MN_GetOption (int dataId)
{
	if (mn.sharedData[dataId].type == MN_SHARED_OPTION) {
		return mn.sharedData[dataId].data.option;
	}
	return NULL;
}

/**
 * @brief Allocates an array of option
 */
menuOption_t* MN_AllocOption (int count)
{
	menuOption_t* newOptions;
	assert(count > 0);

	if (mn.numOptions + count >= MAX_MENUOPTIONS)
		Sys_Error("MN_AllocOption: numOptions exceeded - increase MAX_MENUOPTIONS");

	newOptions = &mn.menuOptions[mn.numOptions];
	mn.numOptions += count;
	return newOptions;
}

/**
 * @brief Resets the mn.menuText pointers from a func node
 * @note You can give this function a parameter to only delete a specific data
 * @sa menutextid_names
 */
static void MN_ResetData_f (void)
{
	if (Cmd_Argc() == 2) {
		const char *menuTextID = Cmd_Argv(1);
		const int id = MN_GetDataIDByName(menuTextID);
		if (id < 0)
			Com_Printf("%s: invalid mn.menuText ID: %s\n", Cmd_Argv(0), menuTextID);
		else
			MN_ResetData(id);
	} else {
		int i;
		for (i = 0; i < MAX_MENUTEXTS; i++)
			MN_ResetData(i);
	}
}

/**
 * @brief Initialize console command about shared menu data
 * @note called by MN_Init
 */
void MN_InitData (void)
{
	Cmd_AddCommand("mn_datareset", MN_ResetData_f, "Resets a menu data pointers");
}

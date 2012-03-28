/**
 * @file ui_node_messagelist.h
 */

/*
Copyright (C) 2002-2011 UFO: Alien Invasion.

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

#ifndef CLIENT_UI_UI_NODE_MESSAGELIST_H
#define CLIENT_UI_UI_NODE_MESSAGELIST_H

#include "ui_node_abstractscrollable.h"

class uiMessageListNode : public uiAbstractScrollableNode {
	void draw(struct uiNode_s *node);
	void loading(struct uiNode_s *node);
	qboolean scroll(struct uiNode_s *node, int deltaX, int deltaY);
	void mouseDown(struct uiNode_s *node, int x, int y, int button);
	void mouseUp(struct uiNode_s *node, int x, int y, int button);
	void capturedMouseMove(struct uiNode_s *node, int x, int y);
	int getCellHeight (uiNode_t *node);
};

struct uiBehaviour_s;
struct uiAction_s;

void UI_RegisterMessageListNode(struct uiBehaviour_s *behaviour);

#endif

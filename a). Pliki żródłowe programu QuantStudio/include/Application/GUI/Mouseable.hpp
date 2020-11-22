/**********************************************************************
Virtual base class for objects that are clickable and hoverable.
Author: Aron Mandrella
@2018
**********************************************************************/

#pragma once

class Mouseable
{
public:
	virtual void processMoveEvent(int x, int y) = 0;
	virtual void processClickEvent(int x, int y) = 0;
};

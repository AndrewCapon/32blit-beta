/*
 * CDCResetHandler.cpp
 *
 *  Created on: 18 Jan 2020
 *      Author: andrewcapon
 */

#include "CDCResetHandler.h"
#include "32blit.h"

bool CDCResetHandler::StreamInit(CDCFourCC uCommand)
{
  // force next reset to bool to firmware.
  blit_set_backup_value(BACKUP_FORCE_FIRMWARE, 1);

	NVIC_SystemReset();

	return false;
}


CDCCommandHandler::StreamResult CDCResetHandler::StreamData(CDCDataStream &dataStream)
{
	return srFinish;
}

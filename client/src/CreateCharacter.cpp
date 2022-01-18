/**
 * @file client/src/CreateCharacter.cpp
 * @ingroup client
 *
 * @author COMP Omega <compomega@tutanota.com>
 *
 * @brief Create character dialog.
 *
 * This file is part of the COMP_hack Test Client (client).
 *
 * Copyright (C) 2012-2021 COMP_hack Team <compomega@tutanota.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CreateCharacter.h"

// Qt Includes
#include <QSettings>

// libcomp Includes
#include <ErrorCodes.h>

// libclient Includes
#include "LogicWorker.h"

// client Includes
#include "GameWorker.h"
#include "LobbyScene.h"

// logic Messages
#include <MessageConnected.h>
#include <MessageConnectionInfo.h>

using namespace game;

using libcomp::Message::MessageClientType;

CreateCharacter::CreateCharacter(GameWorker *pWorker, QWidget *pParent)
    : QDialog(pParent),
      mLogicWorker(pWorker->GetLogicWorker()),
      mGameWorker(pWorker) {
  ui.setupUi(this);
}

CreateCharacter::~CreateCharacter() {}

void CreateCharacter::Create() {
  if (!ui.loginButton->isEnabled()) {
    return;
  }
}

bool CreateCharacter::ProcessClientMessage(
    const libcomp::Message::MessageClient *pMessage) {
  switch (to_underlying(pMessage->GetMessageClientType())) {
    case to_underlying(MessageClientType::CONNECTED_TO_LOBBY):
      return HandleCharacterCreated(pMessage);
    default:
      break;
  }

  return false;
}

bool CreateCharacter::HandleCharacterCreated(
    const libcomp::Message::MessageClient *pMessage) {
  return true;
}

/**
 * @file client/src/CreateCharacter.h
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

#ifndef LIBCLIENT_SRC_CREATECHARACTER_H
#define LIBCLIENT_SRC_CREATECHARACTER_H

// Qt Includes
#include "ui_CreateCharacter.h"

// libclient Includes
#include <ClientManager.h>

// Qt Forward Declarations
class QDnsLookup;

namespace logic {

class LogicWorker;

}  // namespace logic

namespace game {

class GameWorker;

/**
 * Dialog to create a character.
 */
class CreateCharacter : public QDialog, public logic::ClientManager {
  Q_OBJECT

 public:
  /**
   * Construct the login dialog.
   * @param pWorker The GameWorker for the UI.
   * @param pParent Parent Qt widget for the dialog.
   */
  CreateCharacter(GameWorker *pWorker, QWidget *pParent = nullptr);

  /**
   * Cleanup the dialog.
   */
  ~CreateCharacter() override;

  /**
   * Process a client message.
   * @param pMessage Client message to process.
   */
  bool ProcessClientMessage(const libcomp::Message::MessageClient *pMessage);

 private slots:
  /**
   * Called when the create button is clicked.
   */
  void Create();

 private:
  /**
   * Handle the create character reply.
   * @param pMessage Client message to process.
   */
  bool HandleCharacterCreated(const libcomp::Message::MessageClient *pMessage);

  /// Pointer to the LogicWorker.
  logic::LogicWorker *mLogicWorker;

  /// Pointer to the GameWorker.
  GameWorker *mGameWorker;

  /// UI for this dialog.
  Ui::CreateCharacter ui;
};

}  // namespace game

#endif  // LIBCLIENT_SRC_CREATECHARACTER_H

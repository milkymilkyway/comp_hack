/**
 * @file client/src/ControlCenter.h
 * @ingroup client
 *
 * @author COMP Omega <compomega@tutanota.com>
 *
 * @brief Control center UI (manages scripts & clients).
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

#ifndef LIBCLIENT_SRC_CONTROLCENTER_H
#define LIBCLIENT_SRC_CONTROLCENTER_H

// C++11 Includes
#include <unordered_map>

// Qt Includes
#include "ui_ControlCenter.h"

// libclient Includes
#include <ClientManager.h>

namespace logic {

class LogicWorker;

}  // namespace logic

namespace game {

class GameWorker;

}  // namespace game

namespace script {

class ScriptWorker;

/**
 * GUI data associated with a client.
 */
struct ClientData {
  /// Handle for the logic worker for the given client.
  logic::LogicWorker *logic;

  /// Game worker for the given client.
  std::shared_ptr<game::GameWorker> game;

  // List widget item.
  QListWidgetItem *pItem;
};

}  // namespace script

Q_DECLARE_METATYPE(script::ClientData *);

namespace script {

/**
 * Control center to manage scripts and clients.
 */
// class ControlCenter : public QWidget, public logic::ClientManager {
class ControlCenter : public QWidget {
  Q_OBJECT

 public:
  /**
   * Construct the control center.
   * @param pParent Parent Qt widget for the dialog.
   */
  ControlCenter(QWidget *pParent = nullptr);

  /**
   * Cleanup the control center.
   */
  ~ControlCenter() override;

  /**
   * Run a script passed from the command line.
   * @param path Path to the script file to run.
   */
  void runScriptFromCommandLine(const QString &path);

 protected:
  /**
   * Handle a close event on the scene.
   * @param pEvent Close event to handle.
   */
  void closeEvent(QCloseEvent *pEvent) override;

 signals:
  /**
   * Signal that indicates a client has been created.
   * @param pWorker Handle for the client.
   * @param friendlyName Friendly name for the new client.
   */
  void clientCreated(logic::LogicWorker *pWorker, const QString &friendlyName);

  /**
   * Signal that indicates a client has been deleted.
   * @param pWorker Handle for the client.
   */
  void clientDeleted(logic::LogicWorker *pWorker);

  /**
   * Signal that indicates a script has been run.
   * @param path Path to the script file that has been run.
   * @param result Result of the script file that has been run.
   */
  void scriptComplete(const QString &path, bool result);

 private slots:
  /**
   * Create a new client.
   */
  void clientCreate();

  /**
   * Delete a client.
   */
  void clientDelete();

  /**
   * Slogt that indicates a client has been created.
   * @param pWorker Handle for the client.
   * @param friendlyName Friendly name for the new client.
   */
  void clientCreatedSignal(logic::LogicWorker *pWorker,
                           const QString &friendlyName);

  /**
   * Slogt that indicates a client has been deleted.
   * @param pWorker Handle for the client.
   */
  void clientDeletedSignal(logic::LogicWorker *pWorker);

  /**
   * Slot that indicates a script has been run.
   * @param path Path to the script file that has been run.
   * @param result Result of the script file that has been run.
   */
  void scriptCompleteSignal(const QString &path, bool result);

  /**
   * Update the UI when the client list selection changes.
   */
  void itemSelectionChanged();

  /**
   * Run a script.
   */
  void scriptRun();

  /**
   * Browse for a script to run.
   */
  void scriptBrowse();

  /**
   * Check if the script path is valid.
   */
  void checkScriptPath();

 private:
  /// Indicates a script is running.
  bool mRunning;

  /// Pointer to the ScriptWorker.
  std::shared_ptr<ScriptWorker> mScriptWorker;

  // List of clients for this session.
  std::unordered_map<logic::LogicWorker *, ClientData *> mClients;

  /// UI for this dialog.
  Ui::ControlCenter ui;
};

}  // namespace script

#endif  // LIBCLIENT_SRC_CONTROLCENTER_H

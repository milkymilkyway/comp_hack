/**
 * @file client/src/ControlCenter.cpp
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

#include "ControlCenter.h"

// libcomp Includes
#include <Log.h>

// libclient Includes
#include <LogicWorker.h>

// client Includes
#include "GameWorker.h"
#include "ScriptWorker.h"

// logic Messages
#include <MessageCreateDeleteClient.h>
#include <MessageRunScript.h>

// Qt Includes
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>

using namespace script;

using game::cs;
using game::qs;

ControlCenter::ControlCenter(QWidget *pParent)
    : QWidget(pParent),
      mRunning(false),
      mScriptWorker(std::make_shared<ScriptWorker>()) {
  ui.setupUi(this);

  qRegisterMetaType<ClientData *>();

  connect(ui.clientCreate, SIGNAL(clicked(bool)), this, SLOT(clientCreate()));
  connect(ui.clientDelete, SIGNAL(clicked(bool)), this, SLOT(clientDelete()));
  connect(ui.scriptPath, SIGNAL(textChanged(const QString &)), this,
          SLOT(checkScriptPath()));
  connect(ui.scriptPath, SIGNAL(returnPressed()), this, SLOT(scriptRun()));
  connect(ui.scriptRun, SIGNAL(clicked(bool)), this, SLOT(scriptRun()));
  connect(ui.scriptBrowse, SIGNAL(clicked(bool)), this, SLOT(scriptBrowse()));
  connect(ui.clientList, SIGNAL(itemSelectionChanged()), this,
          SLOT(itemSelectionChanged()));

  // Queue these connections from the script thread.
  connect(this, SIGNAL(clientCreated(logic::LogicWorker *, const QString &)),
          this,
          SLOT(clientCreatedSignal(logic::LogicWorker *, const QString &)),
          Qt::QueuedConnection);
  connect(this, SIGNAL(clientDeleted(logic::LogicWorker *)), this,
          SLOT(clientDeletedSignal(logic::LogicWorker *)),
          Qt::QueuedConnection);
  connect(this, SIGNAL(scriptComplete(const QString &, bool)), this,
          SLOT(scriptCompleteSignal(const QString &, bool)),
          Qt::QueuedConnection);

  mScriptWorker->SetUIActive(true);
  mScriptWorker->SetCreateClientCallback(
      [&](script::ScriptWorker *pScriptWorker,
          logic::LogicWorker *pLogicWorker) {
        (void)pScriptWorker;

        if (pLogicWorker) {
          emit clientCreated(pLogicWorker, qs(pLogicWorker->GetFriendlyName()));
        }
      });

  mScriptWorker->SetDeleteClientCallback(
      [&](script::ScriptWorker *pScriptWorker,
          logic::LogicWorker *pLogicWorker) {
        (void)pScriptWorker;

        emit clientDeleted(pLogicWorker);
      });

  mScriptWorker->SetScriptCallback([&](script::ScriptWorker *pScriptWorker,
                                       const libcomp::String &path,
                                       bool result) {
    (void)pScriptWorker;
    emit scriptComplete(qs(path), result);
  });

  mScriptWorker->Start("script");

  QSettings settings;
  ui.scriptPath->setText(settings.value("last_script").toString());

  itemSelectionChanged();
  checkScriptPath();
}

ControlCenter::~ControlCenter() {
  mScriptWorker->Shutdown();
  mScriptWorker->Join();

  std::list<logic::LogicWorker *> workers;

  // Because clientDeletedSignal() will edit this map.
  for (auto pair : mClients) {
    workers.push_back(pair.first);
  }

  for (auto pWorker : workers) {
    clientDeletedSignal(pWorker);
  }
}

void ControlCenter::runScriptFromCommandLine(const QString &path) {
  ui.scriptPath->setText(path);

  checkScriptPath();
  scriptRun();
}

void ControlCenter::closeEvent(QCloseEvent *pEvent) {
  /// @todo Add code here???

  // Continue with the event.
  QWidget::closeEvent(pEvent);
}

void ControlCenter::clientCreate() {
  auto friendlyName =
      cs(QInputDialog::getText(this, tr("Friendly Name"), tr("Friendly Name:"),
                               QLineEdit::Normal, tr("Client")));

  if (friendlyName.IsEmpty()) {
    return;
  }

  mScriptWorker->SendToScript(new logic::MessageCreateClient(friendlyName));
}

void ControlCenter::clientDelete() {
  auto selectedItems = ui.clientList->selectedItems();

  if (selectedItems.empty()) {
    return;
  }

  auto pItem = selectedItems.front();

  if (pItem) {
    auto pClientData = pItem->data(Qt::UserRole).value<ClientData *>();

    if (pClientData) {
      mScriptWorker->SendToScript(new logic::MessageDeleteClient(
          pClientData->logic->GetUUID(), pClientData->logic));
    }
  }
}

void ControlCenter::clientCreatedSignal(logic::LogicWorker *pWorker,
                                        const QString &friendlyName) {
  // This should never happen but just in case...
  if (mClients.count(pWorker)) {
    clientDeletedSignal(pWorker);
  }

  // Create the client data.
  auto pClientData = new ClientData;

  // Add the client to the list.
  auto pItem = new QListWidgetItem(friendlyName);
  pItem->setData(Qt::UserRole, [&pClientData]() -> QVariant {
    QVariant v;
    v.setValue(pClientData);

    return v;
  }());
  ui.clientList->addItem(pItem);

  // Setup the client data.
  pClientData->logic = pWorker;
  pClientData->game = std::make_shared<game::GameWorker>(pWorker);
  pClientData->pItem = pItem;

  // Setup the client now.
  pClientData->game->SetNextWorker(mScriptWorker);
  pWorker->SetGameQueue(pClientData->game->GetMessageQueue());
  pClientData->game->Start(
      libcomp::String("game%1").Arg(pWorker->GetFriendlyName()));

  // Save the client data.
  mClients[pWorker] = pClientData;
}

void ControlCenter::clientDeletedSignal(logic::LogicWorker *pWorker) {
  // Look for the client.
  auto pClientData = mClients.at(pWorker);

  // If the client doesn't exist, exit.
  if (!pClientData) {
    return;
  }

  // Remove the client from the list.
  if (pClientData->pItem) {
    ui.clientList->removeItemWidget(pClientData->pItem);
    delete pClientData->pItem;
  }

  // Stop the GameWorker for the client.
  if (pClientData->game) {
    pClientData->game->SetNextWorker({});
    pClientData->game->Shutdown();
    pClientData->game->Join();
    pClientData->game.reset();
  }

  // Delete the client data.
  delete pClientData;

  // Remove the client from the list.
  mClients.erase(pWorker);
}

void ControlCenter::scriptCompleteSignal(const QString &path, bool result) {
  (void)path;

  if (!result) {
    QMessageBox::critical(this, tr("Script Error"),
                          tr("Script has failed! Check the log for details."));
  }

  mRunning = false;
  checkScriptPath();
}

void ControlCenter::itemSelectionChanged() {
  auto selectedItems = ui.clientList->selectedItems();

  ui.clientDelete->setEnabled(!selectedItems.empty());
}

void ControlCenter::scriptRun() {
  auto path = ui.scriptPath->text();

  if (mRunning || path.isEmpty() || !QFileInfo(path).isReadable()) {
    return;
  }

  QSettings settings;
  settings.setValue("last_script", path);

  ui.scriptRun->setEnabled(false);

  mScriptWorker->SendToScript(new logic::MessageRunScript(cs(path)));
  mRunning = true;
}

void ControlCenter::scriptBrowse() {
  QSettings settings;

  auto lastDir = settings.value("last_script_path").toString();
  auto path = QFileDialog::getOpenFileName(this, tr("Open Script"), lastDir,
                                           tr("Script Files (*.nut)"));
  if (path.isEmpty()) {
    return;
  }

  ui.scriptPath->setText(QDir::toNativeSeparators(path));
  lastDir = QFileInfo(path).absolutePath();
  settings.setValue("last_script_path", lastDir);
  checkScriptPath();
}

void ControlCenter::checkScriptPath() {
  QFileInfo info(ui.scriptPath->text());

  ui.scriptRun->setEnabled(!mRunning && info.isReadable());
}

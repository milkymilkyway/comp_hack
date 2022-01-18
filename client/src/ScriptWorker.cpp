/**
 * @file client/src/ScriptWorker.cpp
 * @ingroup client
 *
 * @author COMP Omega <compomega@tutanota.com>
 *
 * @brief Script worker to run automated client scripts.
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

#include "ScriptWorker.h"

// libcomp Includes
#include <Crypto.h>
#include <EnumUtils.h>
#include <Log.h>

// libhack Includes
#include <ScriptEngine.h>

// libclient Includes
#include <LogicWorker.h>

// logic messages
#include <MessageAccountDump.h>
#include <MessageCharacterList.h>
#include <MessageConnected.h>
#include <MessageConnectionInfo.h>
#include <MessageCreateDeleteClient.h>
#include <MessageRunScript.h>
#include <MessageShutdown.h>
#include <MessageStartGame.h>

#include <QFile>

// Managers
/// @todo Add some?

using namespace script;

using libcomp::Message::MessageClientType;
using libcomp::Message::MessageType;

static void ScriptSleep(double time) {
  std::this_thread::sleep_for(std::chrono::duration<double>(time));
}

ScriptWorker::ScriptWorker()
    : libcomp::Worker(),
      libcomp::Manager(),
      mUIActive(false),
      mEngine(nullptr) {
  // Construct the managers.
  /// @todo Add some?

  // Save pointers to the managers.
  /// @todo Add some?

  // Add the managers so they may process the queue.
  /// @todo Add some?
}

ScriptWorker::~ScriptWorker() {
  /// @todo Add some managers?

  //
  // Delete all clients that still exist.
  //
  std::list<logic::LogicWorker *> workers;

  // Because DeleteClient() will edit this vector.
  for (auto &pair : mClients) {
    workers.push_back(pair.second.get());
  }

  for (auto pWorker : workers) {
    DeleteClient(pWorker);
  }
}

void ScriptWorker::SetUIActive(bool active) { mUIActive = active; }

bool ScriptWorker::IsUIActive() const { return mUIActive; }

std::list<libcomp::Message::MessageType> ScriptWorker::GetSupportedTypes()
    const {
  return {
      MessageType::MESSAGE_TYPE_CLIENT,
  };
}

bool ScriptWorker::ProcessMessage(const libcomp::Message::Message *pMessage) {
  auto range = mMessageCallbacks.equal_range(pMessage->GetType());

  for (auto it = range.first; it != range.second; ++it) {
    it->second.Execute(
        std::shared_ptr<libcomp::Message::Message>(pMessage->Clone()));
  }

  switch (to_underlying(pMessage->GetType())) {
    case to_underlying(MessageType::MESSAGE_TYPE_CLIENT):
      return ProcessClientMessage(
          (const libcomp::Message::MessageClient *)pMessage);
    default:
      break;
  }

  // Don't report errors about not processing messages.
  return true;
}

void ScriptWorker::Run(
    libcomp::MessageQueue<libcomp::Message::Message *> *pMessageQueue) {
  // Add the manager after construction to avoid problems.
  AddManager(shared_from_this());

  libcomp::Worker::Run(pMessageQueue);

  RemoveManager(shared_from_this());
}

void ScriptWorker::Shutdown() {
  std::list<logic::LogicWorker *> workers;

  // Because DeleteClient() will edit this vector.
  for (auto &pair : mClients) {
    workers.push_back(pair.second.get());
  }

  for (auto pWorker : workers) {
    DeleteClient(pWorker);
  }

  libcomp::Worker::Shutdown();
}

bool ScriptWorker::SendToClient(const libobjgen::UUID &uuid,
                                libcomp::Message::Message *pMessage) {
  auto it = mClients.find(uuid);

  if (it == mClients.end()) {
    return false;
  }

  return it->second->SendToLogic(pMessage->Clone());
}

bool ScriptWorker::SendToScript(libcomp::Message::Message *pMessage) {
  auto pMessageQueue = GetMessageQueue();

  if (pMessageQueue) {
    pMessageQueue->Enqueue(pMessage);

    return true;
  } else {
    return false;
  }
}

bool ScriptWorker::ProcessClientMessage(
    const libcomp::Message::MessageClient *pMessage) {
  auto range =
      mClientMessageCallbacks.equal_range(pMessage->GetMessageClientType());

  for (auto it = range.first; it != range.second; ++it) {
    it->second.Execute(
        std::shared_ptr<libcomp::Message::Message>(pMessage->Clone()));
  }

  switch (to_underlying(pMessage->GetMessageClientType())) {
    case to_underlying(MessageClientType::CREATE_CLIENT): {
      auto pCreateClient =
          reinterpret_cast<const logic::MessageCreateClient *>(pMessage);
      CreateClient(pCreateClient ? pCreateClient->GetFriendlyName()
                                 : libcomp::String());

      return true;
    }
    case to_underlying(MessageClientType::DELETE_CLIENT): {
      DeleteClientScript(pMessage->GetClientUUID());

      return true;
    }
    case to_underlying(MessageClientType::RUN_SCRIPT): {
      auto pRunScript =
          reinterpret_cast<const logic::MessageRunScript *>(pMessage);
      bool result = RunScript(pRunScript->GetPath());

      if (mScriptCallback) {
        mScriptCallback(this, pRunScript->GetPath(), result);
      }

      return true;
    }
    default:
      break;
  }

  // Don't report errors about not processing messages.
  return true;
}

void ScriptWorker::SetCreateClientCallback(
    std::function<void(ScriptWorker *, logic::LogicWorker *)> callback) {
  mCreateCallback = callback;
}

void ScriptWorker::SetDeleteClientCallback(
    std::function<void(ScriptWorker *, logic::LogicWorker *)> callback) {
  mDeleteCallback = callback;
}

void ScriptWorker::SetScriptCallback(
    std::function<void(ScriptWorker *, const libcomp::String &, bool)>
        callback) {
  mScriptCallback = callback;
}

logic::LogicWorker *ScriptWorker::CreateClient(
    const libcomp::String &friendlyName) {
  // Create the new client.
  auto logicWorker = std::make_shared<logic::LogicWorker>();
  logicWorker->SetFriendlyName(friendlyName);

  // Attach the script worker as the first worker to get messages. GameWorker
  // may add iteself later.
  logicWorker->SetGameQueue(GetMessageQueue());

  // Start the worker.
  logicWorker->Start(libcomp::String("logic%1").Arg(friendlyName));

  // Add the new client to the list.
  mClients[logicWorker->GetUUID()] = logicWorker;

  if (mCreateCallback) {
    mCreateCallback(this, logicWorker.get());
  }

  return logicWorker.get();
}

libobjgen::UUID ScriptWorker::CreateClientScript(
    const libcomp::String &friendlyName) {
  auto pClient = CreateClient(friendlyName);

  if (nullptr != pClient) {
    return pClient->GetUUID();
  } else {
    return NULLUUID;
  }
}

void ScriptWorker::DeleteClient(logic::LogicWorker *pClient) {
  // Try looking for the client.
  auto it = mClients.find(pClient->GetUUID());

  // If found, delete the client.
  if (it != mClients.end()) {
    pClient->Shutdown();
    pClient->Join();

    if (mDeleteCallback) {
      mDeleteCallback(this, pClient);
    }

    mClients.erase(it);
  }
}

bool ScriptWorker::DeleteClientScript(const libobjgen::UUID &uuid) {
  auto it = mClients.find(uuid);

  if (it != mClients.end()) {
    DeleteClient(it->second.get());

    return true;
  } else {
    return false;
  }
}

bool ScriptWorker::WaitFor(Sqrat::Function func, double timeout) {
  if (0.0 >= timeout || func.IsNull()) {
    return false;
  }

  auto pMessageQueue = GetMessageQueue();

  if (!pMessageQueue) {
    return false;
  }

  std::shared_ptr<bool> result;

  if (!IsRunning()) {
    return false;
  }

  {
    std::list<libcomp::Message::Message *> msgs;
    pMessageQueue->DequeueAny(msgs);

    for (auto pMessage : msgs) {
      HandleMessage(pMessage);
    }
  }

  if ((result = func.Evaluate<bool>()) && *result) {
    return true;
  }

  while (0.01 <= timeout) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    timeout -= 0.01;

    if (!IsRunning()) {
      return false;
    }

    std::list<libcomp::Message::Message *> msgs;
    pMessageQueue->DequeueAny(msgs);

    for (auto pMessage : msgs) {
      HandleMessage(pMessage);
    }

    if ((result = func.Evaluate<bool>()) && *result) {
      return true;
    }
  }

  std::this_thread::sleep_for(std::chrono::duration<double>(timeout));

  if (!IsRunning()) {
    return false;
  }

  {
    std::list<libcomp::Message::Message *> msgs;
    pMessageQueue->DequeueAny(msgs);

    for (auto pMessage : msgs) {
      HandleMessage(pMessage);
    }
  }

  if ((result = func.Evaluate<bool>()) && *result) {
    return true;
  }

  return false;
}

bool ScriptWorker::WaitForMessage(int messageType, Sqrat::Function func,
                                  double timeout) {
  if (0.0 >= timeout || func.IsNull()) {
    return false;
  }

  auto pMessageQueue = GetMessageQueue();

  if (!pMessageQueue) {
    return false;
  }

  std::shared_ptr<libcomp::Message::Message> msg;

  if (!IsRunning()) {
    return false;
  }

  {
    std::list<libcomp::Message::Message *> msgs;
    pMessageQueue->DequeueAny(msgs);

    for (auto pMessage : msgs) {
      if (!msg && messageType == pMessage->GetRawType()) {
        msg = std::shared_ptr<libcomp::Message::Message>(pMessage->Clone());
      }

      HandleMessage(pMessage);
    }
  }

  if (msg) {
    func.Execute(std::shared_ptr<libcomp::Message::Message>(msg->Clone()));

    return true;
  }

  while (0.01 <= timeout) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    timeout -= 0.01;

    if (!IsRunning()) {
      return false;
    }

    std::list<libcomp::Message::Message *> msgs;
    pMessageQueue->DequeueAny(msgs);

    for (auto pMessage : msgs) {
      if (!msg && messageType == pMessage->GetRawType()) {
        msg = std::shared_ptr<libcomp::Message::Message>(pMessage->Clone());
      }

      HandleMessage(pMessage);
    }

    if (msg) {
      func.Execute(std::shared_ptr<libcomp::Message::Message>(msg->Clone()));

      return true;
    }
  }

  std::this_thread::sleep_for(std::chrono::duration<double>(timeout));

  if (!IsRunning()) {
    return false;
  }

  {
    std::list<libcomp::Message::Message *> msgs;
    pMessageQueue->DequeueAny(msgs);

    for (auto pMessage : msgs) {
      if (!msg && messageType == pMessage->GetRawType()) {
        msg = std::shared_ptr<libcomp::Message::Message>(pMessage->Clone());
      }

      HandleMessage(pMessage);
    }
  }

  if (msg) {
    func.Execute(std::shared_ptr<libcomp::Message::Message>(msg->Clone()));

    return true;
  }

  return false;
}

bool ScriptWorker::WaitForClientMessage(const libobjgen::UUID &uuid,
                                        int clientMessageType,
                                        Sqrat::Function func, double timeout) {
  if (0.0 >= timeout || func.IsNull()) {
    return false;
  }

  auto pMessageQueue = GetMessageQueue();

  if (!pMessageQueue) {
    return false;
  }

  std::shared_ptr<libcomp::Message::Message> msg;

  if (!IsRunning()) {
    return false;
  }

  {
    std::list<libcomp::Message::Message *> msgs;
    pMessageQueue->DequeueAny(msgs);

    for (auto pMessage : msgs) {
      auto pClientMsg =
          dynamic_cast<libcomp::Message::MessageClient *>(pMessage);

      if (!msg && pClientMsg &&
          (uuid.IsNull() || uuid == pClientMsg->GetClientUUID()) &&
          clientMessageType == pClientMsg->GetRawMessageClientType()) {
        msg = std::shared_ptr<libcomp::Message::Message>(pMessage->Clone());
      }

      HandleMessage(pMessage);
    }
  }

  if (msg) {
    func.Execute(std::shared_ptr<libcomp::Message::Message>(msg->Clone()));

    return true;
  }

  while (0.01 <= timeout) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    timeout -= 0.01;

    if (!IsRunning()) {
      return false;
    }

    std::list<libcomp::Message::Message *> msgs;
    pMessageQueue->DequeueAny(msgs);

    for (auto pMessage : msgs) {
      auto pClientMsg =
          dynamic_cast<libcomp::Message::MessageClient *>(pMessage);

      if (!msg && pClientMsg &&
          (uuid.IsNull() || uuid == pClientMsg->GetClientUUID()) &&
          clientMessageType == pClientMsg->GetRawMessageClientType()) {
        msg = std::shared_ptr<libcomp::Message::Message>(pMessage->Clone());
      }

      HandleMessage(pMessage);
    }

    if (msg) {
      func.Execute(std::shared_ptr<libcomp::Message::Message>(msg->Clone()));

      return true;
    }
  }

  std::this_thread::sleep_for(std::chrono::duration<double>(timeout));

  if (!IsRunning()) {
    return false;
  }

  {
    std::list<libcomp::Message::Message *> msgs;
    pMessageQueue->DequeueAny(msgs);

    for (auto pMessage : msgs) {
      auto pClientMsg =
          dynamic_cast<libcomp::Message::MessageClient *>(pMessage);

      if (!msg && pClientMsg &&
          (uuid.IsNull() || uuid == pClientMsg->GetClientUUID()) &&
          clientMessageType == pClientMsg->GetRawMessageClientType()) {
        msg = std::shared_ptr<libcomp::Message::Message>(pMessage->Clone());
      }

      HandleMessage(pMessage);
    }
  }

  if (msg) {
    func.Execute(std::shared_ptr<libcomp::Message::Message>(msg->Clone()));

    return true;
  }

  return false;
}

void ScriptWorker::RegisterMessageCallback(int messageType,
                                           Sqrat::Function func) {
  mMessageCallbacks.insert({(libcomp::Message::MessageType)messageType, func});
}

void ScriptWorker::RegisterClientMessageCallback(int messageType,
                                                 Sqrat::Function func) {
  mClientMessageCallbacks.insert(
      {(libcomp::Message::MessageClientType)messageType, func});
}

bool ScriptWorker::ScriptInclude(const libcomp::String &path) {
  if (mIncludes.count(path)) {
    return true;
  }

  std::vector<char> file = libcomp::Crypto::LoadFile(path.ToUtf8());

  if (file.empty()) {
    return false;
  }

  file.push_back(0);

  if (!mEngine->Eval(&file[0], path.C())) {
    return false;
  }

  mIncludes.insert(path);

  return true;
}

namespace libcomp {
template <>
BaseScriptEngine &BaseScriptEngine::Using<ScriptWorker>() {
  if (!BindingExists("ScriptWorker")) {
    // See also Sqrat::NoCopy and Sqrat::CopyOnly
    Using<libobjgen::UUID>();
    Using<logic::MessageAccountDump>();
    Using<logic::MessageAccountDumpStatus>();
    Using<logic::MessageCharacterList>();
    Using<logic::MessageConnectedToChannel>();
    Using<logic::MessageConnectedToLobby>();
    Using<logic::MessageConnectionClose>();
    Using<logic::MessageConnectToChannel>();
    Using<logic::MessageConnectToLobby>();
    Using<logic::MessageRequestStartGame>();
    Using<libcomp::Packet>();

    Sqrat::Class<ScriptWorker> binding(mVM, "ScriptWorker");
    Bind<ScriptWorker>("ScriptWorker", binding);

    // These are needed by the script engine.
    // Using<objects::MiDevilData>();

    binding.Func("Include", &ScriptWorker::ScriptInclude)
        .Func("WaitFor", &ScriptWorker::WaitFor)
        .Func("WaitForMessage", &ScriptWorker::WaitForMessage)
        .Func("WaitForClientMessage", &ScriptWorker::WaitForClientMessage)
        .Func("SendToClient", &ScriptWorker::SendToClient)
        .Func("CreateClient", &ScriptWorker::CreateClientScript)
        .Func("DeleteClient", &ScriptWorker::DeleteClientScript)
        .Func("RegisterMessageCallback", &ScriptWorker::RegisterMessageCallback)
        .Func("RegisterClientMessageCallback",
              &ScriptWorker::RegisterClientMessageCallback)
        .Func("IsUIActive", &ScriptWorker::IsUIActive)
        .Prop("UIActive", &ScriptWorker::IsUIActive);  // Last call to binding

    // Bind some constants as well
    Sqrat::Enumeration messageTypes(mVM);
    messageTypes.Const(
        "MESSAGE_TYPE_SYSTEM",
        to_underlying(libcomp::Message::MessageType::MESSAGE_TYPE_SYSTEM));
    messageTypes.Const(
        "MESSAGE_TYPE_PACKET",
        to_underlying(libcomp::Message::MessageType::MESSAGE_TYPE_PACKET));
    messageTypes.Const(
        "MESSAGE_TYPE_CONNECTION",
        to_underlying(libcomp::Message::MessageType::MESSAGE_TYPE_CONNECTION));
    messageTypes.Const(
        "MESSAGE_TYPE_CLIENT",
        to_underlying(libcomp::Message::MessageType::MESSAGE_TYPE_CLIENT));

    Sqrat::Enumeration clientMessageTypes(mVM);
    clientMessageTypes.Const(
        "CONNECTED_TO_LOBBY",
        to_underlying(libcomp::Message::MessageClientType::CONNECTED_TO_LOBBY));
    clientMessageTypes.Const(
        "CHARACTER_LIST_UPDATE",
        to_underlying(
            libcomp::Message::MessageClientType::CHARACTER_LIST_UPDATE));

    Sqrat::ConstTable(mVM)
        .Enum("ClientMessageType", clientMessageTypes)
        .Enum("MessageType", messageTypes);

    Sqrat::RootTable(mVM).Func("Sleep", &ScriptSleep);
  }

  return *this;
}
}  // namespace libcomp

bool ScriptWorker::RunScript(const libcomp::String &path) {
  auto scriptData = libcomp::Crypto::LoadFile(path.ToUtf8());

  if (scriptData.empty()) {
    LogScriptEngineError([&]() {
      return libcomp::String("Failed to load script: %1\n").Arg(path);
    });

    return false;
  }

  scriptData.push_back(0);

  auto script = libcomp::String(&scriptData[0]);
  scriptData.clear();

  libhack::ScriptEngine engine(true /* don't prefix log messages */);
  engine.Using<ScriptWorker>();

  QFile builtins(":/builtins.nut");
  if (builtins.open(QIODevice::ReadOnly)) {
    auto bytes = builtins.readAll();
    bytes.append('\0');

    (void)engine.Eval(bytes.constData(), ":/builtins.nut");
  }

  // Save this off for functions like Include().
  mEngine = &engine;

  Sqrat::RootTable(engine.GetVM())
      .SetValue("SCRIPT_ENGINE", shared_from_this());

  bool result = engine.Eval(script, path);

  mMessageCallbacks.clear();
  mClientMessageCallbacks.clear();

  // Engine is no longer valid.
  mEngine = nullptr;

  return result;
}

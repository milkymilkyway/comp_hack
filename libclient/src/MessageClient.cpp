/**
 * @file libclient/src/MessageClient.cpp
 * @ingroup libclient
 *
 * @author COMP Omega <compomega@tutanota.com>
 *
 * @brief Base message class for client messages.
 *
 * This file is part of the COMP_hack Client Library (libclient).
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

#include "MessageClient.h"

#include "BaseScriptEngine.h"
#include "LogicWorker.h"

using namespace libcomp;

namespace libcomp {
template <>
BaseScriptEngine &BaseScriptEngine::Using<Message::MessageClient>() {
  if (!BindingExists("Message.MessageClient")) {
    Using<Message::Message>();

    Sqrat::DerivedClass<Message::MessageClient, Message::Message,
                        Sqrat::NoConstructor<Message::MessageClient>>
        binding(mVM, "Message.MessageClient");
    Bind("Message.MessageClient", binding);

    binding
        .Func("GetMessageClientType",
              &Message::MessageClient::GetRawMessageClientType)
        .Func("GetRawMessageClientType",
              &Message::MessageClient::GetRawMessageClientType)
        .Func("GetClienUUIDt", &Message::MessageClient::GetClientUUID)
        .Prop("MessageClientType",
              &Message::MessageClient::GetRawMessageClientType)
        .Prop("RawMessageClientType",
              &Message::MessageClient::GetRawMessageClientType)
        .Prop("ClientUUID", &Message::MessageClient::GetClientUUID);
  }

  return *this;
}
}  // namespace libcomp

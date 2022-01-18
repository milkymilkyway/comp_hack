/**
 * @file libcomp/src/MessageConnectionInfo.cpp
 * @ingroup libcomp
 *
 * @author COMP Omega <compomega@tutanota.com>
 *
 * @brief Client message.
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

#include "MessageConnectionInfo.h"

#include <BaseScriptEngine.h>

namespace libcomp {
template <>
BaseScriptEngine &BaseScriptEngine::Using<logic::MessageConnectionInfo>() {
  if (!BindingExists("logic.MessageConnectionInfo")) {
    Using<Message::MessageClient>();

    Sqrat::DerivedClass<logic::MessageConnectionInfo, Message::MessageClient,
                        Sqrat::NoConstructor<logic::MessageConnectionInfo>>
        binding(mVM, "logic.MessageConnectionInfo");
    Bind("logic.MessageConnectionInfo", binding);

    binding
        .Func("GetConnectionID", &logic::MessageConnectionInfo::GetConnectionID)
        .Prop("ConnectionID", &logic::MessageConnectionInfo::GetConnectionID)
        .Func("GetHost", &logic::MessageConnectionInfo::GetHost)
        .Prop("Host", &logic::MessageConnectionInfo::GetHost)
        .Func("GetPort", &logic::MessageConnectionInfo::GetPort)
        .Prop("Port", &logic::MessageConnectionInfo::GetPort);
  }

  return *this;
}

template <>
BaseScriptEngine &BaseScriptEngine::Using<logic::MessageConnectToLobby>() {
  if (!BindingExists("logic.MessageConnectToLobby")) {
    Using<logic::MessageConnectionInfo>();

    Sqrat::DerivedClass<logic::MessageConnectToLobby,
                        logic::MessageConnectionInfo>
        binding(mVM, "logic.MessageConnectToLobby");
    Bind("logic.MessageConnectToLobby", binding);

    binding
        .Ctor<const libobjgen::UUID &, const libcomp::String &,
              const libcomp::String &, uint32_t, const libcomp::String &,
              const libcomp::String &, uint16_t, const libobjgen::UUID &>()
        .Func("GetUsername", &logic::MessageConnectToLobby::GetUsername)
        .Prop("Username", &logic::MessageConnectToLobby::GetUsername)
        .Func("GetPassword", &logic::MessageConnectToLobby::GetPassword)
        .Prop("Password", &logic::MessageConnectToLobby::GetPassword)
        .Func("GetClientVersion",
              &logic::MessageConnectToLobby::GetClientVersion)
        .Prop("ClientVersion", &logic::MessageConnectToLobby::GetClientVersion)
        .Func("GetMachineUUID", &logic::MessageConnectToLobby::GetMachineUUID)
        .Prop("MachineUUID", &logic::MessageConnectToLobby::GetMachineUUID);
  }

  return *this;
}

template <>
BaseScriptEngine &BaseScriptEngine::Using<logic::MessageConnectToChannel>() {
  if (!BindingExists("logic.MessageConnectToChannel")) {
    Using<logic::MessageConnectionInfo>();

    Sqrat::DerivedClass<logic::MessageConnectToChannel,
                        logic::MessageConnectionInfo>
        binding(mVM, "logic.MessageConnectToChannel");
    Bind("logic.MessageConnectToChannel", binding);

    binding
        .Func("GetSessionKey", &logic::MessageConnectToChannel::GetSessionKey)
        .Prop("SessionKey", &logic::MessageConnectToChannel::GetSessionKey);
  }

  return *this;
}

template <>
BaseScriptEngine &BaseScriptEngine::Using<logic::MessageConnectionClose>() {
  if (!BindingExists("logic.MessageConnectionClose")) {
    Using<libcomp::Message::MessageClient>();

    Sqrat::DerivedClass<logic::MessageConnectionClose,
                        libcomp::Message::MessageClient>
        binding(mVM, "logic.MessageConnectionClose");
    Bind("logic.MessageConnectionClose", binding);
  }

  return *this;
}
}  // namespace libcomp

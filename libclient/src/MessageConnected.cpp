/**
 * @file libcomp/src/MessageConnected.cpp
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

#include "MessageConnected.h"

#include <BaseScriptEngine.h>

namespace libcomp {
template <>
BaseScriptEngine &BaseScriptEngine::Using<logic::MessageConnected>() {
  if (!BindingExists("logic.MessageConnected")) {
    Using<Message::MessageClient>();

    Sqrat::DerivedClass<logic::MessageConnected, Message::MessageClient,
                        Sqrat::NoConstructor<logic::MessageConnected>>
        binding(mVM, "logic.MessageConnected");
    Bind("logic.MessageConnected", binding);

    binding.Func("GetConnectionID", &logic::MessageConnected::GetConnectionID)
        .Prop("ConnectionID", &logic::MessageConnected::GetConnectionID)
        .Func("GetErrorCode", &logic::MessageConnected::GetRawErrorCode)
        .Prop("ErrorCode", &logic::MessageConnected::GetRawErrorCode)
        .Func("GetRawErrorCode", &logic::MessageConnected::GetRawErrorCode)
        .Prop("RawErrorCode", &logic::MessageConnected::GetRawErrorCode);
  }

  return *this;
}

template <>
BaseScriptEngine &BaseScriptEngine::Using<logic::MessageConnectedToLobby>() {
  if (!BindingExists("logic.MessageConnectedToLobby")) {
    Using<logic::MessageConnected>();

    Sqrat::DerivedClass<logic::MessageConnectedToLobby, logic::MessageConnected>
        binding(mVM, "logic.MessageConnectedToLobby");
    Bind("logic.MessageConnectedToLobby", binding);

    binding.Func("GetSID", &logic::MessageConnectedToLobby::GetSID)
        .Prop("SID", &logic::MessageConnectedToLobby::GetSID);
  }

  return *this;
}

template <>
BaseScriptEngine &BaseScriptEngine::Using<logic::MessageConnectedToChannel>() {
  if (!BindingExists("logic.MessageConnectedToChannel")) {
    Using<logic::MessageConnected>();

    Sqrat::DerivedClass<logic::MessageConnectedToChannel,
                        logic::MessageConnected>
        binding(mVM, "logic.MessageConnectedToChannel");
    Bind("logic.MessageConnectedToChannel", binding);
  }

  return *this;
}
}  // namespace libcomp

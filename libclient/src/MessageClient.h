/**
 * @file libclient/src/MessageClient.h
 * @ingroup libclient
 *
 * @author COMP Omega <compomega@tutanota.com>
 *
 * @brief Base message class for client messages.
 *
 * This file is part of the COMP_hack Client Library (libclient).
 *
 * Copyright (C) 2012-2020 COMP_hack Team <compomega@tutanota.com>
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

#include "Message.h"

#ifndef LIBCLIENT_SRC_MESSAGECLIENT_H
#define LIBCLIENT_SRC_MESSAGECLIENT_H

/// libobjgen Includes
#include <UUID.h>

namespace logic {

class LogicWorker;

}  // namespace logic

namespace libcomp {

namespace Message {

/**
 * Specific connection message type.
 */
enum class MessageClientType : int32_t {
  //
  // ConnectionManager related requests
  //

  /// Connect to the lobby server
  CONNECT_TO_LOBBY = 1000,
  /// Connect to the channel server
  CONNECT_TO_CHANNEL,
  /// Close the active connection
  CONNECTION_CLOSE,

  //
  // ConnectionManager related events
  //

  /// Now connected to the lobby
  CONNECTED_TO_LOBBY = 2000,
  /// Now connected to the channel
  CONNECTED_TO_CHANNEL,

  //
  // LobbyManager related requests
  //

  REQUEST_START_GAME = 3000,

  //
  // LobbyManager related events
  //
  CHARACTER_LIST_UPDATE = 4000,

  //
  // AmalaManager related requests
  //
  REQUEST_ACCOUNT_DUMP = 5000,

  //
  // AmalaManager related events
  //
  ACCOUNT_DUMP_STATUS = 6000,

  //
  // Script related events
  //
  CREATE_CLIENT = 7000,
  DELETE_CLIENT,
  RUN_SCRIPT,
  SEND_PACKET,
  SEND_OBJECT,
  PACKET_RECEIVED,
};

/**
 * Message signifying that a connection based action has occurred.
 */
class MessageClient : public Message {
 public:
  /**
   * Create the message.
   * @param uuid Client UUID this message is involved with.
   */
  MessageClient(const libobjgen::UUID &uuid)
      : libcomp::Message::Message(), mClientUUID(uuid) {}

  /**
   * Cleanup the message.
   */
  ~MessageClient() override {}

  /**
   * Get the message type.
   * @return The message's type.
   */
  MessageType GetType() const override {
    return MessageType::MESSAGE_TYPE_CLIENT;
  }

  /**
   * Get the specific client message type.
   * @return The message's client message type.
   */
  virtual MessageClientType GetMessageClientType() const = 0;

  /**
   * Get the specific raw client message type.
   * @return The message's raw client message type.
   */
  int GetRawMessageClientType() const {
    return to_underlying(GetMessageClientType());
  }

  /**
   * Get the client UUID this message is involved with.
   * @return Client UUID this message is involved with.
   */
  libobjgen::UUID GetClientUUID() const { return mClientUUID; }

 protected:
  /// Client UUID this message is involved with.
  libobjgen::UUID mClientUUID;
};

}  // namespace Message

}  // namespace libcomp

#endif  // LIBCLIENT_SRC_MESSAGECLIENT_H

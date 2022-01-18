/**
 * @file libclient/src/MessageCreateDeleteClient.h
 * @ingroup libclient
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

#ifndef LIBCLIENT_SRC_MESSAGECREATEDELETECLIENT_H
#define LIBCLIENT_SRC_MESSAGECREATEDELETECLIENT_H

// libcomp Includes
#include <CString.h>
#include <MessageClient.h>

namespace logic {

/**
 * Message requesting a new client be created.
 */
class MessageCreateClient : public libcomp::Message::MessageClient {
 public:
  /**
   * Create the message.
   * @param friendlyName A friendly name for the client.
   */
  MessageCreateClient(const libcomp::String &friendlyName)
      : libcomp::Message::MessageClient(NULLUUID),
        mFriendlyName(friendlyName) {}

  /**
   * Cleanup the message.
   */
  ~MessageCreateClient() override {}

  Message *Clone() const override { return new MessageCreateClient(*this); }

  /**
   * Get the friendly name for the new client.
   * @returns Friendly name for the new client.
   */
  libcomp::String GetFriendlyName() const { return mFriendlyName; }

  /**
   * Get the specific client message type.
   * @return The message's client message type
   */
  libcomp::Message::MessageClientType GetMessageClientType() const override {
    return libcomp::Message::MessageClientType::CREATE_CLIENT;
  }

  /**
   * Dump the message for logging.
   * @return String representation of the message.
   */
  libcomp::String Dump() const override {
    return libcomp::String("Message: Create client\nFriendly Name: %1")
        .Arg(mFriendlyName);
  }

 protected:
  /// Friendly name for the client.
  libcomp::String mFriendlyName;
};

/**
 * Message requesting a client be deleted.
 */
class MessageDeleteClient : public libcomp::Message::MessageClient {
 public:
  /**
   * Create the message.
   * @param uuid Client UUID this message is involved with.
   * @param pClient Client this message is involved with.
   */
  MessageDeleteClient(const libobjgen::UUID &uuid, logic::LogicWorker *pClient)
      : libcomp::Message::MessageClient(uuid), mClient(pClient) {}

  /**
   * Cleanup the message.
   */
  ~MessageDeleteClient() override {}

  Message *Clone() const override { return new MessageDeleteClient(*this); }

  /**
   * Get the client this message is involved with.
   * @return Client this message is involved with.
   */
  logic::LogicWorker *GetClient() const { return mClient; }

  /**
   * Get the specific client message type.
   * @return The message's client message type
   */
  libcomp::Message::MessageClientType GetMessageClientType() const override {
    return libcomp::Message::MessageClientType::DELETE_CLIENT;
  }

  /**
   * Dump the message for logging.
   * @return String representation of the message.
   */
  libcomp::String Dump() const override { return "Message: Delete client"; }

 protected:
  /// Client this message is involved with.
  logic::LogicWorker *mClient;
};

}  // namespace logic

#endif  // LIBCLIENT_SRC_MESSAGECREATEDELETECLIENT_H

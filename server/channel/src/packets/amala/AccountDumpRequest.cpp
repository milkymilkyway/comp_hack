/**
 * @file server/channel/src/packets/amala/AccountDumpRequest.cpp
 * @ingroup channel
 *
 * @author COMP Omega <compomega@tutanota.com>
 *
 * @brief Request from the client to dump the account information.
 *
 * This file is part of the Channel Server (channel).
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

#include "Packets.h"

// libcomp Includes
#include <Account.h>
#include <AccountLogin.h>
#include <Crypto.h>
#include <Log.h>
#include <ManagerPacket.h>
#include <Packet.h>
#include <PacketCodes.h>

// channel Includes
#include "AccountManager.h"
#include "ChannelServer.h"

using namespace channel;

#define PART_SIZE (1024)

void SendPart(const std::shared_ptr<ChannelClientConnection> client,
              int partNumber) {
  // Don't bother if the client is no longer connected.
  if (libcomp::TcpConnection::ConnectionStatus_t::STATUS_ENCRYPTED !=
      client->GetStatus()) {
    return;
  }

  auto state = client->GetClientState();

  auto partOffset = state->GetNextAccountDumpOffset();
  auto dumpSize = state->GetAccountDumpSize();
  auto szDumpData = state->GetAccountDumpData();

  // Sanity check the account dump data.
  if (!szDumpData || 0 == dumpSize || 0 > partOffset) {
    return;
  }

  uint32_t partSize = (uint32_t)dumpSize - (uint32_t)partOffset;

  if (PART_SIZE < partSize) {
    partSize = PART_SIZE;
  }

  libcomp::Packet reply;
  reply.WritePacketCode(
      ChannelToClientPacketCode_t::PACKET_AMALA_ACCOUNT_DUMP_PART);
  reply.WriteS32Little(partNumber);
  reply.WriteU32Little(partSize);
  reply.WriteArray(&szDumpData[partOffset], partSize);

  client->SendPacket(reply);

  if (!state->HaveNextAccountDumpOffset()) {
    state->ClearAccountDumpData();
  }
}

void DumpAccount(std::shared_ptr<ChannelServer> server,
                 AccountManager* accountManager,
                 const std::shared_ptr<ChannelClientConnection> client,
                 bool throttleParts, bool waitForRequest) {
  auto state = client->GetClientState();

  std::string dump = accountManager->DumpAccount(state).ToUtf8();

  // Send the account dump to the client.
  if (!dump.empty()) {
    std::vector<char> dumpData(dump.c_str(), dump.c_str() + dump.size());
    auto partCount = ((uint32_t)dumpData.size() + PART_SIZE - 1) / PART_SIZE;

    {
      auto accountName = state->GetAccountLogin()->GetAccount()->GetUsername();

      libcomp::Packet reply;
      reply.WritePacketCode(
          ChannelToClientPacketCode_t::PACKET_AMALA_ACCOUNT_DUMP_HEADER);
      reply.WriteU32Little((uint32_t)dump.size());
      reply.WriteU32Little(partCount);
      reply.WriteString16Little(libcomp::Convert::Encoding_t::ENCODING_UTF8,
                                libcomp::Crypto::SHA1(dumpData), true);
      reply.WriteString16Little(libcomp::Convert::Encoding_t::ENCODING_UTF8,
                                accountName, true);

      client->SendPacket(reply);
    }

    auto startStamp = server->GetServerTime();

    std::list<int32_t> parts;
    parts.push_back(0);

    for (uint32_t i = 1; i < partCount; ++i) {
      parts.push_back((int)(i * PART_SIZE));

      // Send another part of the account dump every 2 ms.
      if (throttleParts) {
        server->ScheduleWork(startStamp + i * 2000ULL, SendPart, client, i + 1);
      }
    }

    state->SetAccountDumpData(std::move(dumpData), std::move(parts));

    SendPart(client, 1);

    if (!throttleParts && !waitForRequest) {
      for (uint32_t i = 1; i < partCount; ++i) {
        SendPart(client, (int)(i + 1));
      }
    }
  }
}

bool Parsers::AmalaAccountDumpRequest::Parse(
    libcomp::ManagerPacket* pPacketManager,
    const std::shared_ptr<libcomp::TcpConnection>& connection,
    libcomp::ReadOnlyPacket& p) const {
  if (0 != p.Size() && 1 != p.Size()) {
    return false;
  }

  bool throttleParts = true;
  bool waitForRequest = false;

  if (1 == p.Size()) {
    switch (p.ReadU8()) {
      case 1:
        throttleParts = false;
        waitForRequest = false;
        break;
      case 2:
        throttleParts = false;
        waitForRequest = true;
        break;
      default:
        break;
    }
  }

  auto client = std::dynamic_pointer_cast<ChannelClientConnection>(connection);
  auto server =
      std::dynamic_pointer_cast<ChannelServer>(pPacketManager->GetServer());

  server->QueueWork(DumpAccount, server, server->GetAccountManager(), client,
                    throttleParts, waitForRequest);

  return true;
}

uuid <- CreateClient("test client 1");
SendToClient(uuid, logic.MessageConnectToLobby(uuid, "testalpha", "same_as_my_luggage",
    1666, "lobby", "127.0.0.1", 10666, UUID()));
msg <- WaitForClientMessage(uuid, ClientMessageType.CONNECTED_TO_LOBBY);
assert(msg.ErrorCode == 0); /// @todo add enum for the error codes?

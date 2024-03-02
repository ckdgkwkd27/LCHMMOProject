using System;
using Google.Protobuf;
using UnityEngine;
using ServerCore;
using System.Net;

public class ServerSession : Session
{
    public uint playerId;

    public void Send(IMessage packet)
    {
        string msgName = packet.Descriptor.Name.Replace("_", string.Empty);
        PacketID msgId = Util.MessageToID(msgName);
        //Debug.Log($"msgId = {msgId}");
        ushort size = (ushort)packet.CalculateSize();
        byte[] sendBuffer = new byte[size + 4];
        Array.Copy(BitConverter.GetBytes((ushort)(size + 4)), 0, sendBuffer, 0, sizeof(ushort));
        Array.Copy(BitConverter.GetBytes((ushort)msgId), 0, sendBuffer, 2, sizeof(ushort));
        Array.Copy(packet.ToByteArray(), 0, sendBuffer, 4, size);
        Send(new ArraySegment<byte>(sendBuffer));
    }

    public override void OnConnected(EndPoint endPoint)
    {
        Debug.Log($"OnConnected: {endPoint}");
        Util._networkState = Util.NetworkState.CONNECTED;

        PacketManager.Instance.CustomHandler = (s, m, i) =>
        {
            PacketQueue.Instance.Push(i, m);
        };
    }

    public override void OnDisconnected(EndPoint endPoint)
    {
        Debug.Log($"OnDisconnected : {endPoint}");
    }

    public override int OnRecv(ArraySegment<byte> buffer)
    {
        int processLen = 0;

        while(true)
        {
            if (buffer.Count < sizeof(ushort))
                break;

            ushort dataSize = BitConverter.ToUInt16(buffer.Array, buffer.Offset);
            if (buffer.Count < dataSize)
                break;

            PacketManager.Instance.OnRecvPacket(this, new ArraySegment<byte>(buffer.Array, buffer.Offset, dataSize));
            processLen += dataSize;
            buffer = new ArraySegment<byte>(buffer.Array, buffer.Offset + dataSize, buffer.Count - dataSize);
        }

        return processLen;
    }

    public override void OnSend(int numOfBytes)
    {
    }
}

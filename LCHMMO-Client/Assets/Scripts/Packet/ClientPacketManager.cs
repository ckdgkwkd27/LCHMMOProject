using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Google.Protobuf;
using Unity.VisualScripting;
using UnityEngine.XR;

public enum PacketID
{
    PKT_CS_JOIN = 1,
    PKT_SC_JOIN,
    PKT_CS_LOGIN,
    PKT_SC_LOGIN,
    PKT_CS_ENTER_GAME,
    PKT_SC_ENTER_GAME,
    PKT_SC_SPAWN,
    PKT_CS_MOVE,
    PKT_SC_MOVE,
    PKT_SC_SET_HP,
    PKT_CS_SKILL,
    PKT_SC_SKILL,
    PKT_SC_DIE,
    PKT_SC_DESPAWN,
    PKT_CS_TELEPORT,
    PKT_SC_TELEPORT,
    PKT_S_VIEWPORT_UDPATE,
    PKT_CS_INIT_CLIENT,

    PKT_ERROR
};

class PacketManager
{
    static PacketManager _instance = new PacketManager();
    public static PacketManager Instance { get { return _instance; } }

    PacketManager()
    {
        Register();
    }

    Dictionary<ushort, Action<ServerSession, ArraySegment<byte>, ushort>> _onRecv = new Dictionary<ushort, Action<ServerSession, ArraySegment<byte>, ushort>>();
    Dictionary<ushort, Action<ServerSession, IMessage>> _handler = new Dictionary<ushort, Action<ServerSession, IMessage>>();
    public Action<ServerSession, IMessage, ushort> CustomHandler { get; set; }

    public void Register()
    {
        _onRecv.Add((ushort)PacketID.PKT_SC_JOIN, MakePacket<Protocol.ReturnJoin>);
        _handler.Add((ushort)PacketID.PKT_SC_JOIN, PacketHandler.HandleReturnJoin);
        _onRecv.Add((ushort)PacketID.PKT_SC_LOGIN, MakePacket<Protocol.ReturnLogin>);
        _handler.Add((ushort)PacketID.PKT_SC_LOGIN, PacketHandler.HandleReturnLogin);
        _onRecv.Add((ushort)PacketID.PKT_SC_ENTER_GAME, MakePacket<Protocol.ReturnEnterGame>);
        _handler.Add((ushort)(PacketID.PKT_SC_ENTER_GAME), PacketHandler.HandleReturnEnterGame);
        _onRecv.Add((ushort)PacketID.PKT_SC_SPAWN, MakePacket<Protocol.NotifySpawn>);
        _handler.Add((ushort)(PacketID.PKT_SC_SPAWN), PacketHandler.HandleNotifySpawn);
        _onRecv.Add((ushort)(PacketID.PKT_SC_MOVE), MakePacket<Protocol.ReturnMove>);
        _handler.Add((ushort)(PacketID.PKT_SC_MOVE), PacketHandler.HandleReturnMove);
        _onRecv.Add((ushort)(PacketID.PKT_SC_SET_HP), MakePacket<Protocol.NotifySetHp>);
        _handler.Add((ushort)(PacketID.PKT_SC_SET_HP), PacketHandler.HandleNotifySetHp);
        _onRecv.Add((ushort)(PacketID.PKT_SC_SKILL), MakePacket<Protocol.ReturnSkill>);
        _handler.Add((ushort)(PacketID.PKT_SC_SKILL), PacketHandler.HandleReturnSkill);
        _onRecv.Add((ushort)(PacketID.PKT_SC_DIE), MakePacket<Protocol.NotifyDie>);
        _handler.Add((ushort)(PacketID.PKT_SC_DIE), PacketHandler.HandleNotifyDie);
        _onRecv.Add((ushort)(PacketID.PKT_SC_DESPAWN), MakePacket<Protocol.NotifyDespawn>);
        _handler.Add((ushort)(PacketID.PKT_SC_DESPAWN), PacketHandler.HandleNotifyDespawn);
    }

    public void OnRecvPacket(ServerSession session, ArraySegment<byte> buffer)
    {
        ushort count = 0;

        ushort size = BitConverter.ToUInt16(buffer.Array, buffer.Offset);
        count += 2;
        ushort id = BitConverter.ToUInt16(buffer.Array, buffer.Offset + count);
        count += 2;
        //UnityEngine.Debug.Log($"PacketID={id}");
        Action<ServerSession, ArraySegment<byte>, ushort> action = null;
        if (_onRecv.TryGetValue(id, out action))
            action.Invoke(session, buffer, id);
    }

    void MakePacket<T>(ServerSession session, ArraySegment<byte> buffer, ushort id) where T : IMessage, new()
    {
        T pkt = new T();
        pkt.MergeFrom(buffer.Array, buffer.Offset + 4, buffer.Count - 4);

        if(CustomHandler != null)
        {
            CustomHandler.Invoke(session, pkt, id);
        }

        else
        {
            Action<ServerSession, IMessage> action = null;
            if(_handler.TryGetValue(id, out action))
            {
                action.Invoke(session, pkt);
            }
        }
    }

    public Action<ServerSession, IMessage> GetPacketHandler(ushort id)
    {
        Action<ServerSession, IMessage> action = null;
        if (_handler.TryGetValue(id, out action))
            return action;
        return null;
    }
}
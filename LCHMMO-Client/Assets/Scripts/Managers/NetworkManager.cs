using System.Collections;
using System.Collections.Generic;
using Google.Protobuf;
using System.Net;
using ServerCore;
using System;
using Unity.VisualScripting;

public class NetworkManager
{
    ServerSession _session = new ServerSession();

    public void Send(IMessage packet)
    {
        _session.Send(packet);
    }

    public void Init()
    {
        string host = Dns.GetHostName();
        IPAddress ipAddr = IPAddress.Parse("127.0.0.1");
        IPEndPoint endPoint = new IPEndPoint(ipAddr, 8888);
        UnityEngine.Debug.Log($"IP: {ipAddr}");

        Connector connector = new Connector();
        connector.Connect(endPoint, () => { return _session; }, 1);
    }

    public void Update()
    {
        List<PacketMessage> list = PacketQueue.Instance.PopAll();
        foreach(PacketMessage packet in list)
        {
            Action<ServerSession, IMessage> handler = PacketManager.Instance.GetPacketHandler(packet.Id);
            if (handler != null)
            {
                handler.Invoke(_session, packet.Message);
            }
        }
    }

}

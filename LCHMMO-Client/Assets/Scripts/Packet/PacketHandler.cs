using Google.Protobuf;
using Protocol;
using System;
using UnityEngine;
using UnityEngine.SceneManagement;

class PacketHandler
{
    public static void HandleReturnJoin(ServerSession session, IMessage packet)
    {
        ReturnJoin returnJoin = packet as ReturnJoin;
        if (returnJoin.Result == 0)
        {
            Debug.Log($"Join Success!!");
        }
        else
            Debug.LogError($"Join Failed!!");
    }

    public static void HandleReturnLogin(ServerSession session, IMessage packet)
    {
        ReturnLogin returnLogin = packet as ReturnLogin;
        if (returnLogin.Result == 0)
        {
            session.playerId = returnLogin.PlayerId;
            Util._networkState = Util.NetworkState.LOGIN;
            Debug.Log($"Login Success!! playerId={session.playerId}");

            SceneManager.LoadScene("StartField");

            RequestEnterGame enterGame = new RequestEnterGame();
            enterGame.PlayerId = session.playerId;
            enterGame.ZoneId = 0;
            Managers.Network.Send(enterGame);
        }
        else
            Debug.LogError($"Login Failed!!");
    }

    public static void HandleReturnEnterGame(ServerSession session, IMessage packet) 
    {
        ReturnEnterGame returnEnterGame = packet as ReturnEnterGame;

        Debug.Log($"EnterGame Success!, ActorID={returnEnterGame.MyPlayer.ActorId}, ZoneID={returnEnterGame.ZoneId}, PosX={returnEnterGame.MyPlayer.PosInfo.PosX}," +
            $"PosY={returnEnterGame.MyPlayer.PosInfo.PosX}");

        Managers.Map.LoadMap(returnEnterGame.ZoneId);
        Managers.Object.Add(returnEnterGame.MyPlayer, true);
    }

    public static void HandleNotifySpawn(ServerSession session, IMessage packet)
    {
        NotifySpawn spawnPacket = packet as NotifySpawn;
        Debug.Log($"SpawnPacket Size={spawnPacket.Objects.Capacity}");
        foreach (ObjectInfo obj in spawnPacket.Objects)
        {
            Managers.Object.Add(obj, false);
        }
    }

    public static void HandleReturnMove(ServerSession session, IMessage packet)
    {
        ReturnMove movePacket = packet as ReturnMove;

        GameObject go = Managers.Object.FindById(movePacket.ActorId);
        if (go == null)
            return;

        if (Managers.Object.MyPlayer.Id == movePacket.ActorId)
            return;

        BaseController bc = go.GetComponent<BaseController>();
        if(bc == null)
            return;

        bc.PosInfo = movePacket.PosInfo;
        Debug.Log("END: 이동 처리 완료");
    }

    public static void HandleNotifySetHp(ServerSession session, IMessage message)
    {
        NotifySetHp setHpPacket = message as NotifySetHp;

        GameObject go = Managers.Object.FindById(setHpPacket.ActorId);
        if (go == null) return;

        CreatureController cc = go.GetComponent<CreatureController>();
        if(cc != null)
        {
            cc.Hp = setHpPacket.Hp;
        }

        //Debug.Log($"SetHpPacket HP to {setHpPacket.Hp}");
    }

    public static void HandleReturnSkill(ServerSession session, IMessage message)
    {
        ReturnSkill skillPacket = message as ReturnSkill;

        GameObject go = Managers.Object.FindById(skillPacket.ActorId);
        if (go == null)
            return;

        CreatureController cc = go.GetComponent<CreatureController>();
        if (cc != null)
        {
            cc.UseSkill(skillPacket.SkillId);
        }

        Debug.Log($"Skill ActorId={skillPacket.ActorId}, State={cc.State}");
    }

    public static void HandleNotifyDie(ServerSession session, IMessage message)
    {
        NotifyDie diePacket = message as NotifyDie;

        GameObject go = Managers.Object.FindById(diePacket.ActorId);
        if(go == null) 
            return;

        CreatureController cc = go.GetComponent<CreatureController>();
        if (cc != null)
        {
            cc.Hp = 0;
            cc.OnDead();
        }

        Debug.Log($"Die Actor={diePacket.ActorId}");
    }

    public static void HandleNotifyDespawn(ServerSession session, IMessage message)
    {
        NotifyDespawn desPacket = message as NotifyDespawn;
        foreach (uint id in desPacket.ActorIds)
        {
            Debug.Log($"Despawn Actor={id}");
            Managers.Object.Remove(id);
        }
    }
}

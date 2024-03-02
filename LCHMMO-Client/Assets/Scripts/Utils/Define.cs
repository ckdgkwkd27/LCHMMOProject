using System;

public class Define
{
    public enum Scene
    {
        Unknown,
        Login,
        Lobby,
        Game,
    }

    public enum MouseEvent
    {
        PRESSED,
        RELEASED,
    }
}

public enum MsgId
{
    S_ENTER_GAME = 0,
    S_LEAVE_GAME = 1,
    S_SPAWN = 2,
    S_DESPAWN = 3,
    C_MOVE = 4,
    S_MOVE = 5,
    C_SKILL = 6,
    S_SKILL = 7,
    S_CHANGE_HP = 8,
    S_DIE = 9,
}

public enum CreatureState : ushort
{
    IDLE = 0,
    MOVING,
    CHASING,
    SKILL,
    DEAD,
}

public enum MoveDirType : ushort
{
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
}

public enum GameObjectType : ushort
{
    NONE = 0,
    PLAYER = 1,
    MONSTER = 2,
    PROJECTILE = 3,
}

public enum SkillType : ushort
{
    SKILL_NONE = 0,
    SKILL_AUTO = 1,
    SKILL_PROJECTILE = 2,
}


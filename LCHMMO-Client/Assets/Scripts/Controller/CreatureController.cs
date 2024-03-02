using UnityEngine;
using static Define;

public class CreatureController : BaseController
{
    public override Protocol.StatInfo Stat
    {
        get { return base.Stat; }
        set { base.Stat = value; }
    }

    public override uint Hp
    {
        get { return Stat.Hp; }
        set { base.Hp = value; }
    }

    protected override void Init()
    {
        base.Init();
    }

    public virtual void OnDamaged()
    {

    }

    public virtual void OnDead()
    {
        State = CreatureState.DEAD;
    }

    public virtual void UseSkill(uint skillId)
    {
        State = CreatureState.SKILL;
    }
}
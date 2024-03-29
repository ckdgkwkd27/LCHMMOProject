﻿using System.Collections;
using UnityEngine;
using static Define;

public class PlayerController : CreatureController
{
    protected Coroutine _coSkill;
    protected bool _rangedSkill = false;
    public int _playerID;

    protected override void Init()
    {
        base.Init();
    }

    protected override void UpdateAnimation()
    {
        if(_animator == null)
            Debug.LogError("Animator NULL VALUE");

        if (_animator == null || _sprite == null)
            return;
        if (State == CreatureState.IDLE)
        {
            switch (Dir)
            {
                case MoveDirType.UP:
                    _animator.Play("Idle");
                    break;
                case MoveDirType.DOWN:
                    _animator.Play("Idle");
                    break;
                case MoveDirType.LEFT:
                    _animator.Play("Idle");
                    _sprite.flipX = false;
                    break;
                case MoveDirType.RIGHT:
                    _animator.Play("Idle");
                    _sprite.flipX = true;
                    break;
            }
        }
        else if (State == CreatureState.MOVING)
        {
            switch (Dir)
            {
                case MoveDirType.UP:
                    _animator.Play("Run");
                    break;
                case MoveDirType.DOWN:
                    _animator.Play("Run");
                    break;
                case MoveDirType.LEFT:
                    _animator.Play("Run");
                    _sprite.flipX = false;
                    break;
                case MoveDirType.RIGHT:
                    _animator.Play("Run");
                    _sprite.flipX = true;
                    break;
            }
        }
        else if (State == CreatureState.CHASING)
        {
            switch (Dir)
            {
                case MoveDirType.UP:
                    _animator.Play("Run");
                    break;
                case MoveDirType.DOWN:
                    _animator.Play("Run");
                    break;
                case MoveDirType.LEFT:
                    _animator.Play("Run");
                    _sprite.flipX = false;
                    break;
                case MoveDirType.RIGHT:
                    _animator.Play("Run");
                    _sprite.flipX = true;
                    break;
            }
        }
        else if (State == CreatureState.SKILL)
        {
            switch (Dir)
            {
                case MoveDirType.UP:
                    _animator.Play("SwordAttack1");
                    break;
                case MoveDirType.DOWN:
                    _animator.Play("SwordAttack1");
                    break;
                case MoveDirType.LEFT:
                    _animator.Play("SwordAttack1");
                    _sprite.flipX = false;
                    break;
                case MoveDirType.RIGHT:
                    _animator.Play("SwordAttack1");
                    _sprite.flipX = true;
                    break;
            }
        }
        else if(State == CreatureState.DEAD)
        {
            _animator.Play("Death");
        }
        else
        {
            Debug.LogError($"Player State FAILURE! State={State}");
        }
    }

    protected override void UpdateController()
    {
        base.UpdateController();
    }

    public override void UseSkill(uint skillId)
    {
        Debug.Log("Use Skill");
        if(skillId == 2)
        {
            _coSkill = StartCoroutine("CoStartMelee");
            Debug.Log("StartMelee!");
        }
        else if(skillId == 3)
        {
            _coSkill = StartCoroutine("CoStartShootArrow");
        }
    }

    protected virtual void CheckUpdatedFlag()
    {

    }

    IEnumerator CoStartMelee()
    {
        _rangedSkill = false;
        State = CreatureState.SKILL;
        yield return new WaitForSeconds(0.5f);
        State = CreatureState.IDLE;
        _coSkill = null;
        CheckUpdatedFlag();
    }

    IEnumerator CoStartShootArrow()
    {
        _rangedSkill = true;
        State = CreatureState.SKILL;
        yield return new WaitForSeconds(0.3f);
        State = CreatureState.IDLE;
        _coSkill = null;
        CheckUpdatedFlag();
    }

    public override void OnDamaged()
    {
        Debug.Log("Player HIT!");
    }
}
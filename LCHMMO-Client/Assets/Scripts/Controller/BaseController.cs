using UnityEngine;
using Google.Protobuf;
using static Define;

public class BaseController : MonoBehaviour
{
    public uint Id;
    Protocol.StatInfo _stat = new Protocol.StatInfo();
    public virtual Protocol.StatInfo Stat
    {
        get { return _stat; }
        set
        {
            if (_stat.Equals(value))
                return;

            _stat.Hp = value.Hp;
            _stat.MaxHp = value.MaxHp;
            _stat.Speed = value.Speed;
        }
    }

    public float Speed
    {
        get { return Stat.Speed; }
        set { Stat.Speed = value; }
    }

    public virtual uint Hp
    {
        get { return Stat.Hp; }
        set
        {
            Stat.Hp = value;
        }
    }

    protected bool _updated = false;
    Protocol.PositionInfo _positionInfo = new Protocol.PositionInfo();
    public Protocol.PositionInfo PosInfo
    {
        get { return _positionInfo; }
        set
        {
            if (_positionInfo.Equals(value))
                return;

            CellPos = new Vector3Int(value.PosX, value.PosY, 0);
            State = (CreatureState)value.State;
            Dir = (MoveDirType)value.MoveDir;
        }
    }

    public void SyncPos()
    {
        Vector3 destPos = Managers.Map.CurrentGrid.CellToWorld(CellPos);
        transform.position = destPos;
    }

    public Vector3Int CellPos
    {
        get
        {
            return new Vector3Int(PosInfo.PosX, PosInfo.PosY, 0);
        }

        set
        {
            if (PosInfo.PosX == value.x && PosInfo.PosY == value.y)
                return;

            PosInfo.PosX = value.x;
            PosInfo.PosY = value.y;
            _updated = true;
        }
    }

    protected Animator _animator;
    protected SpriteRenderer _sprite;

    public virtual CreatureState State
    {
        get { return (CreatureState)PosInfo.State; }
        set
        {
            if ((CreatureState)PosInfo.State == value)
                return;

            PosInfo.State = (uint)value;
            UpdateAnimation();
            _updated = true;
        }
    }

    public MoveDirType Dir
    {
        get { return (MoveDirType)PosInfo.MoveDir; }
        set
        {
            if ((MoveDirType)PosInfo.MoveDir == value)
                return;

            PosInfo.MoveDir = (uint)value;

            UpdateAnimation();
            _updated = true;
        }
    }

    public MoveDirType GetDirFromVec(Vector3Int dir)
    {
        if (dir.x > 0)
            return MoveDirType.RIGHT;
        else if (dir.x < 0)
            return MoveDirType.LEFT;
        else if (dir.y > 0)
            return MoveDirType.UP;
        else
            return MoveDirType.DOWN;
    }

    public Vector3Int GetFrontCellPos()
    {
        Vector3Int cellPos = CellPos;

        switch (Dir)
        {
            case MoveDirType.UP:
                cellPos += Vector3Int.up;
                break;
            case MoveDirType.DOWN:
                cellPos += Vector3Int.down;
                break;
            case MoveDirType.LEFT:
                cellPos += Vector3Int.left;
                break;
            case MoveDirType.RIGHT:
                cellPos += Vector3Int.right;
                break;
        }

        return cellPos;
    }

    protected virtual void UpdateAnimation()
    {
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
                    _animator.Play("Attack");
                    break;
                case MoveDirType.DOWN:
                    _animator.Play("Attack");
                    break;
                case MoveDirType.LEFT:
                    _animator.Play("Attack");
                    _sprite.flipX = false;
                    break;
                case MoveDirType.RIGHT:
                    _animator.Play("Attack");
                    _sprite.flipX = true;
                    break;
            }
        }
        else if (State == CreatureState.DEAD)
        {
            switch (Dir)
            {
                case MoveDirType.UP:
                    _animator.Play("Death");
                    break;
                case MoveDirType.DOWN:
                    _animator.Play("Death");
                    break;
                case MoveDirType.LEFT:
                    _animator.Play("Death");
                    _sprite.flipX = false;
                    break;
                case MoveDirType.RIGHT:
                    _animator.Play("Death");
                    _sprite.flipX = true;
                    break;
            }
        }
        else
        {
            Debug.LogError($"STATE CRITICAL ERROR! State={State}");
        }
    }

    void Start()
    {
        Init();
    }

    protected virtual void Update()
    {
        UpdateController();
    }

    protected virtual void Init()
    {
        _animator = GetComponent<Animator>();
        _sprite = GetComponent<SpriteRenderer>();
        Vector3 pos = Managers.Map.CurrentGrid.CellToWorld(CellPos);
        transform.position = pos;

        UpdateAnimation();
    }

    protected virtual void UpdateController()
    {
        switch (State)
        {
            case CreatureState.IDLE:
                UpdateIdle();
                break;
            case CreatureState.MOVING:
                UpdateMoving();
                break;
            case CreatureState.CHASING:
                UpdateChasing();
                break;
            case CreatureState.SKILL:
                UpdateSkill();
                break;
            case CreatureState.DEAD:
                UpdateDead();
                break;
        }
    }

    protected virtual void UpdateIdle()
    {
    }

    protected virtual void UpdateMoving()
    {
        Vector3 destPos = Managers.Map.CurrentGrid.CellToWorld(CellPos);
        Vector3 moveDir = destPos - transform.position;

        // 도착 여부 체크
        float dist = moveDir.magnitude;
        if (dist < Speed * Time.deltaTime)
        {
            transform.position = destPos;
            MoveToNextPos();
        }
        else
        {
            transform.position += moveDir.normalized * Speed * Time.deltaTime;
            State = CreatureState.MOVING;
        }
    }

    protected virtual void UpdateChasing()
    {
        Vector3 destPos = Managers.Map.CurrentGrid.CellToWorld(CellPos);
        Vector3 moveDir = destPos - transform.position;

        // 도착 여부 체크
        float dist = moveDir.magnitude;
        if (dist < Speed * Time.deltaTime)
        {
            transform.position = destPos;
            MoveToNextPos();
        }
        else
        {
            transform.position += moveDir.normalized * Speed * Time.deltaTime;
            State = CreatureState.CHASING;
        }
    }

    protected virtual void MoveToNextPos()
    {

    }

    protected virtual void UpdateSkill()
    {

    }

    protected virtual void UpdateDead()
    {

    }
}
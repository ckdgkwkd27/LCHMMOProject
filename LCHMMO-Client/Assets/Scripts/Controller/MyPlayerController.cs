using System;
using System.Collections;
using Unity.VisualScripting;
using UnityEngine;

public class MyPlayerController : PlayerController
{
    bool _moveKeyPressed = false, _moveUpdateReserved = false;
    MonsterController targetActor = null;
    public const float MeleeDistance = 5.0f;

    protected override void Init()
    {
        base.Init();
    }

    const float MOVE_PACKET_SEND_DELAY = 0.2f;
    float MovePacketSendTimer = MOVE_PACKET_SEND_DELAY;
    Vector2 DesiredInput, LastDesiredInput;

    protected override void Update()
    {
        base.Update();

        bool ForceSendPacket = false;
        if(LastDesiredInput != DesiredInput)
        {
            ForceSendPacket = true;
            LastDesiredInput = DesiredInput;
        }

        if(DesiredInput == Vector2.zero)
        {
            if(_moveUpdateReserved == false)
                PosInfo.State = (uint)CreatureState.IDLE;
        }
        else
        {
            PosInfo.State = (uint)CreatureState.MOVING;
        }

        MovePacketSendTimer -= Time.deltaTime;

        if (MovePacketSendTimer <= 0 || ForceSendPacket)
        {
            MovePacketSendTimer = MOVE_PACKET_SEND_DELAY;
            Protocol.RequestMove movePacket = new Protocol.RequestMove();
            movePacket.ActorId = Id;
            movePacket.PosInfo = PosInfo;
            Managers.Network.Send(movePacket);
        }

        DesiredInput = Vector2.zero;
    }

    protected override void UpdateController()
    {
        switch(State)
        {
            case CreatureState.IDLE:
                GetDirInput();
                GetMouseInput();
                break;
            case CreatureState.MOVING:
                GetDirInput();
                GetMouseInput();
                break;
        }

        base.UpdateController();
    }

    protected override void UpdateIdle()
    {
        if (_moveKeyPressed)
        {
            State = CreatureState.MOVING;
            _moveUpdateReserved = true;
            return;
        }

        if(_coSkillCooltime == null && Input.GetKey(KeyCode.Space))
        {
            if (targetActor == null)
                return;

            if (Vector3.Distance(transform.position, targetActor.transform.position) > MeleeDistance)
                return;

            Protocol.RequestSkill skillPacket = new Protocol.RequestSkill();
            skillPacket.SkillId = 2;
            skillPacket.TargetActorId = targetActor.Id;
            Managers.Network.Send(skillPacket);
            Debug.Log("Send Skill");

            _coSkillCooltime = StartCoroutine("CoInputCooltime", 0.2f);
        }
    }

    protected override void UpdateMoving()
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

        //cache
        DesiredInput = destPos;
        _moveUpdateReserved = false;

        if(Input.GetKey(KeyCode.Space))
        {
            if (_coSkillCooltime == null)
            {
                if (targetActor == null)
                    return;

                if (Vector3.Distance(transform.position, targetActor.transform.position) > MeleeDistance)
                    return;

                Protocol.RequestSkill skillPacket = new Protocol.RequestSkill();
                skillPacket.SkillId = 2;
                skillPacket.TargetActorId = targetActor.Id;
                Managers.Network.Send(skillPacket);
                Debug.Log("Send Skill");

                _coSkillCooltime = StartCoroutine("CoInputCooltime", 0.2f);
            }
        }
    }

    Coroutine _coSkillCooltime;
    IEnumerator CoInputCooltime(float time)
    {
        yield return new WaitForSeconds(time);
        _coSkillCooltime = null;
    }

    void LateUpdate()
    {
        Camera.main.transform.position = new Vector3(transform.position.x, transform.position.y, -10);
    }

    // 키보드 입력
    void GetDirInput()
    {
        _moveKeyPressed = true;

        if (Input.GetKey(KeyCode.W))
        {
            Dir = MoveDirType.UP;
        }
        else if (Input.GetKey(KeyCode.S))
        {
            Dir = MoveDirType.DOWN;
        }
        else if (Input.GetKey(KeyCode.A))
        {
            Dir = MoveDirType.LEFT;
        }
        else if (Input.GetKey(KeyCode.D))
        {
            Dir = MoveDirType.RIGHT;
        }
        else
        {
            _moveKeyPressed = false;
        }
    }

    void GetMouseInput()
    {
        if (Input.GetMouseButtonDown(0))
        {
            Camera camera = FindAnyObjectByType<Camera>();
            Vector3 inputPos = Input.mousePosition;
            inputPos.z = 10.0f;
            Vector3 mousePos = camera.ScreenToWorldPoint(inputPos);

            RaycastHit2D hit = Physics2D.Raycast(mousePos, Vector2.zero);
            if(hit)
            {
                if (hit.transform.gameObject.layer == LayerMask.NameToLayer("Monster"))
                {
                    targetActor = hit.transform.gameObject.GetComponent<MonsterController>();
                    targetActor.transform.Find("BlobShadow").GetComponent<SpriteRenderer>().enabled = true;
                }
            }

            else
            {
                if(targetActor != null)
                    targetActor.transform.Find("BlobShadow").GetComponent<SpriteRenderer>().enabled = false;
                targetActor = null;
            }
        }
    }

    protected override void MoveToNextPos()
    {
        if (_moveKeyPressed == false)
        {
            State = CreatureState.IDLE;
            return;
        }

        Vector3Int destPos = CellPos;

        switch (Dir)
        {
            case MoveDirType.UP:
                destPos += Vector3Int.up;
                break;
            case MoveDirType.DOWN:
                destPos += Vector3Int.down;
                break;
            case MoveDirType.LEFT:
                destPos += Vector3Int.left;
                break;
            case MoveDirType.RIGHT:
                destPos += Vector3Int.right;
                break;
        }

        if (Managers.Map.CanGo(destPos))
        {
            if (Managers.Object.FindCreature(destPos) == null)
            {
                CellPos = destPos;
            }
        }
    }
}

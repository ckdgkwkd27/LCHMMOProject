using System;
using System.Collections;
using Unity.VisualScripting;
using UnityEngine;

public class MyPlayerController : PlayerController
{
    bool _moveKeyPressed = false;
    MonsterController targetActor = null;
    public const float MeleeDistance = 5.0f;

    protected override void Init()
    {
        base.Init();
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
        if(_moveKeyPressed)
        {
            State = CreatureState.MOVING;
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
            CheckUpdatedFlag();
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

        CheckUpdatedFlag();
    }

    protected override void CheckUpdatedFlag()
    {
        if (_updated)
        {
            Protocol.RequestMove movePacket = new Protocol.RequestMove();
            movePacket.PosInfo = PosInfo;
            Managers.Network.Send(movePacket);
            _updated = false;

            Debug.Log("START: 이동 처리 시작");
        }
    }
}

using UnityEngine;

public class LoginScene : BaseScene
{
    UI_Login ui_Login;
    protected override void Init()
    {
        base.Init();
        Screen.SetResolution(960, 720, false);
    }

    public override void Clear()
    {

    }
}
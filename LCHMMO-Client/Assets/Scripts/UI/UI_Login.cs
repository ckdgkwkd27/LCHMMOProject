using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Events;
using TMPro;
using Protocol;
using ServerCore;

public class UI_Login : MonoBehaviour
{
    public TMP_InputField idField;
    public TMP_InputField pwField;
    public Button joinButton;
    public Button loginButton;
    public Button playButton;
    public TMP_Text StateText;

    public void Start()
    {
        var canvas = GameObject.Find("Canvas");
        idField = canvas.transform.Find("IdInput").GetComponent<TMP_InputField>();
        pwField = canvas.transform.Find("PwInput").GetComponent<TMP_InputField>();
        //StateText = canvas.transform.Find("StateText").GetComponent<TMP_Text>();
    }

    public void OnJoinButtonClick()
    {
        string id = idField.text;
        string pw = pwField.text;
        Debug.Log($"INFO: Join Id={id}, pw={pw}");

        RequestJoin requestJoin = new RequestJoin();
        requestJoin.Id = id;
        requestJoin.Password = pw;
        Managers.Network.Send(requestJoin);
    }

    public void OnLoginButtonClick()
    {
        string id = idField.text;
        string pw = pwField.text;
        Debug.Log($"INFO: Login Id={id}, pw={pw}");

        RequestLogin requestLogin = new RequestLogin();
        requestLogin.Id = id;
        requestLogin.Password = pw;
        Managers.Network.Send(requestLogin);
    }
}
using UnityEngine;
using UnityEngine.EventSystems;

public enum SceneType
{
    UNKNOWN,
    LOGIN,
    WORLD
}

public abstract class BaseScene : MonoBehaviour
{
    public SceneType sceneType { get; protected set; } = SceneType.UNKNOWN;
    void Awake()
    {
        Init();
    }

    protected virtual void Init()
    {
        Object obj = GameObject.FindObjectOfType(typeof(EventSystem));
        if(obj == null)
        {
        }
    }

    public abstract void Clear();
}
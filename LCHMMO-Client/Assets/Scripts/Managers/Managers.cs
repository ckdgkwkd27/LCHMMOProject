using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Unity.VisualScripting;
using UnityEngine;

public class Managers : MonoBehaviour
{
    static Managers s_instance;
    static Managers Instance { get { Init(); return s_instance; } }

    MapManager _map = new MapManager();
    NetworkManager _network = new NetworkManager();
    UIManager _ui = new UIManager();
    ResourceManager _resource = new ResourceManager();
    ObjectManager _object = new ObjectManager();
    PoolManager _pool = new PoolManager();

    public static NetworkManager Network { get { return Instance._network; } }
    public static UIManager UI { get { return Instance._ui; } }
    public static ResourceManager Resource { get { return Instance._resource; } }
    public static MapManager Map { get { return Instance._map; } }
    public static ObjectManager Object { get { return Instance._object; } }
    public static PoolManager Pool { get { return Instance._pool; } }

    void Start()
    {
        Init();
    }

    private void Update()
    {
        _network.Update();
    }

    static void Init()
    {
        if(s_instance == null)
        {
            Debug.Log("Manager Initialize!");

            GameObject go = GameObject.Find("@Managers");
            if (go == null)
            {
                go = new GameObject { name = "@Managers" };
                go.AddComponent<Managers>();
            }

            DontDestroyOnLoad(go);
            s_instance = go.GetComponent<Managers>();

            s_instance._network.Init();
            s_instance._pool.Init();
        }
    }

    public static void Clear()
    {
        Pool.Clear();
    }
}
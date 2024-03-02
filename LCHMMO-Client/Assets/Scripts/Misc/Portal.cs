using Protocol;
using UnityEngine;

public class Portal : MonoBehaviour
{
    public uint zoneId;

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    private void OnTriggerEnter2D(Collider2D collision)
    {
        //Collision Box를 Player한테 둘러야함.
        PlayerController pc = collision.gameObject.GetComponent<PlayerController>();
        if (pc != null)
        {
            Managers.Object.Clear();

            Debug.Log("On Collision Portal!");
            Protocol.RequestTeleport packet = new Protocol.RequestTeleport();
            packet.ActorId = pc.Id;
            packet.ZoneId = zoneId;
            Managers.Network.Send(packet);
        }
    }
}

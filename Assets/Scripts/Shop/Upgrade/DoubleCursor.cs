using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DoubleCursor : MonoBehaviour, IUpgrade
{
    public void ApplyUpgrade()
    {
        //ƒJ[ƒ\ƒ‹‚Ì©“®¶¬”‚ğ2”{‚É‚·‚é
        //è“®¶¬”‚ğ2”{‚É‚·‚é
        PlayerManager.Instance.AddManualGenerateCount(PlayerManager.Instance.ManualGenerateCount);
    }
}

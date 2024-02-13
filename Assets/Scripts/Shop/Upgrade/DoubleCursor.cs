using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DoubleCursor : MonoBehaviour, IUpgrade
{
    public void ApplyUpgrade()
    {
        //ƒJ[ƒ\ƒ‹‚Ì©“®¶¬”‚ğ2”{‚É‚·‚é
        var cursorAutoGenerateInfo = PlayerManager.Instance.AutoGeneratorDictionary["Cursor"];
        var newfactoryInfo = new FactoryInfo("Cursor", cursorAutoGenerateInfo.BaseGeneratorValue, cursorAutoGenerateInfo.BaseScale + 1);
        cursorAutoGenerateInfo = newfactoryInfo;
        //è“®¶¬”‚ğ2”{‚É‚·‚é
        PlayerManager.Instance.ManualGenerateCount.AddScale(1);
    }
}

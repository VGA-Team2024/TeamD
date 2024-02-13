using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// アップグレードの基底クラス
/// </summary>
public class UpgradeBase : ShopItemBase
{
    public override void Buy()
    {
        this.GetComponent<IUpgrade>()?.ApplyUpgrade();
    }
}

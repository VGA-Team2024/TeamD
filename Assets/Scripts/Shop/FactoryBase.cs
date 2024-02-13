using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// 施設の基底クラス
/// </summary>
public class FactoryBase : ShopItemBase
{
    [SerializeField, Header("生成する数")] private LargeNumber _generateNum;
    const double _increaseRate = 1.15d;

    public override void Buy()
    {
        //プレイヤーの自動生成数を増やす
        PlayerManager.Instance.AddAutoGenerateCount(_generateNum);
    }

    public void IncreasePrice()
    {
        _price *= _increaseRate;
    }
}

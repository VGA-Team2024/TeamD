using UnityEngine;

/// <summary>
/// 施設の基底クラス
/// </summary>
public class FactoryBase : ShopItemBase
{
    [SerializeField, Header("生成する数")] private double _generateNum;
    private const double IncreaseRate = 1.15d;
    public double GenerateNum => _generateNum;

    public override void Buy(FactoryInfo factoryInfo)
    {
        //プレイヤーの自動生成数を増やす
        //PlayerManager.Instance.AddAutoGenerateCount(factoryInfo.Name, _generateNum);
    }

    public void Sell(FactoryInfo factoryInfo)
    {
        //プレイヤーの自動生成数を減らす
        //PlayerManager.Instance.SubtractGeneratorCount(factoryInfo.Name, _generateNum);
    }

    public void IncreasePrice()
    {
        _price *= IncreaseRate;
    }
    
    public void DecreasePrice()
    {
        _price /= IncreaseRate;
    }
}

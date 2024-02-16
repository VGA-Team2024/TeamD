using UnityEngine;

/// <summary>
/// ショップで販売するアイテムの基底クラス
/// 施設とアップグレードの基底クラス
/// </summary>
public abstract class ShopItemBase : MonoBehaviour
{
    [SerializeField, Header("名前")] private string _name;
    [SerializeField, Header("値段")] protected double _price;
    public double Price => _price;

    public abstract void Buy(FactoryInfo factoryInfo);
}

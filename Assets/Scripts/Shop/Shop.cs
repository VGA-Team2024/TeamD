using System.Collections.Generic;
using System.Linq;
using UniRx;
using UnityEngine;

/// <summary>
/// ショップのクラス
/// UIに施設とアップグレードボタンを表示し、押下時の処理を登録する。
/// </summary>
public class Shop : MonoBehaviour
{
    [SerializeField] private GameObject _shopItemButtonPrefab;
    [SerializeField] private Transform _factoryButtonParent;
    [SerializeField] Transform _upgradeButtonParent;
    [SerializeField, Tooltip("施設の値上げ倍率")] float _factoryMultiplier = 1.15f;

    [SerializeField, Tooltip("施設売却時の返金割合")]
    float _sellFactoryRatio = 2f / 3f;

    Factories _factories;
    Tiers _tiers;
    Upgrades _upgrades;

    List<ShopItemButton> _currentButtons = new();

    private void Start()
    {
        _factories = Resources.Load<Factories>("Excel/Factories");
        _tiers = Resources.Load<Tiers>("Excel/Tiers");
        _upgrades = Resources.Load<Upgrades>("Excel/Upgrades");
        
        foreach (var factory in _factories.Entities)
        {   
            var button = Instantiate(_shopItemButtonPrefab, _factoryButtonParent).GetComponent<ShopItemButton>();
            button.SetItemName(factory.Name, true);
            //  施設のベース価格 × 1.15^施設数
            button.SetPriceText(factory.BasePrice * Mathf.Pow(_factoryMultiplier, StatsManager.FactoryStats[factory.Key].Amount));
            //購入時の処理を登録する。
            button.OnLeftClickEvent += () =>
            {
                //  施設のベース価格 × 1.15^施設数
                var price = factory.BasePrice * Mathf.Pow(_factoryMultiplier, StatsManager.FactoryStats[factory.Key].Amount);
                if (price <= PlayerManager.Instance.CookieCount)
                {
                    PlayerManager.Instance.SubtractCookie(price);
                    var stat = StatsManager.FactoryStats[factory.Key];
                    stat.Amount++;
                    StatsManager.FactoryStats[factory.Key] = stat;
                    button.SetCurrentOwnNum(1); //  表示の数値を増やす
                    button.SetPriceText(price * _factoryMultiplier);    //  増えた分値段表示を更新
                }
            };

            //売却時の処理を登録する
            button.OnRightClickEvent += () =>
            {
                var price = factory.BasePrice * Mathf.Pow(_factoryMultiplier, StatsManager.FactoryStats[factory.Key].Amount);
                if (0 < StatsManager.FactoryStats[factory.Key].Amount)
                {
                    //  現在の施設価格 × 2 / 3のクッキーを追加する
                    PlayerManager.Instance.AddCookie(price * _sellFactoryRatio);
                    var stat = StatsManager.FactoryStats[factory.Key];
                    stat.Amount--;
                    StatsManager.FactoryStats[factory.Key] = stat;
                    button.SetCurrentOwnNum(-1);    //  表示の数値を減らす
                    button.SetPriceText(price / _factoryMultiplier); //  減った分値段表示を更新
                }
            };
        }
        StatsManager.FactoryStats.ObserveReplace().Subscribe(_=> GenerateUpgradeShop()).AddTo(this);
    }

    void GenerateUpgradeShop()
    {
        _currentButtons?.ForEach(b=>Destroy(b.gameObject));
        _currentButtons?.Clear();
        foreach (var pair in StatsManager.FactoryStats)
        {
            var topTier = _tiers.Entities.FirstOrDefault(x => x.Tier == pair.Value.Tier + 1);
            if(topTier == null) continue;
            if (topTier.Condition <= pair.Value.Amount)
            {
                var button = Instantiate(_shopItemButtonPrefab, _upgradeButtonParent).GetComponent<ShopItemButton>();
                _currentButtons.Add(button);
                var price = _factories.Entities.Find(x => x.Key == pair.Key).BasePrice * topTier.Multiplier;
                button.SetItemName(_upgrades.Entities.Find(x=>x.Key.Equals(pair.Key) && x.Tier.Equals(topTier.Tier)).Name, false);
                button.SetPriceText(price);
                button.OnLeftClickEvent += () =>
                {
                    if (price <= PlayerManager.Instance.CookieCount)
                    {
                        var factoryStat = StatsManager.FactoryStats[pair.Key];
                        factoryStat.Tier += 1;
                        StatsManager.FactoryStats[pair.Key] = factoryStat;
                        PlayerManager.Instance.SubtractCookie(price);
                        Destroy(button.gameObject);
                    }
                };
            }
        }
    }
}

using System.Collections.Generic;
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

    List<ShopItemButton> _currentButtons = new();

    private void Start()
    {
        _factories = Resources.Load<Factories>("Excel/Factories");
        
        foreach (var factory in _factories.Entities)
        {   
            var button = Instantiate(_shopItemButtonPrefab, _factoryButtonParent).GetComponent<ShopItemButton>();
            button.SetItemName(factory.Name, true);
            button.CurrentOwnNumText.text = StatsManager.CurrentFactories[factory.Key].Amount.ToString();
            //  施設のベース価格 × 1.15^施設数
            button.SetPriceText(factory.BasePrice * Mathf.Pow(_factoryMultiplier, StatsManager.CurrentFactories[factory.Key].Amount));
            //購入時の処理を登録する。
            button.OnLeftClickEvent += () =>
            {
                //  施設のベース価格 × 1.15^施設数
                var price = factory.BasePrice * Mathf.Pow(_factoryMultiplier, StatsManager.CurrentFactories[factory.Key].Amount);
                if (price <= PlayerManager.Instance.CookieCount)
                {
                    PlayerManager.Instance.SubtractCookie(price);
                    var stat = StatsManager.CurrentFactories[factory.Key];
                    stat.Amount++;
                    StatsManager.CurrentFactories[factory.Key] = stat;
                    button.CurrentOwnNumText.text = StatsManager.CurrentFactories[factory.Key].Amount.ToString();
                    button.SetPriceText(price * _factoryMultiplier);    //  増えた分値段表示を更新
                }
            };

            //売却時の処理を登録する
            button.OnRightClickEvent += () =>
            {
                var price = factory.BasePrice * Mathf.Pow(_factoryMultiplier, StatsManager.CurrentFactories[factory.Key].Amount);
                if (0 < StatsManager.CurrentFactories[factory.Key].Amount)
                {
                    //  現在の施設価格 × 2 / 3のクッキーを追加する
                    PlayerManager.Instance.AddCookie(price * _sellFactoryRatio);
                    var stat = StatsManager.CurrentFactories[factory.Key];
                    stat.Amount--;
                    StatsManager.CurrentFactories[factory.Key] = stat;
                    StatsManager.FactorySellCount[factory.Key] += 1;
                    button.CurrentOwnNumText.text = StatsManager.CurrentFactories[factory.Key].Amount.ToString();
                    button.SetPriceText(price / _factoryMultiplier); //  減った分値段表示を更新
                }
            };
        }
        StatsManager.OnUpdateNextUpgrades.Subscribe(GenerateUpgradeShop).AddTo(this);
        StatsManager.UpdateNextUpgrades();
    }

    void GenerateUpgradeShop(Dictionary<FactoriesEntity, (TiersEntity Tier, UpgradesEntity UpgradesInfo)> nextUpgrades)
    {
        _currentButtons?.ForEach(b=>Destroy(b.gameObject));
        _currentButtons?.Clear();
        foreach (var nextUpgrade in nextUpgrades)
        {
            var button = Instantiate(_shopItemButtonPrefab, _upgradeButtonParent).GetComponent<ShopItemButton>();
            _currentButtons.Add(button);
            var price = nextUpgrade.Key.BasePrice * nextUpgrade.Value.Tier.Multiplier;
            button.SetItemName(nextUpgrade.Value.UpgradesInfo.Name, false);
            button.SetPriceText(price);
            button.OnLeftClickEvent += () =>
            {
                if (price <= PlayerManager.Instance.CookieCount)
                {
                    var factoryStat = StatsManager.CurrentFactories[nextUpgrade.Key.Key];
                    factoryStat.Tier += 1;
                    StatsManager.CurrentFactories[nextUpgrade.Key.Key] = factoryStat;
                    PlayerManager.Instance.SubtractCookie(price);
                    Destroy(button.gameObject);
                }
            };
        }
    }
}

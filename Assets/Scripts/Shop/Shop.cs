using System.Collections;
using System.Collections.Generic;
using TeamD.Enum;
using UnityEngine;

/// <summary>
/// ショップのクラス
/// UIに施設とアップグレードボタンを表示し、押下時の処理を登録する。
/// </summary>
public class Shop : MonoBehaviour
{
    [SerializeField] private List<UpgradeBase> _upgrades;
    [SerializeField] private Factories _factories;
    [SerializeField] private GameObject _shopItemButtonPrefab;
    [SerializeField] private Transform _shopItemButtonParent;

    private void Start()
    {
        var reverseEntities = _factories.Entities;
        foreach (var factory in reverseEntities)
        {   
            var button = Instantiate(_shopItemButtonPrefab, _shopItemButtonParent).GetComponent<ShopItemButton>();
            button.SetItemName(factory.Name, true);
            button.SetPriceText(factory.BasePrice * Mathf.Pow(1.15f, StatsManager.FactoryStats[factory.Key].Amount));
            //購入時の処理を登録する。
            button.OnLeftClickEvent += () =>
            {
                var price = factory.BasePrice * Mathf.Pow(1.15f, StatsManager.FactoryStats[factory.Key].Amount);
                if (price <= PlayerManager.Instance.CookieCount)
                {
                    PlayerManager.Instance.SubtractCookie(price);
                    var stat = StatsManager.FactoryStats[factory.Key];
                    stat.Amount++;
                    StatsManager.FactoryStats[factory.Key] = stat;
                    button.SetCurrentOwnNum(1);
                    button.SetPriceText(factory.BasePrice * Mathf.Pow(1.15f, stat.Amount));
                    //プレイヤーへの処理
                    //・資源を減らす
                    //・自動生成数を増やす
                    //・所持数データを増やす
                }
            };

            //売却時の処理を登録する
            button.OnRightClickEvent += () =>
            {
                var price = factory.BasePrice * Mathf.Pow(1.15f, StatsManager.FactoryStats[factory.Key].Amount);
                if (0 < StatsManager.FactoryStats[factory.Key].Amount)
                {
                    PlayerManager.Instance.AddCookie(price * (2d / 3d));
                    var stat = StatsManager.FactoryStats[factory.Key];
                    stat.Amount--;
                    StatsManager.FactoryStats[factory.Key] = stat;
                    button.SetCurrentOwnNum(-1);
                    button.SetPriceText(factory.BasePrice * Mathf.Pow(1.15f, stat.Amount));
                }
            };
        }

        foreach (var upgrade in _upgrades)
        {
            var button = Instantiate(_shopItemButtonPrefab, _shopItemButtonParent).GetComponent<ShopItemButton>();
            button.SetItemName(upgrade.name, false);
            button.SetPriceText(upgrade.Price);
            var dummyFactoryInfo = new FactoryInfo(upgrade.name, upgrade.Price, 1);
            button.OnLeftClickEvent += () =>
            {
                if (upgrade.Price <= PlayerManager.Instance.CookieCount)
                {
                    //プレイヤーへの処理
                    //・資源を減らす
                    //・倍率を増やす
                    upgrade.Buy(dummyFactoryInfo);
                    PlayerManager.Instance.SubtractCookie(upgrade.Price);
                    Destroy(button.gameObject);
                }
            };
        }
    }
}

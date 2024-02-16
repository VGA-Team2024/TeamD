using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// ショップのクラス
/// UIに施設とアップグレードボタンを表示し、押下時の処理を登録する。
/// </summary>
public class Shop : MonoBehaviour
{
    [SerializeField] private List<UpgradeBase> _upgrades;
    [SerializeField] private List<FactoryBase> _factories;
    [SerializeField] private GameObject _shopItemButtonPrefab;
    [SerializeField] private Transform _shopItemButtonParent;

    private void Start()
    {
        int id = 0; //Playerに登録する際のID
        foreach (var factory in _factories)
        {   
            var button = Instantiate(_shopItemButtonPrefab, _shopItemButtonParent).GetComponent<ShopItemButton>();
            var factoryInfo = new FactoryInfo(factory.name, 0, 0);
            PlayerManager.Instance.AutoGeneratorDictionary.Add(factory.name, factoryInfo);
            button.SetItemName(factory.name, true);
            button.SetPriceText(factory.Price);
            //購入時の処理を登録する。
            button.OnLeftClickEvent += () =>
            {
                if (factory.Price <= PlayerManager.Instance.CookieCount)
                {
                    PlayerManager.Instance.SubtractCookie(factory.Price);
                    factory.Buy(factoryInfo);
                    factory.IncreasePrice();
                    button.SetCurrentOwnNum(1);
                    button.SetPriceText(factory.Price);
                    //プレイヤーへの処理
                    //・資源を減らす
                    //・自動生成数を増やす
                    //・所持数データを増やす
                }
            };

            //売却時の処理を登録する
            button.OnRightClickEvent += () =>
            {
                if (0 < button.CurrentOwnNum)
                {
                    PlayerManager.Instance.AddCookie(factory.Price * (2d / 3d));
                    factory.Sell(factoryInfo);
                    factory.DecreasePrice();
                    button.SetCurrentOwnNum(-1);
                    button.SetPriceText(factory.Price);
                }
            };
            id++;
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

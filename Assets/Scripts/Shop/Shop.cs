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
            var factoryInfo = new FactoryInfo(factory.name, new(0, 0), 1);
            PlayerManager.Instance.AutoGeneratorDictionary.Add(factory.name, factoryInfo);
            button.SetItemName(factory.name, true);
            button.SetPrice(factory.Price);
            //購入時の処理を登録する。
            button.OnClickEvent += () =>
            {
                if (factory.Price <= PlayerManager.Instance.PlayerResources)
                {
                    PlayerManager.Instance.SubtractResource(factory.Price);
                    factory.Buy(factoryInfo);
                    factory.IncreasePrice();
                    button.SetCurrentOwnNum();
                    button.SetPrice(factory.Price);
                    //プレイヤーへの処理
                    //・資源を減らす
                    //・自動生成数を増やす
                    //・所持数データを増やす
                }
            };
            id++;
        }

        foreach (var upgrade in _upgrades)
        {
            var button = Instantiate(_shopItemButtonPrefab, _shopItemButtonParent).GetComponent<ShopItemButton>();
            button.SetItemName(upgrade.name, false);
            button.SetPrice(upgrade.Price);
            var dummyFactoryInfo = new FactoryInfo(upgrade.name, upgrade.Price, 1);
            button.OnClickEvent += () =>
            {
                if (upgrade.Price <= PlayerManager.Instance.PlayerResources)
                {
                    //プレイヤーへの処理
                    //・資源を減らす
                    //・倍率を増やす
                    upgrade.Buy(dummyFactoryInfo);
                    PlayerManager.Instance.SubtractResource(upgrade.Price);
                    Destroy(button.gameObject);
                }
            };
        }
    }
}

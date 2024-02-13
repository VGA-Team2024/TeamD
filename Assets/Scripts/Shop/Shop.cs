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
        foreach (var factory in _factories)
        {   
            var button = Instantiate(_shopItemButtonPrefab, _shopItemButtonParent).GetComponent<ShopItemButton>();
            button.SetItemName(factory.name, true);
            button.SetPrice(factory.Price);
            //購入時の処理を登録する。
            button.OnClickEvent += () =>
            {
                if (factory.Price <= PlayerManager.Instance.PlayerResources)
                {
                    factory.Buy();
                    factory.IncreasePrice();
                    button.SetCurrentOwnNum();
                    button.SetPrice(factory.Price);
                    PlayerManager.Instance.SubtractResource(factory.Price);
                }
            };
        }

        foreach (var upgrade in _upgrades)
        {
            var button = Instantiate(_shopItemButtonPrefab, _shopItemButtonParent).GetComponent<ShopItemButton>();
            button.SetItemName(upgrade.name, false);
            button.SetPrice(upgrade.Price);
            button.OnClickEvent += () =>
            {
                if (upgrade.Price <= PlayerManager.Instance.PlayerResources)
                {
                    upgrade.Buy();
                    PlayerManager.Instance.SubtractResource(upgrade.Price);
                    Destroy(button.gameObject);
                }
            };
        }
    }
}

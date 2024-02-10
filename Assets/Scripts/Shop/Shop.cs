using System.Collections;
using System.Collections.Generic;
using UnityEngine;

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
            button.SetItemName(factory.name);
            //w“üŽž‚Ìˆ—‚ð“o˜^‚·‚éB
            button.OnClickEvent += () =>
            {
                //if (factory.Price <= PlayerData.Instance.Money)
                {
                    factory.Buy();
                    factory.IncreasePrice();
                }
            };
        }

        foreach (var upgrade in _upgrades)
        {
            var button = Instantiate(_shopItemButtonPrefab, _shopItemButtonParent).GetComponent<ShopItemButton>();
        }
    }
}

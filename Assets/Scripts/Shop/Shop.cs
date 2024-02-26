using System.Collections.Generic;
using System.Linq;
using UniRx;
using UnityEngine;
using UnityEngine.UI;

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
    [SerializeField, Tooltip("施設売却時の返金割合")] float _sellFactoryRatio = 2f / 3f;
    [SerializeField] OriginalButton _switchPurchaseButton;
    [SerializeField] OriginalButton _switchSellButton;
    Factories _factories;
    readonly List<ShopItemButton> _currentFactoryButtons = new();
    readonly List<ShopItemButton> _currentUpgradeButtons = new();

    private void Start()
    {
        _factories = Resources.Load<Factories>("Excel/Factories");
        
        foreach (var factory in _factories.Entities)
        {   
            var button = Instantiate(_shopItemButtonPrefab, _factoryButtonParent).GetComponent<ShopItemButton>();
            _currentFactoryButtons.Add(button);
            button.SetItemName(factory.Name, true);
            button.CurrentOwnNumText.text = StatsManager.CurrentFactories[factory.Key].Amount.ToString();
            //  施設のベース価格 × 1.15^施設数
            button.SetPriceText(factory.BasePrice * Mathf.Pow(_factoryMultiplier, StatsManager.CurrentFactories[factory.Key].Amount));
            //  表示更新の処理を登録
            button.UpdateEvent += flag =>
            {
                button.CurrentOwnNumText.text = StatsManager.CurrentFactories[factory.Key].Amount.ToString();
                double price = factory.BasePrice * Mathf.Pow(_factoryMultiplier, 
                    StatsManager.CurrentFactories[factory.Key].Amount);;
                if (!flag)
                {
                    price *= _sellFactoryRatio;
                }
                button.SetPriceText(price);
            };
            //購入時の処理を登録する。
            button.PurchaseEvent += () =>
            {
                //  施設のベース価格 × 1.15^施設数
                var price = factory.BasePrice * Mathf.Pow(_factoryMultiplier, StatsManager.CurrentFactories[factory.Key].Amount);
                if (price <= PlayerManager.Instance.CookieCount)
                {
                    PlayerManager.Instance.SubtractCookie(price);
                    var stat = StatsManager.CurrentFactories[factory.Key];
                    stat.Amount++;
                    StatsManager.CurrentFactories[factory.Key] = stat;
                    button.UpdateEvent?.Invoke(true);
                }
            };

            //売却時の処理を登録する
            button.SellEvent += () =>
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
                    button.UpdateEvent?.Invoke(false);
                }
            };
        }
        StatsManager.OnUpdateNextUpgrades.Subscribe(GenerateUpgradeShop).AddTo(this);
        StatsManager.UpdateNextUpgrades();
        
        //  購入モード、売却モード切り替えボタン処理登録
        _switchPurchaseButton.RimLight.gameObject.SetActive(true);
        _switchPurchaseButton.Button.onClick.AddListener(() =>
        {
            _currentFactoryButtons.ForEach(b => b.IsPurchase.Value = true);
            UpdateFactoryShop();   //  値段の表示を買値に切り替え
            _switchPurchaseButton.RimLight.gameObject.SetActive(true);
            _switchSellButton.RimLight.gameObject.SetActive(false);
        });
        _switchSellButton.Button.onClick.AddListener(() =>
        {
            _currentFactoryButtons.ForEach(b => b.IsPurchase.Value = false);
            UpdateFactoryShop();    //  値段の表示を売値に切り替え
            _switchSellButton.RimLight.gameObject.SetActive(true);
            _switchPurchaseButton.RimLight.gameObject.SetActive(false);
        });
    }
    /// <summary>
    /// 施設ショップの表示を更新
    /// </summary>
    public void UpdateFactoryShop()
    {
        var flag = _currentFactoryButtons[0].IsPurchase.Value;
        foreach (var currentFactoryButton in _currentFactoryButtons)
        {
            currentFactoryButton.UpdateEvent?.Invoke(flag);
        }
    }

    void GenerateUpgradeShop(Dictionary<FactoriesEntity, (TiersEntity Tier, UpgradesEntity UpgradesInfo)> nextUpgrades)
    {
        _currentUpgradeButtons?.ForEach(b=>Destroy(b.gameObject));
        _currentUpgradeButtons?.Clear();
        foreach (var nextUpgrade in nextUpgrades)
        {
            var button = Instantiate(_shopItemButtonPrefab, _upgradeButtonParent).GetComponent<ShopItemButton>();
            _currentUpgradeButtons.Add(button);
            var price = nextUpgrade.Key.BasePrice * nextUpgrade.Value.Tier.Multiplier;
            button.SetItemName(nextUpgrade.Value.UpgradesInfo.Name, false);
            button.SetPriceText(price);
            if (ColorUtility.TryParseHtmlString(nextUpgrade.Value.Tier.Color, out Color color))
            {
                button.ButtonImage.color = color;
            }
            button.PurchaseEvent += () =>
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

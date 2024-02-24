using System.Linq;
using TeamD.Enum;
using UnityEngine;
using VContainer;

/// <summary>
/// 転生イベントクラス
/// </summary>
public class ReincarnationEvent : IEventClip
{
    CanvasManager _canvasManager;
    EventManager _eventManager;
    Shop _shop;
    [Inject]
    public void Construct(CanvasManager canvasManager, EventManager eventManager, Shop shop)
    {
        _canvasManager = canvasManager;
        _eventManager = eventManager;
        _shop = shop;
    }
    public void StartEvent()
    {
        _canvasManager.ReincarnationButton.onClick.RemoveListener(Reincarnation);
        _canvasManager.ReincarnationButton.onClick.AddListener(Reincarnation);
        _canvasManager.ReincarnationButton.interactable = true;
    }

    void Reincarnation()
    {
        _canvasManager.ReincarnationButton.interactable = false;
        StatsManager.ReincarnationCount += 1;
        var cookieCount = PlayerManager.Instance.CookieCount;
        if (cookieCount >= 1E+16)   //  1京(10^16)
        {
            StatsManager.HeavenlyChips += 1000;
        }
        else if (cookieCount >= 1E+12)  //  1兆(10^12)
        {
            StatsManager.HeavenlyChips += 10;
        }   
        else if (cookieCount >= 1E+8)   //  1億(10^8)  
        {
            StatsManager.HeavenlyChips++;
        }

        //  クッキーの数、施設、アップグレードを初期化する。
        PlayerManager.Instance.CookieCount = 0;
        foreach (var keyValue in StatsManager.CurrentFactories.ToList())
        {
            StatsManager.CurrentFactories[keyValue.Key] = (UpgradeTier.NoUpgrade, 0);
        }
        
        //  このクラスのEventClipを持っているEventTriggerのbool値を初期化する
        foreach (var index in _eventManager.EventTriggerData
                     .Where(datum => datum.EventTrigger.EventClip.Equals(this))
                     .Select((_, index) => index))
        {
            _eventManager.EventTriggerData[index].IsTriggered = false;
        }
        //  ショップ表示更新
        _shop.UpdateFactoryShop();
    }
}

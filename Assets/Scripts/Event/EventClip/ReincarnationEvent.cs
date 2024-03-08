using System.Linq;
using Story;
using TeamD.Enum;
using VContainer;

/// <summary>
/// 転生イベントクラス
/// </summary>
public class ReincarnationEvent : IEventClip
{
    CanvasManager _canvasManager;
    EventManager _eventManager;
    StoryEventManager _storyEventManager;
    Shop _shop;
    [Inject]
    public void Construct(CanvasManager canvasManager, EventManager eventManager, Shop shop, StoryEventManager storyEventManager)
    {
        _canvasManager = canvasManager;
        _eventManager = eventManager;
        _storyEventManager = storyEventManager;
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
        AddHeavenlyChips();
        
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
        //  ストーリー進行のフラグをリセットする
        for (int i = 0; i < _storyEventManager.EventTriggerInfos.Count; i++)
        {
            _storyEventManager.EventTriggerInfos[i].IsTriggered = false;
            _storyEventManager.EventTriggerInfos[i].IsStoryEnded = false;
        }
        //  ショップ表示更新
        _shop.UpdateFactoryShop();
    }

    void AddHeavenlyChips()
    {
        var cookieCount = PlayerManager.Instance.CookieCount;
        foreach (var entities in StatsManager.ReincarnationData.Entities
                     .OrderByDescending(e=>e.CookieCount))
        {
            if (cookieCount >= entities.CookieCount 
                && (entities.Limit > StatsManager.ReincarnationRewardCount[entities.CookieCount] || entities.Limit == 0))
            {
                StatsManager.HeavenlyChips += entities.HeavenlyChips;
                StatsManager.ReincarnationRewardCount[entities.CookieCount]++;
                return;
            }
        }
    }
}

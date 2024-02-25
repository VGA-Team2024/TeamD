using System.Collections;
using System.Linq;
using Editor.EditorClicker.Data;
using Editor.EditorClicker.Scripts;
using Story;
using UnityEngine;
using VContainer;

public class JsonManager : MonoBehaviour
{
    [Inject] EventManager EventManager { get; set; }
    [Inject] StoryEventManager StoryEventManager { get; set; }
    Coroutine _autoSave;
    void Awake()
    {
        JsonLoad();
    }

    void Start()
    {
        _autoSave = StartCoroutine(AutoSave());
    }

    void OnDestroy()
    {
        if(_autoSave != null)
            StopCoroutine(_autoSave);
    }

    void OnApplicationQuit()
    {
        JsonSave();
    }
    public void JsonSave()
    {
        UserData userData = new UserData
        {
            CookieCount = PlayerManager.Instance.CookieCount,
            Achievements = StatsManager.Achievements,
            GoldenCookieObtainCount = StatsManager.GoldenCookieObtainCount,
            HeavenlyChips = StatsManager.HeavenlyChips,
            ReincarnationCount = StatsManager.ReincarnationCount,
            FactorySellData = StatsManager.FactorySellCount
                .Select(dic => new FactorySellData
                {
                    FactoryKey = dic.Key, SellCount = dic.Value
                }).ToList(),
            FactoryData = StatsManager.CurrentFactories
                .Select(dic=> new FactoryData
                {
                    FactoryKey = dic.Key, UpgradeTier = dic.Value.Tier, Amount = dic.Value.Amount
                }).ToList(),
            EventTriggerData = EventManager.EventTriggerData
                .Select(datum=> new EventTriggerSaveDatum
                {
                    UniqueID = datum.EventTrigger.UniqueID, IsTriggered = datum.IsTriggered
                }).ToList(),
            StoryControllerData = StoryEventManager.EventTriggerInfos
                .Select(datum=> new EventTriggerSaveDatum
            {
                UniqueID = datum.EventTrigger.UniqueID, IsTriggered = datum.IsTriggered
            }).ToList(),
            StoryFlag = StoryTextManager.Instance.StoryFlag
        };
        SaveService.Save(userData);
    }

    public void JsonLoad()
    {
        if (SaveService.Load(out UserData data))
        {
            PlayerManager.Instance.CookieCount = data.CookieCount;
            StatsManager.Achievements = data.Achievements;
            StatsManager.GoldenCookieObtainCount = data.GoldenCookieObtainCount;
            StatsManager.HeavenlyChips = data.HeavenlyChips;
            StatsManager.ReincarnationCount = data.ReincarnationCount;
            StatsManager.FactorySellCount.Clear();
            StatsManager.FactorySellCount = data.FactorySellData
                .ToDictionary(d=>d.FactoryKey, d=>d.SellCount);
            StatsManager.CurrentFactories.Clear();
                data.FactoryData.ForEach(list=>
                    StatsManager.CurrentFactories
                    .Add(list.FactoryKey, (list.UpgradeTier, list.Amount)));
            StatsManager.UpdateCpS();
            StatsManager.UpdateNextUpgrades();
            foreach (var eventTriggerDatum in EventManager.EventTriggerData)
            {
                //  保存されたトリガーの中に現在EventManagerで設定されているトリガーと同じものがあれば、
                // 保存された方のbool値でEventManagerで設定されているトリガーのbool値を上書きする。
                var findData = data.EventTriggerData.FirstOrDefault(datum=> datum.UniqueID.Equals(eventTriggerDatum.EventTrigger.UniqueID));
                if (findData != null)
                {
                    eventTriggerDatum.IsTriggered = findData.IsTriggered;
                }
            }
            foreach (var eventTriggerDatum in StoryEventManager.EventTriggerInfos)
            {
                var findData = data.StoryControllerData.FirstOrDefault(datum=> datum.UniqueID.Equals(eventTriggerDatum.EventTrigger.UniqueID));
                if (findData != null)
                {
                    eventTriggerDatum.IsTriggered = findData.IsTriggered;
                }
            }
            StoryTextManager.Instance.StoryFlag = data.StoryFlag;
        }
    }

    IEnumerator AutoSave()
    {
        var waitForSeconds = new WaitForSeconds(60);
        while (true)
        {
            print("Auto save");
            JsonSave();
            yield return waitForSeconds;
        }
    }
}

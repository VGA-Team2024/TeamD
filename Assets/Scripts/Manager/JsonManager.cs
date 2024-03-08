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
            StoryFlag = StoryTextManager.Instance.StoryFlag,
            ReincarnationRewardCounts = StatsManager.ReincarnationRewardCount
                .Select(c=>new ReincarnationRewardCount
            {
                Key = c.Key, Count = c.Value
            }).ToList(),
            StoryData = StoryEventManager.EventTriggerInfos.Select(info=>new StoryDatum
            {
                UniqueID = info.EventTrigger.UniqueID, IsStoryEnded = info.IsStoryEnded
            }).ToList()
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
            foreach (var info in StoryEventManager.EventTriggerInfos)
            {
                //  保存されたトリガーの中に現在EventManagerで設定されているトリガーと同じものがあれば、
                // 保存された方のbool値でEventManagerで設定されているトリガーのbool値を上書きする。
                var findData = data.StoryData.FirstOrDefault(datum=> datum.UniqueID.Equals(info.EventTrigger.UniqueID));
                if (findData != null)
                {
                    info.IsStoryEnded = findData.IsStoryEnded;
                }
            }
            StoryTextManager.Instance.StoryFlag = data.StoryFlag;
            foreach (var count in data.ReincarnationRewardCounts)
            {
                StatsManager.ReincarnationRewardCount[count.Key] = count.Count;
            }
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

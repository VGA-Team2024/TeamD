using System;
using System.Linq;
using Editor.EditorClicker.Data;
using Editor.EditorClicker.Scripts;
using UnityEngine;

public class JsonManager : MonoBehaviour
{
    public static JsonManager Instance;
    void Awake()
    {
        if (Instance)
        {
            Destroy(gameObject);
        }
        else
        {
            Instance = this;
            //  何かの子オブジェクトになっている場合はDontDestroyOnLoadできないので実行時にはParentを解除する
            transform.SetParent(null);  
            DontDestroyOnLoad(gameObject);
        }
    }

    void Start()
    {
        if (Instance == this)
        {
            JsonLoad();
        }
    }

    void OnApplicationQuit()
    {
        JsonSave();
    }

    [ContextMenu("セーブする")]
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
            EventTriggerData = EventManager.Instance.EventTriggerData
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
            foreach (var eventTriggerDatum in EventManager.Instance.EventTriggerData)
            {
                //  保存されたトリガーの中に現在EventManagerで設定されているトリガーと同じものがあれば、
                // 保存された方のbool値でEventManagerで設定されているトリガーのbool値を上書きする。
                var findData = data.EventTriggerData.FirstOrDefault(e=> e.EventTrigger.Equals(eventTriggerDatum.EventTrigger));
                if (findData != null)
                {
                    eventTriggerDatum.IsTriggered = findData.IsTriggered;
                }
            }
        }
    }
}

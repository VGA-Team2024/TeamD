using System;
using System.Collections.Generic;
using System.Linq;
using TeamD.Enum;
using UniRx;
using UnityEngine;

/// <summary>
///     統計管理クラス
/// </summary>
public static class StatsManager
{
    /// <summary>購入可能なアップグレードのディクショナリ</summary>
    static readonly Dictionary<FactoriesEntity, (TiersEntity Tier, UpgradesEntity UpgradesInfo)> _nextUpgrades = new();

    static readonly Factories _factories;
    static readonly Tiers _tiers;
    static readonly Upgrades _upgrades;
    public static readonly ReincarnationData ReincarnationData;

    static StatsManager()
    {
        _factories = Resources.Load<Factories>("Excel/Factories");
        _tiers = Resources.Load<Tiers>("Excel/Tiers");
        _upgrades = Resources.Load<Upgrades>("Excel/Upgrades");
        ReincarnationData = Resources.Load<ReincarnationData>("Excel/ReincarnationData");
        //  転生用データを読み取ってDictionaryを作成
        ReincarnationRewardCount = ReincarnationData.Entities.ToDictionary(e => e.CookieCount, _ => 0);
        CurrentFactories.ObserveReplace().Subscribe(_ => UpdateCpS());
        CurrentFactories.ObserveReplace().Subscribe(_ => UpdateNextUpgrades());
    }
    /// <summary>ゴールデンクッキー獲得回数</summary>
    public static int GoldenCookieObtainCount { get; set; }
    /// <summary>一定のクッキーを所持した状態で転生した際に増やす値</summary>
    public static int HeavenlyChips { get; set; }
    /// <summary>転生回数</summary>
    public static int ReincarnationCount { get; set; }
    /// <summary>保有実績</summary>
    public static Achievement Achievements { get; set; }
    /// <summary>施設売却数</summary>
    public static Dictionary<FactoryKey, int> FactorySellCount { get; set; } 
        = new(Enum.GetValues(typeof(FactoryKey)).Cast<FactoryKey>().ToDictionary(e=>e, _=>0));

    /// <summary>施設のアップグレード状況と数</summary>
    public static ReactiveDictionary<FactoryKey, (UpgradeTier Tier, int Amount)> CurrentFactories { get; } 
        = new(Enum.GetValues(typeof(FactoryKey)).Cast<FactoryKey>().ToDictionary(e=>e, _=>(UpgradeTier.NoUpgrade,0)));

    /// <summary>購入可能なアップグレードのディクショナリが更新された時に呼ばれるSubject</summary>
    public static Subject<Dictionary<FactoriesEntity, (TiersEntity Tier, UpgradesEntity UpgradesInfo)>> OnUpdateNextUpgrades { get; } = new();

    /// <summary>Cookie Per Seconds: 一秒間に生成するクッキー数</summary>
    public static double CpS { get; private set; }
    /// <summary>転生時に受け取った報酬のカウント</summary>
    public static Dictionary<double, int> ReincarnationRewardCount { get; set; }

    public static void UpdateCpS()
    {
        var sum = CurrentFactories.Select(x => _factories.Entities
            .Find(e => e.Key == x.Key).CpS * x.Value.Amount * (1 << (int)x.Value.Tier)).Sum();
        CpS = sum + sum * HeavenlyChips / 100;
    }

    public static void UpdateNextUpgrades()
    {
        _nextUpgrades.Clear();
        foreach (var pair in CurrentFactories)
        {
            var nextTier = _tiers.Entities.FirstOrDefault(x => x.Tier == pair.Value.Tier + 1);
            if (nextTier == null) continue;
            if (nextTier.Condition <= pair.Value.Amount)
                _nextUpgrades.Add(_factories.Entities
                    .First(e => e.Key.Equals(pair.Key)), (nextTier, _upgrades.Entities
                    .FirstOrDefault(e => e.Key.Equals(pair.Key) && e.Tier.Equals(nextTier.Tier))));
        }

        OnUpdateNextUpgrades.OnNext(_nextUpgrades);
    }
}
using System;
using System.Collections.Generic;
using System.Linq;
using TeamD.Enum;
using UniRx;
using UnityEngine;

/// <summary>
/// 統計管理クラス
/// </summary>
public static class StatsManager
{
    /// <summary>一定のクッキーを所持した状態で転生した際に増やす値</summary>
    public static int HeavenlyChipStats { get; set; }
    /// <summary>保有実績</summary>
    public static Achievement AchievementStats { get; set; }

    /// <summary>施設のアップグレード状況と数</summary>
    public static ReactiveDictionary<FactoryKey, (UpgradeTier Tier, int Amount)> FactoryStats { get; set; }

    /// <summary>購入可能なアップグレードのディクショナリ</summary>
    static Dictionary<FactoriesEntity, (TiersEntity Tier, UpgradesEntity UpgradesInfo)> _nextUpgrades = new();
    public static Subject<Dictionary<FactoriesEntity, (TiersEntity Tier, UpgradesEntity UpgradesInfo)>> OnUpdateNextUpgrades { get; } = new();
    /// <summary>Cookie Per Seconds: 一秒間に生成するクッキー数</summary>
    public static double CpS { get; private set; }

    static Factories _factories;
    static Tiers _tiers;
    static Upgrades _upgrades;

    static void UpdateCpS()
    {
        CpS = FactoryStats.Select(x => _factories.Entities
            .Find(e => e.Key == x.Key).CpS * x.Value.Amount * (1 << (int)x.Value.Tier)).Sum();
    }

    static void UpdateNextUpgrades()
    {
        _nextUpgrades.Clear();
        foreach (var pair in FactoryStats)
        {
            var nextTier = _tiers.Entities.FirstOrDefault(x => x.Tier == pair.Value.Tier + 1);
            if (nextTier == null) continue;
            if (nextTier.Condition <= pair.Value.Amount)
            {
                _nextUpgrades.Add(_factories.Entities
                    .First(e=>e.Key.Equals(pair.Key)), (nextTier, _upgrades.Entities
                    .FirstOrDefault(e=>e.Key.Equals(pair.Key) && e.Tier.Equals(nextTier.Tier))));
            }
        }
        OnUpdateNextUpgrades.OnNext(_nextUpgrades);
    }
    static StatsManager()
    {
        _factories = Resources.Load<Factories>("Excel/Factories");
        _tiers = Resources.Load<Tiers>("Excel/Tiers");
        _upgrades = Resources.Load<Upgrades>("Excel/Upgrades");
        FactoryStats = _factories.Entities.ToDictionary(entity=>entity.Key, _=>(UpgradeTier.NoUpgrade, 0))
            .ToReactiveDictionary();
        FactoryStats.ObserveReplace().Subscribe(_ => UpdateCpS());
        FactoryStats.ObserveReplace().Subscribe(_ => UpdateNextUpgrades());
    }
}
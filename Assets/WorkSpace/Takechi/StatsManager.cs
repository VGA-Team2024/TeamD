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

    static StatsManager()
    {
        var factories = Resources.Load<Factories>("Excel/Factories");
        FactoryStats = factories.Entities.ToDictionary(entity=>entity.Key, _=>(UpgradeTier.NoUpgrade, 0))
            .ToReactiveDictionary();
    }
}
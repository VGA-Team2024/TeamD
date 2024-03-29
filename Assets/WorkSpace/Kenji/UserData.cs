using System;
using System.Collections.Generic;
using Story;
using TeamD.Enum;
using UnityEngine;


namespace Editor.EditorClicker.Data
{
    /// <summary>
    /// ユーザのデータ
    /// </summary>
    [Serializable]
    public class UserData
    {
        [field: SerializeField]
        public double CookieCount { get; set; }
        [field: SerializeField]
        public List<FactoryData> FactoryData { get; set; }
        [field: SerializeField]
        public List<FactorySellData> FactorySellData { get; set; }
        [field: SerializeField]
        public Achievement Achievements { get; set; }
        [field: SerializeField]
        public int HeavenlyChips { get; set; }
        [field: SerializeField]
        public int GoldenCookieObtainCount { get; set; }
        [field: SerializeField]
        public int ReincarnationCount { get; set; }
        [field: SerializeField]
        public StoryTextManager.StoryFlagEnum StoryFlag { get; set; }
        [field: SerializeField] 
        public List<ReincarnationRewardCount> ReincarnationRewardCounts { get; set; }
        [field: SerializeField]
        public List<StoryDatum> StoryData { get; set; }
    }
    [Serializable]
    public class StoryDatum
    {
        [field: SerializeField] public string UniqueID { get; set; }
        [field: SerializeField] public bool IsStoryEnded { get; set; }
    }
    [Serializable]
    public struct FactoryData
    {
        [field: SerializeField]
        public FactoryKey FactoryKey { get; set; }
        [field: SerializeField]
        public UpgradeTier UpgradeTier { get; set; }
        [field: SerializeField]
        public int Amount { get; set; }
    }
    [Serializable]
    public struct FactorySellData
    {
        [field: SerializeField]
        public FactoryKey FactoryKey { get; set; }
        [field: SerializeField]
        public int SellCount { get; set; }
    }

    [Serializable]
    public struct ReincarnationRewardCount
    {
        [field: SerializeField] public double Key { get; set; }
        [field: SerializeField] public int Count { get; set; }
    }
}
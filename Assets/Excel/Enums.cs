using System;
using UnityEngine;

namespace TeamD.Enum
{
    [Flags]
    public enum Achievement
    {
        [InspectorName("アポカリプス")] Apocalypse           = 1 << 0, 
        [InspectorName("ババアグッバイ")] GoodbyeGrandma       = 1 << 1,
        [InspectorName("ラッキー！")] Lucky                = 1 << 2,
        [InspectorName("転生者")] Reincarnation        = 1 << 3,
        [InspectorName("世界崩壊")] CollapseOfTheWorld   = 1 << 4,
        [InspectorName("無限ループ")] InfiniteLoop         = 1 << 5,
        [InspectorName("すべてを知るもの")] TheAllKnowingOne     = 1 << 6,
        [InspectorName("全クリ")] CompletingEveryLevel = 1 << 7,
    }
    public enum FactoryKey
    {
        Cursor,
        Grandma,
        Farm,
        Mine,
        Factory,
        Bank,
        Temple,
        WizardTower,
        Shipment,
        AlchemyLab,
        Portal,
        TimeMachine,
        AntimatterCondenser,
        Prism,
        ChanceMaker,
        FractalEngine,
        JavascriptConsole,
        Idleverse,
        CortexBaker,
        You
    }

    public enum UpgradeTier
    {
        Plain,
        Berrylium,
        Blueberrylium,
        Chalcedhoney,
        Buttergold,
        Sugarmuck,
        Jetmint,
        Cherrysilver,
        Hazelrald,
        Mooncandy,
        Astrofudge,
        Alabascream,
        Lridyum,
        Glucosmium,
        Glimmeringue,
    }
}
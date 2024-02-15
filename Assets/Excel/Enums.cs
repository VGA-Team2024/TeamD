using System;

namespace TeamD.Enum
{
    [Flags]
    public enum Achievement
    {
        Apocalypse           = 1 << 0, //  アポカリプス
        GoodbyeGrandma       = 1 << 1, //  ババアグッバイ
        Lucky                = 1 << 2, //  ラッキー！
        Reincarnation        = 1 << 3, //  転生者
        CollapseOfTheWorld   = 1 << 4, //  世界崩壊
        InfiniteLoop         = 1 << 5, //  無限ループ
        TheAllKnowingOne     = 1 << 6, //  すべてを知るもの
        CompletingEveryLevel = 1 << 7, //  全クリ
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
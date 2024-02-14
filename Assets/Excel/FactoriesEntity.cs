using System;

[Serializable]
public class FactoriesEntity
{
    public FactoryKey Key;
    public string Name;
    public double BasePrice;
    public double CpS;
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
}


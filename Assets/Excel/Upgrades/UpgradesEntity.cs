using System;
using TeamD.Enum;

[Serializable]
public class UpgradesEntity
{
    public FactoryKey Key;
    public UpgradeTier Tier;
    public string Name;
}
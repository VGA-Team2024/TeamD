using System;
using TeamD.Enum;

[Serializable]
public class TiersEntity
{
    public UpgradeTier Tier;
    public string Name;
    public int Condition;
    public double Multiplier;
    public string Color;
}
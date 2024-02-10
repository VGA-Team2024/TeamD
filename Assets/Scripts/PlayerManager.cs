using UnityEngine;

public class PlayerManager
{
    public decimal PlayerResources { get; private set; } = 0;
    public decimal AutoGenerateCount { get; private set; } = 1;
    public decimal ManualGenerateCount { get; private set; } = 10000;

    public void AddResource(decimal resource)
    {
        PlayerResources += resource;
    }
}
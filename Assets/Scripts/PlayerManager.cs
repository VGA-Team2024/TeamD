using UnityEngine;

public class PlayerManager
{
    public long PlayerResources { get; private set; } = 0;
    public long AutoGenerateCount { get; private set; } = 1;
    public long ManualGenerateCount { get; private set; } = 10000;

    public void AddResource(long resource)
    {
        PlayerResources += resource;
    }
}
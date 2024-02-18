using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.UI;

public class RE_Life : UpgradeBase
{
    [SerializeField]
    public Button button;
    public  LargeNumber PlayerResources { get; private set; } = new(0, 0);

    public FactoryInfo ManualGenerateCount = new("manual", new(1, 0), 1);

    private void Reset()
    {
        button.onClick.RemoveAllListeners();
    }
}

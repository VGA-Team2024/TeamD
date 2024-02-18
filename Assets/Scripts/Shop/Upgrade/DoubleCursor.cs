using UnityEngine;

public class DoubleCursor : MonoBehaviour, IUpgrade
{
    [SerializeField, Tooltip("カーソル施設のプレハブ名")]
    private string _factoryCursorName;
    public void ApplyUpgrade()
    {
        //カーソルの自動生成数の2の累乗の指数の数を増やす
        // var cursorAutoGenerateInfo = PlayerManager.Instance.AutoGeneratorDictionary[_factoryCursorName];
        // var newFactoryInfo = new FactoryInfo(_factoryCursorName, cursorAutoGenerateInfo.BaseGeneratorValue, cursorAutoGenerateInfo.BasePower + 1);
        // cursorAutoGenerateInfo = newFactoryInfo;
        // //手動生成数の2の累乗の指数の数を増やす
        // PlayerManager.Instance.ManualGenerateCount.AddPower(1);
    }
}

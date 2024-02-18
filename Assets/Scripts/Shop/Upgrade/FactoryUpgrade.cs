using UnityEngine;

public class FactoryUpgrade : MonoBehaviour, IUpgrade
{
    [SerializeField, Tooltip("アップグレードする施設のプレハブ名")]
    string _factoryName;
    public void ApplyUpgrade()
    {
        //対象の施設の自動生成数にかける2の指数を増やす
        // var cursorAutoGenerateInfo = PlayerManager.Instance.AutoGeneratorDictionary[_factoryName];
        // var newfactoryInfo = new FactoryInfo(_factoryName, cursorAutoGenerateInfo.BaseGeneratorValue, cursorAutoGenerateInfo.BasePower + 1);
        // cursorAutoGenerateInfo = newfactoryInfo;
    }
}

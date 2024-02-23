using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GameManager : MonoBehaviour
{
    //ScriptableObjectの活用
    //・データベース
    private void Start()
    {
        //AchievementsScriptで取得したitemを名前や数で返している
        //Debug.Log(DataSO.Entity.itemDatas[0].name);
        //Debug.Log(DataSO.Entity.itemDatas[0].count);
        Debug.Log(DataSO.Entity.GetItem(ItemData.Type.COOKIE).name);
        Debug.Log(DataSO.Entity.GetItem(ItemData.Type.COOKIE).count);
    }
}

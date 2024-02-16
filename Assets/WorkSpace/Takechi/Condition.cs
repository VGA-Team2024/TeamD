using System;
using TeamD.Enum;
using UniRx;
using UnityEngine;

/// <summary>
/// 条件構造体
/// </summary>
[Serializable]
public struct Condition
{
    [Header("数値比較")] [SerializeField, Tooltip("左項に使うパラメーターの名前")]
    string _leftNumberKey;
    [SerializeField, Tooltip("比較に使う演算子")] CompareOptions _compareOptions;
    [SerializeField, Tooltip("右項")] int _rightNumber;
    [Header("要求実績")] [SerializeField, Tooltip("要求実績")] Achievement _requiredAchievement;

    public bool CheckCondition(ref ReactiveDictionary<string, int> dic, Achievement achievement)
    {
        //  キーが設定されていなかったら数値での条件判定は行わない
        if (_leftNumberKey != String.Empty)
        {
            //  キーが無ければfalseを返す
            if (!dic.ContainsKey(_leftNumberKey)) return false;
            //  指定された条件に当てはまらなければfalseを返す
            switch (_compareOptions)
            {
                case CompareOptions.Equals :
                    if (dic[_leftNumberKey] != _rightNumber) return false;
                    break;
                case CompareOptions.NotEquals :
                    if (dic[_leftNumberKey] == _rightNumber) return false;
                    break;
                case CompareOptions.GreaterEqual :
                    if (dic[_leftNumberKey] < _rightNumber) return false;
                    break;
                case CompareOptions.GreaterThan :
                    if (dic[_leftNumberKey] <= _rightNumber) return false;
                    break;
                case CompareOptions.LessEqual :
                    if (dic[_leftNumberKey] > _rightNumber) return false;
                    break;
                case CompareOptions.LessThan :
                    if (dic[_leftNumberKey] >= _rightNumber) return false;
                    break;
            }
        }
        //  指定された実績を保持していればtrue, いなければfalse
        return (achievement & _requiredAchievement) == _requiredAchievement;
    }
}
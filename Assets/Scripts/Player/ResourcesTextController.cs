using System;
using TMPro;
using UnityEngine;

public class ResourcesTextController : MonoBehaviour
{
    private TextMeshProUGUI _text;
    private PlayerManager _playerManager = PlayerManager.Instance;
    private const int MaxDigit = 68;

    public static readonly string[] _scales =
    {
        // この桁が分からない人向け https://www.hakko.co.jp/library/qa/qakit/html/h06010.htm
        //"", "k", "M", "G", "T", "P", "E", "Z", "Y", "R", "Q"
        "", "万", "億", "兆", "京", "垓", "𥝱", "穣", "溝", "澗", "正", "載", "極", "恒河沙", "阿僧祇", "那由多", "不可思議", "無量大数"
    };

    private void Start()
    {
        _text = GetComponent<TextMeshProUGUI>();
    }

    private void Update()
    {
        _text.text = $"{UseDigitRepresentation(_playerManager.CookieCount)} 枚";
    }

    /// <summary>
    /// 値に応じて桁表現を行い、テキストを更新するメソッド
    /// </summary>
    public static string UseDigitRepresentation(double value)
    {
        var count = Math.Log10(Math.Truncate(value));
        // 日本語表記での桁の最大表現
        if (MaxDigit <= count)
        {
            return $"{value / Math.Pow(10, MaxDigit):F2} {_scales[_scales.Length]}";
        }

        if (count < 4)
        {
            return $"{(value):F2}";
        }

        if (count % 4 == 0)
        {
            return $"{value / Math.Pow(10, count):F2} {_scales[(int)count / 4]}";
        }

        return $"{value / Math.Pow(10, count - count % 4):F2} {_scales[(int)count / 4]}";
    }
}
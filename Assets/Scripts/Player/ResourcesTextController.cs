using System;
using TMPro;
using UnityEngine;

public class ResourcesTextController : MonoBehaviour
{
    private TextMeshProUGUI _text;

    private PlayerManager _playerManager = PlayerManager.Instance;

    // 英語表現での最大桁数
    private const int MaxDigit = 99;
    
    public static readonly string[] _scales =
    {
        // 参考サイト : https://nanaon.tamura-shippo.com/entertainment/game/cookie-clicker/unit/
        "", "", "", "million", "billion", "trillion", "quadrillion", "quintillion", "sextillion", "septillion",
        "octillion", "nonillion", "decillion", "undecillion", "duodecillion", "tredecillion", "quattuordecillion",
        "quindecillion", "sexdecillion", "septendecillion", "octodecillion", "novemdecillion", "vigintillion",
        "unvigintillion", "duovigintillion", "tresvigintillion", "quattuorvigintillion", "quinvigintillion",
        "sexvigintillion", "septenvigintillion", "octovigintillion", "novemvigintillion", "trigintillion",
        "untrigintillion", "duotrigintillion"
    };

    private void Start()
    {
        _text = GetComponent<TextMeshProUGUI>();
    }

    private void Update()
    {
        var str = UseDigitRepresentation(_playerManager.CookieCount * 1000000).Split();
        _text.text = $"{str[0]}\n{str[1]}クッキー";
    }

    /// <summary>
    /// 値に応じて桁表現を行い、テキストを更新するメソッド
    /// </summary>
    public static string UseDigitRepresentation(double value)
    {
        var count = Math.Log10(Math.Truncate(value));
        if (MaxDigit <= count)
        {
            return $"{value / Math.Pow(10, MaxDigit):F2} {_scales[_scales.Length]}";
        }

        if (count < 6)
        {
            return $"{(value):F2} ";
        }

        if (count % 3 == 0)
        {
            return $"{value / Math.Pow(10, count):F2} {_scales[(int)count / 3]}";
        }

        return $"{value / Math.Pow(10, count - count % 3):F2} {_scales[(int)count / 3]}";
    }
}
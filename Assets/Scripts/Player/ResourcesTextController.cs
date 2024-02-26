using System;
using TMPro;
using UnityEngine;

public class ResourcesTextController : MonoBehaviour
{
    private TextMeshProUGUI _text;

    private PlayerManager _playerManager = PlayerManager.Instance;

    // 英語表現での最大桁数
    private const int MaxDigitEn = 99;
    private const int MaxDigitJp = 68;

    public static readonly string[] ScalesJp =
    {
        "", "万", "億", "兆", "京", "垓", "秭", "穣", "溝", "澗", "正", "載", "極", "恒河沙", "阿僧祇", "那由多", "不可思議", "無量大数"
    };

    public static readonly string[] ScalesEn =
    {
        // 参考サイト : https://nanaon.tamura-shippo.com/entertainment/game/cookie-clicker/unit/
        "",  "", "million", "billion", "trillion", "quadrillion", "quintillion", "sextillion", "septillion",
        "octillion", "nonillion", "decillion", "undecillion", "duodecillion", "tredecillion", "quattuordecillion",
        "quindecillion", "sexdecillion", "septendecillion", "octodecillion", "novemdecillion", "vigintillion",
        "unvigintillion", "duovigintillion", "tresvigintillion", "quattuorvigintillion", "quinvigintillion",
        "sexvigintillion", "septenvigintillion", "octovigintillion", "novemvigintillion", "trigintillion",
        "untrigintillion", "duotrigintillion"
    };

    private enum LanguageType
    {
        En,
        Jp
    }

    [SerializeField] private LanguageType _type;
    private static LanguageType _languageMode;

    private void Awake()
    {
        _text = GetComponent<TextMeshProUGUI>();
        _languageMode = _type;
    }

    private void Update()
    {
        var str = UseDigitRepresentation(_playerManager.CookieCount).Split();
        if (0 < _playerManager.CookieCount)
        {
            _text.text = $"{str[0]}\n{str[1]}クッキー";
        }
        else
        {
            _text.text = $"- {str[0]}\n{str[1]}クッキー";
        }
    }

    /// <summary>
    /// 値に応じて桁表現を行い、テキストを更新するメソッド
    /// </summary>
    public static string UseDigitRepresentation(double value)
    {
        value = Math.Abs(value);
        var count = Math.Log10(Math.Truncate(value));
        switch (_languageMode)
        {
            case LanguageType.En:
                if (MaxDigitEn <= count)
                {
                    return $"{value / Math.Pow(10, MaxDigitEn):F2} {ScalesEn[^1]}";
                }

                if (count < 6)
                {
                    return $"{(value):F2} ";
                }

                if (count % 3 == 0)
                {
                    return $"{value / Math.Pow(10, count):F2} {ScalesEn[(int)count / 3]}";
                }

                return $"{value / Math.Pow(10, count - count % 3):F2} {ScalesEn[(int)count / 3]}";
            case LanguageType.Jp:
                if (MaxDigitJp <= count)
                {
                    return $"{value / Math.Pow(10, MaxDigitJp):F2} {ScalesJp[^1]}";
                }

                if (count < 4)
                {
                    return $"{(value):F2} ";
                }

                if (count % 4 == 0)
                {
                    return $"{value / Math.Pow(10, count):F2} {ScalesJp[(int)count / 4]}";
                }

                return $"{value / Math.Pow(10, count - count % 4):F2} {ScalesJp[(int)count / 4]}";
        }

        return "Enum未設定";
    }
}
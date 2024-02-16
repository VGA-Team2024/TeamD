using TMPro;
using UnityEngine;

public class ResourcesTextController : MonoBehaviour
{
    private TextMeshProUGUI _text;
    private PlayerManager _playerManager = PlayerManager.Instance;
    private bool _isCarry;
    
    /// <summary>桁上がりタイミングのカウント変数</summary>
    private int _carryCount;

    private string[] _scales =
    {
        // この桁が分からない人向け https://www.hakko.co.jp/library/qa/qakit/html/h06010.htm
        "", "k", "M", "G", "T", "P", "E", "Z", "Y", "R", "Q"
        // TODO 桁一覧TextMeshPro適用
        // "", "万", "億", "兆", "京", "垓", "𥝱", "穣", "溝", "澗", "正", "載", "極", "恒河沙", "阿僧祇", "那由多", "不可思議", "無量大数"
    };

    private void Start()
    {
        _text = GetComponent<TextMeshProUGUI>();
    }

    private void Update()
    {
        UpdateText();
    }

    /// <summary>
    /// 値に応じて桁表現を行い、テキストを更新するメソッド
    /// </summary>
    private void UpdateText()
    {
        var value = _playerManager.CookieCount;
        var count = Mathf.Log10((int)value);
        if (count < 3)
        {
            _text.text = $"{(value):F2} Cookies";
        }
        else if ((int)count % 3 == 0)
        {
            if (!_isCarry)
            {
                _carryCount = (int)count;
                _isCarry = true;
            }
            _text.text = $"{value / Mathf.Pow(10, (int)count):F2} {_scales[(int)count / 3]} Cookies";
        }
        else
        {
            _isCarry = false;
            _text.text = $"{value / Mathf.Pow(10, _carryCount):F2} {_scales[_carryCount / 3]} Cookies";
        }
    }
}
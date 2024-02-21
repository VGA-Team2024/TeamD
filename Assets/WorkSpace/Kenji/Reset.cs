using System.Linq;
using UnityEngine;
using TeamD.Enum;
using System;
using UnityEngine.UI;
public class Reset : MonoBehaviour
{
    public TMPro.TextMeshProUGUI _textMeshPro;
    public Button _resetButton;
    int _reincarnationCount = 1;
    [SerializeField] int _heavenlyChips = 1;

    // Update is called once per frame
    void Update()
    {
        if (GetType(FactoryKey.TimeMachine) == true)　//38行目で定義したものをここで使ってボタンでActive化する
        {
            bool isActive = true;

            _resetButton.onClick.AddListener(() =>
            {
                isActive = !isActive;
                _textMeshPro.gameObject.SetActive(isActive);
            });
        }
        void OnButtonReset()
        {
            PlayerManager.Instance.SubtractCookie(PlayerManager.Instance.CookieCount);
            StatsManager.CurrentFactories.Clear();
        }//クッキーの数,施設,アップグレードの値を初期化

        void GetButtonDpwm()
        {
            StatsManager.ReincarnationCount += _reincarnationCount; //転生回数をプラス

            if (PlayerManager.Instance.CookieCount >= 1e+8 && PlayerManager.Instance.CookieCount < 1e+11)
            {
                StatsManager.HeavenlyChips += _heavenlyChips;
            }else if (PlayerManager.Instance.CookieCount >= 1e+11 && PlayerManager.Instance.CookieCount < 1e+16)
            {
                StatsManager.HeavenlyChips += _heavenlyChips * 20;
            }
            else
            {
                StatsManager.HeavenlyChips += _heavenlyChips * 100;
            }
        }
    }

    private bool GetType(FactoryKey timeMachine)
    {
        throw new NotImplementedException();
    }

    [SerializeField] Factories _factories;
    
    void Find()
    {
        _factories.Entities.First(e=>e.Key == FactoryKey.TimeMachine);
    }//TimeMachineを探す
}


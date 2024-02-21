using System.Linq;
using UnityEngine;
using TeamD.Enum;
using System;
using UnityEngine.UI;
public class Reset : MonoBehaviour
{
    public TMPro.TextMeshProUGUI textMeshPro;
    public Button resetButton;

    // Update is called once per frame
    void Update()
    {
        if (GetType(FactoryKey.TimeMachine) == true)　//38行目で定義したものをここで使ってボタンでActive化する
        {
            bool isActive = true;

            resetButton.onClick.AddListener(() =>
            {
                isActive = !isActive;
                textMeshPro.gameObject.SetActive(isActive);
            });
        }
        void OnButtonReset()
        {
            PlayerManager.Instance.SubtractCookie(PlayerManager.Instance.CookieCount);
            StatsManager.CurrentFactories.Clear();
        }//クッキーの数,施設,アップグレードの値を初期化
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


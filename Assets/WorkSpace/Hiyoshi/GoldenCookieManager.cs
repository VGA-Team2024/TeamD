using System;
using UnityEngine;

public class GoldenCookieManager : MonoBehaviour
{
    [SerializeField] GameObject GoldenCookie = default;
    PlayerManager playerManager = PlayerManager.Instance;
    DateTime start; 
    RectTransform rectTransform;

    static int XRange = 100, YRange = 100; //ゴールデンクッキーが出現するX座標の範囲。0は画面の中央。
    float x = new System.Random().Next(-XRange, XRange);
    float y = new System.Random().Next(-YRange, YRange);
    int NextInstanceTime = new System.Random().Next(5,16); //ゴールデンクッキーが次に出現する時間

    void Start()
    {
        start = DateTime.Now;
        GoldenCookie.SetActive(false);
        rectTransform = GoldenCookie.GetComponent<RectTransform>();
        Debug.Log(NextInstanceTime);
    }

    void Update()
    {
        if (DateTime.Now - start >= TimeSpan.FromMinutes(NextInstanceTime) && GoldenCookie.activeSelf == false)
        {
            InstantiateGoldenCookie();
        }
    }
    /// <summary>
    /// ゴールデンクッキーがクリックされたときに呼ばれるメソッド
    /// </summary>
    public void ClickGoldenCookie()
    {
        playerManager.ChangeGoldenCookieMode(true);
        GoldenCookie.SetActive(false);
        start = DateTime.Now;
    }

    /// <summary>
    /// ゴールデンクッキーを出現させるときに呼ぶメソッド
    /// </summary>
    void InstantiateGoldenCookie()
    {
        GoldenCookie.SetActive(true);
        rectTransform.anchoredPosition = RandomPosition();
        Invoke(nameof(DeleteGoldenCookie), 13);
    }

    /// <summary>
    /// ゴールデンクッキーをDeleteするメソッド
    /// </summary>
    void DeleteGoldenCookie()
    {
        if (!playerManager.IsGoldenCookieMode)
        {
            GoldenCookie.SetActive(false);
            start = DateTime.Now;
        }
    }

    static Vector2 RandomPosition()
    {
        float x = new System.Random().Next(-XRange ,XRange);
        float y = new System.Random().Next(-YRange, YRange);
        Vector2 pos = new(x, y);
        return pos;
    }
}

using System;
using System.Collections;
using UnityEngine;
using UnityEngine.UI;
using Random = UnityEngine.Random;

public class GoldenCookieManager : MonoBehaviour
{
    [SerializeField] Button _goldenCookie = default;
    [SerializeField] Canvas _parentCanvas;
    PlayerManager _playerManager = PlayerManager.Instance;
    DateTime _start; 
    Coroutine _deleteGoldenCookieCoroutine;
    Button _instantiatedGoldenCookie;

    int _nextInstanceTime = new System.Random().Next(5,16); //�S�[���f���N�b�L�[�����ɏo�����鎞��
    bool _goldenCookieActivate;

    void Start()
    {
        _start = DateTime.Now;
        Debug.Log(_nextInstanceTime);
    }
    void Update()
    {
        if (DateTime.Now - _start >= TimeSpan.FromMinutes(_nextInstanceTime) && _goldenCookieActivate == false)
        {
            InstantiateGoldenCookie();
            _goldenCookieActivate = true;
            _nextInstanceTime = new System.Random().Next(5,16);
        }
    }
    /// <summary>
    /// �S�[���f���N�b�L�[���N���b�N���ꂽ�Ƃ��ɌĂ΂�郁�\�b�h
    /// </summary>
    public void ClickGoldenCookie()
    {
        if (_deleteGoldenCookieCoroutine != null)
        {
            StopCoroutine(_deleteGoldenCookieCoroutine);
            _deleteGoldenCookieCoroutine = null;
        }

        StatsManager.GoldenCookieObtainCount += 1;
        Destroy(_instantiatedGoldenCookie.gameObject);
        StartCoroutine(GoldenCookieBuff());
    }

    /// <summary>
    /// �S�[���f���N�b�L�[���o��������Ƃ��ɌĂԃ��\�b�h
    /// </summary>
    void InstantiateGoldenCookie()
    {
        _instantiatedGoldenCookie = Instantiate(_goldenCookie, _parentCanvas.transform);
        _instantiatedGoldenCookie.GetComponent<RectTransform>().anchoredPosition = RandomPosition();
        _instantiatedGoldenCookie.onClick.AddListener(ClickGoldenCookie);
        _deleteGoldenCookieCoroutine = StartCoroutine(DeleteGoldenCookie());
    }

    /// <summary>
    /// �S�[���f���N�b�L�[��Delete���郁�\�b�h
    /// </summary>
    IEnumerator DeleteGoldenCookie()
    {
        yield return new WaitForSeconds(13);
        Destroy(_instantiatedGoldenCookie.gameObject);
        _goldenCookieActivate = false;
        _start = DateTime.Now;
    }

    IEnumerator GoldenCookieBuff()
    {
        _playerManager.ChangeGoldenCookieMode(true);
        yield return new WaitForSeconds(77);
        _playerManager.ChangeGoldenCookieMode(false);
        _goldenCookieActivate = false;
        _start = DateTime.Now;
    }

    static Vector2 RandomPosition()
    {
        var halfWidth = Screen.currentResolution.width / 2;
        var halfHeight = Screen.currentResolution.height / 2;
        var randomPosition = new Vector2(Random.Range(-halfWidth, halfWidth), Random.Range(-halfHeight, halfHeight));
        return randomPosition;
    }
}

using System;
using TMPro;
using UniRx;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

/// <summary>
/// ショップのアイテムボタン
/// </summary>
[RequireComponent(typeof(Button))]
public class ShopItemButton : MonoBehaviour, IPointerEnterHandler, IPointerExitHandler
{
    [SerializeField] private TextMeshProUGUI _itemName;
    [SerializeField] private TextMeshProUGUI _currentOwnNumText;
    [SerializeField] private TextMeshProUGUI _priceText;
    [SerializeField] Image _buttonImage;
    [SerializeField] Image _highLight;
    [SerializeField] Image _rimLight;
    Button _button;
    private bool _isFactory;
    public event Action PurchaseEvent;
    public event Action SellEvent;
    /// <summary>表示更新イベント</summary>
    public Action<bool> UpdateEvent;
    public TextMeshProUGUI CurrentOwnNumText => _currentOwnNumText;
    public Image ButtonImage => _buttonImage;
    public BoolReactiveProperty IsPurchase { get; set; } = new(true);

    void Awake()
    {
        _highLight.gameObject.SetActive(false);
        _rimLight.gameObject.SetActive(false);
        _button = GetComponent<Button>();
        _button.onClick.AddListener(OnButtonClick);
        IsPurchase.Subscribe(b => _rimLight.gameObject.SetActive(!b)).AddTo(this);
    }

    public void OnButtonClick()
    {
        if (IsPurchase.Value)
        {
            PurchaseEvent?.Invoke();
        }
        else
        {
            SellEvent?.Invoke();
        }
    }

    /// <summary>
    /// 施設ならtrueを渡す
    /// </summary>
    public void SetItemName(string itemName, bool isFactory)
    {
        _itemName.text = itemName;
        _isFactory = isFactory;
        if (!_isFactory)
        {
            _currentOwnNumText.gameObject.SetActive(false);
        }
    }

    public void SetPriceText(double price)
    {
        _priceText.text = ResourcesTextController.UseDigitRepresentation(price);
    }

    public void OnPointerEnter(PointerEventData eventData)
    {
        _highLight.gameObject.SetActive(true);
    }

    public void OnPointerExit(PointerEventData eventData)
    {
        _highLight.gameObject.SetActive(false);
    }
}

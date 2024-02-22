using System;
using UniRx;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;
[RequireComponent(typeof(Button))]
public class OriginalButton : MonoBehaviour, IPointerEnterHandler, IPointerExitHandler
{
    [SerializeField] Image _notInteractableRimLight;
    [SerializeField] Image _highLight;
    [SerializeField] Image _rimLight;
    Button _button;
    public Image RimLight => _rimLight;
    public Button Button => _button;
    void Awake()
    {
        _button = GetComponent<Button>();
        _highLight.gameObject.SetActive(false);
        _notInteractableRimLight.gameObject.SetActive(false);
        if(_rimLight) _rimLight.gameObject.SetActive(false);
        this.ObserveEveryValueChanged(self=>self._button.interactable)
            .Subscribe(b =>_notInteractableRimLight.gameObject.SetActive(!b)).AddTo(this);
    }

    public void OnPointerEnter(PointerEventData eventData)
    {
        if (!_button.interactable) return;
        _highLight.gameObject.SetActive(true);
    }

    public void OnPointerExit(PointerEventData eventData)
    {
        _highLight.gameObject.SetActive(false);
    }
}

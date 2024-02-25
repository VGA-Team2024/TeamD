using DG.Tweening;
using UnityEngine;
using UnityEngine.Events;
using UnityEngine.EventSystems;

public class EventSystemScript : 
    MonoBehaviour, ICanvasRaycastFilter, 
    IPointerEnterHandler, IPointerExitHandler, 
    IPointerDownHandler, IPointerUpHandler
{
    [SerializeField] private UnityEvent _event;
    float _baseRadius, _radius;
    bool _pointerEnter;
    Tween _tween;
    float _prevSize = 1;

    void Awake()
    {
        _baseRadius = GetComponent<RectTransform>().rect.width / 2;
        _radius = _baseRadius;
    }
    /// <summary>サイズ変更アニメーション</summary>
    void Resize(float size)
    {
        _tween?.Kill();
        //  DoScaleだとTween途中のサイズ->変更後サイズをベースにイージングされるので
        //  DoVirtual.Floatで変更前サイズ->変更後サイズをベースにイージングを作ることで
        //  大きさを一定に保つことができた
        _tween = DOVirtual.Float(_prevSize, size, 1f, f =>
        {
            if (_prevSize >= size && transform.localScale[0] >= f)
            {
                transform.localScale = new Vector3(f,f,f);
            }
            if (_prevSize <= size && transform.localScale[0] <= f)
            {
                transform.localScale = new Vector3(f,f,f);
            }
        }).SetEase(Ease.OutElastic).SetLink(gameObject);
        _prevSize = size;
        //  マウスカーソルの接触判定を取るための半径をscaleに合わせる
        _radius = _baseRadius * size;
    }
    /// <summary>
    /// マウスカーソルの接触判定
    /// </summary>
    public bool IsRaycastLocationValid(Vector2 sp, Camera eventCamera)
    {
        return ((Vector3)sp - transform.position).sqrMagnitude < _radius * _radius;
    }
    public void OnPointerEnter(PointerEventData eventData)
    {
        _pointerEnter = true;
        Resize(1.05f);
    }

    public void OnPointerExit(PointerEventData eventData)
    {
        _pointerEnter = false;
        Resize(1f);
    }

    public void OnPointerDown(PointerEventData eventData)
    {
        _event.Invoke();
        Resize(0.95f);
    }

    public void OnPointerUp(PointerEventData eventData)
    {
        if(_pointerEnter) Resize(1.05f);
    }
}
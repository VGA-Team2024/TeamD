using UnityEngine;
using UnityEngine.Events;
using UnityEngine.EventSystems;

public class EventSystemScript : MonoBehaviour, IPointerClickHandler, ICanvasRaycastFilter
{
    [SerializeField] private UnityEvent _event;
    private const float Radius = 220;

    public void OnPointerClick(PointerEventData eventData)
    {
        _event.Invoke();
    }

    public bool IsRaycastLocationValid(Vector2 sp, Camera eventCamera)
    {
        return Vector2.Distance(sp, transform.position) < Radius;
    }
}
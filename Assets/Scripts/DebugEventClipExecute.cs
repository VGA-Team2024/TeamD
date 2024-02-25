using System.Diagnostics;
using UnityEngine;

public class DebugEventClipExecute : MonoBehaviour
{
    [SerializeReference, SubclassSelector] private IEventClip _eventClip;

    [Conditional("UNITY_EDITOR"), ContextMenu("ExecuteEvent")]
    public void ExecuteEvent()
    {
        _eventClip.StartEvent();
    }
}
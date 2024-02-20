using System.Collections.Generic;
using UnityEngine;

namespace Story
{
    public class Prologue : IEventClip
    {
        private static string[] _str =
        {
            "はじめまして", "おいしいクッキーを焼いてあげるよ"
        };

        private Queue<string> _sentence = new(_str);
        public void StartEvent()
        {
            Debug.Log("Play Data 1");
            StoryTextController.Instance.UpdateText(_sentence);
        }
    }
}
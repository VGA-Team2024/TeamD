using System;
using System.IO;
using System.Text;
using Editor.EditorClicker.Data;
using UnityEngine;

namespace Editor.EditorClicker.Scripts
{
    /// <summary>
    /// データをセーブする    
    /// </summary>
    public static class SaveService
    {
        static readonly string DataPath = Application.dataPath + "/StreamingAssets/";
        public static void Save(UserData data)
        {
            string jsonData = JsonUtility.ToJson(data);
            string path = new StringBuilder().Append(DataPath).Append(nameof(UserData)).Append(".json").ToString();
            File.WriteAllText(path, jsonData);
        }

        public static bool Load(out UserData data)
        {
            data = default;
            string path = new StringBuilder().Append(DataPath).Append(nameof(UserData)).Append(".json").ToString();
            if (!File.Exists(path)) return false;
            string jsonData = File.ReadAllText(path);
            //  TODO: JsonからUserDataクラスへの変換に失敗した時の処理を書いてないのでいつか書きます。
            data = JsonUtility.FromJson<UserData>(jsonData);
            return true;
        }
    }
}
import subprocess
import json

# 実行ファイルパス
EXE_PATH = "./a"
# jsonファイルパス
JSON_PATH = "./test_info.json"


## jsonファイルの読み込み関数 ##
def read_json():
    with open(JSON_PATH, "r") as f:
        read_data = json.load(f)

    return read_data


## テスト実行関数 ##
def test_proc(json_data):
    for info in json_data["test_info"]:
        test_name = info["test_name"]
        target_console = info["console_no"]

        # 元の内容を保持（使い回し対策）
        with open(test_name, "r", encoding="cp932") as f:
            original_lines = f.readlines()

        new_lines = []
        for line in original_lines:
            stripped = line.strip()

            # 行の途中に console no : が含まれているか
            if "console no :" in stripped:
                # iniファイルからコンソール番号抽出
                no = int(stripped.split(":")[1].strip())

                # 抽出した番号とJSONで指定した番号の比較
                if no == target_console:
                    # 一致する番号 → コメント解除
                    new_lines.append(f"console no : {no}\n")
                else:
                    # 一致しない番号 → コメント化
                    new_lines.append(f"# console no : {no}\n")
            else:
                # console no : が含まれていない行はそのまま
                new_lines.append(line)

        # 加工した内容を書き込み
        with open(test_name, "w") as f:
            f.writelines(new_lines)

        print("CMD:", EXE_PATH, test_name, "(console_no:", target_console, ")")
        result = subprocess.run([EXE_PATH, test_name])
        print("return code:", result.returncode)
        print("-" * 40)

        # 実行後に元に戻す（使い回し対策）
        with open(test_name, "w") as f:
            f.writelines(original_lines)

def main_proc():
    # jsonファイルの読み込み
    json_data = read_json()

    # テスト実行
    test_proc(json_data)

if __name__ == "__main__":
    main_proc()

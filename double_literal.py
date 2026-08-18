import os
import re
import argparse

# ----------------------------------------
# 実数抽出用の正規表現（float除外）
# ----------------------------------------
float_literal = re.compile(r"""
    (?<![A-Za-z0-9_])          # 前が識別子でない
    \d+\.\d+                   # 小数点を含む数値（小数部は全部読む）
    (?![A-Za-z0-9_]*[fF])      # 末尾が f/F で終わるものを除外
""", re.VERBOSE)

# ----------------------------------------
# コメント空白化（/* ... */ と // ... を空白に置換）
# ----------------------------------------
def blank_comments(code: str) -> str:
    result = []
    in_block = False

    for line in code.splitlines():
        i = 0
        new_line = ""
        while i < len(line):
            if not in_block and line.startswith("/*", i):
                in_block = True
                new_line += "  "
                i += 2
                continue

            if in_block and line.startswith("*/", i):
                in_block = False
                new_line += "  "
                i += 2
                continue

            if in_block:
                new_line += " "
                i += 1
                continue

            if line.startswith("//", i):
                new_line += " " * (len(line) - i)
                break

            new_line += line[i]
            i += 1

        result.append(new_line)

    return "\n".join(result)

# ----------------------------------------
# 1ファイル内の実数抽出（行番号付き）
# ----------------------------------------
def grep_double_lines(code: str):
    cleaned = blank_comments(code)
    hits = []

    for lineno, (original_line, cleaned_line) in enumerate(
        zip(code.splitlines(), cleaned.splitlines()), start=1
    ):
        m = float_literal.search(cleaned_line)
        if m:
            hits.append((lineno, m.group(), original_line))

    return hits

# ----------------------------------------
# ディレクトリ内のすべての .c / .h ファイルを検索
# ----------------------------------------
def search_directory(root_dir):
    results = []

    for dirpath, dirnames, filenames in os.walk(root_dir):
        for filename in filenames:
            # .c と .h のみ
            if not filename.endswith((".c", ".h")):
                continue

            fullpath = os.path.join(dirpath, filename)
            abspath = os.path.abspath(fullpath)

            try:
                with open(fullpath, "r", encoding="utf-8") as f:
                    code = f.read()
            except Exception as e:
                print(f"読み込み失敗: {abspath} ({e})")
                continue

            hits = grep_double_lines(code)
            if hits:
                results.append((abspath, hits))

    return results

# ----------------------------------------
# メイン：引数でディレクトリ指定
# ----------------------------------------
def main():
    parser = argparse.ArgumentParser(description="実数抽出プログラム")
    parser.add_argument("directory", help="検索対象ディレクトリ")
    args = parser.parse_args()

    results = search_directory(args.directory)

    for filepath, hits in results:
        for lineno, value, line in hits:
            print(f"{filepath}:{lineno}:{line}")

if __name__ == "__main__":
    main()

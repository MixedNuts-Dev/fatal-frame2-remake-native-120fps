# Native 120FPS Option

**FATAL FRAME II: Crimson Butterfly REMAKE** 用の Mod です。
A mod for FATAL FRAME / PROJECT ZERO II: Crimson Butterfly REMAKE.

Created by MixedNuts

---

# 日本語

## これは何か

ゲーム内のグラフィック設定で、最大フレームレートに **120** を選べるようにします。
標準では 30 と 60 の 2 つしか選べません。

ゲーム本体には 120FPS で動作する機能が最初から備わっています。メニュー側の処理が
選択肢を 2 つに固定しているだけなので、そこを解除しています。

## 動作環境

- FATAL FRAME II: Crimson Butterfly REMAKE（Steam 版）
- 120Hz 以上に対応したディスプレイ
- 120FPS を維持できる PC 性能

ゲームのファイルは一切変更しないため、Steam のファイル整合性チェックに
引っかかることはありません。

## 同梱ファイル

| ファイル | 役割 |
|---|---|
| `dinput8.dll` | ローダー |
| `Mods\native120fps\native120fps.dll` | 本体 |
| `Mods\native120fps\native120fps.ini` | 設定ファイル |
| `Mods\native120fps\README.md` | このファイル |

このうち **`dinput8.dll` と `Mods` フォルダの 2 つ**をコピーします。

## 導入方法

1. ゲームを終了します

2. 同梱の `dinput8.dll` と `Mods` フォルダを、ゲームのルートディレクトリ
   （`FatalFrameII.exe` と同じ場所）にそのままコピーします

   ```
   ...\steamapps\common\FatalFrameII\FatalFrameII.exe
   ...\steamapps\common\FatalFrameII\dinput8.dll         ← 追加
   ...\steamapps\common\FatalFrameII\Mods\native120fps\  ← 追加
   ```

   ゲームフォルダの開き方：Steam ライブラリでタイトルを右クリック →
   **管理** → **ローカルファイルを閲覧**

3. ゲームを起動します

4. **オプション → 画面設定 → 最大FPS** を開きます。
   選択肢が **「30 / 60 / 120」** の 3 つになっています

5. **「120」を選び、タイトル画面まで戻る**と反映されます
   （この設定はタイトルに戻ったタイミングで適用されます）

6. 併せて **Vsync（垂直同期）を無効**にしてください。
   有効のままだとモニタのリフレッシュレートで頭打ちになります

## 削除方法

`dinput8.dll` と `Mods` フォルダを削除するだけです。
ゲームのファイルは一切変更していないため、完全に元に戻ります。

一時的に無効化したい場合は、`native120fps.ini` の `Enabled` を `0` にしてください。
ファイルを消さずに素の状態で起動できます。

## 設定ファイル

`native120fps.ini` で次の項目を変更できます。

| 項目 | 意味 |
|---|---|
| `Enabled` | `1` = 有効 / `0` = 無効 |
| `Log` | `1` = ログを出力 / `0` = 出力しない |

`LabelId` という項目もありますが、これは内部的な値なので通常は変更する必要はありません。

## 免責事項

**この Mod は無保証で提供されます。使用によって生じたいかなる損害についても、
作者は一切の責任を負いません。** セーブデータの破損・消失、ゲームの動作不良、
その他の不具合を含みます。自己責任でご使用ください。

**導入前に、必ずセーブデータとゲームフォルダのバックアップを取ってください。**
セーブデータの場所は次のとおりです。

```
%LOCALAPPDATA%\KoeiTecmo\FatalFrameII\Savedata\
```

## 注意事項

- 120FPS を出すには相応の PC 性能と、120Hz 以上に対応したディスプレイが必要です
- 垂直同期を切らないとモニタのリフレッシュレートで頭打ちになります
- ゲームのアップデート後に動かなくなることがあります。その場合は Mod の更新をお待ちください
- ウイルス対策ソフトが誤検知することがあります。他プロセスのメモリを書き換える
  仕組みのためで、この Mod はネットワーク通信もファイル改変も行いません

## うまく動かないとき

選択肢が 2 つのままの場合、次を順に確認してください。

1. `dinput8.dll` がゲームのルート（`FatalFrameII.exe` と同じ場所）にあるか。
   **`Mods` フォルダの中ではありません**
2. `Mods\native120fps\` の中に `native120fps.dll` があるか。
   フォルダ名・ファイル名を変更していないか
3. `native120fps.ini` の `Enabled` が `1` になっているか
4. `Mods\native120fps\` に `native120fps.log` が生成されているか。
   生成されていなければ `dinput8.dll` が読み込まれていません

ログに次の行が出ていれば正常に適用されています。

```
[OK] メニューハンドラを解除
[OK] 選択肢を 3 つに拡張
[OK] ラベルを "120" に差し替え
=== 完了 ===
```

なお Mod は起動から数秒かけて適用されます。**タイトル画面まで進んでから**
オプションを開いてください。

## 不具合の報告

不具合を見つけた場合は、GitHub の Issue でご報告ください。その際、**必ず
`native120fps.log` を添付してください。** ログが無いと原因を特定できず、
対応できない場合があります。

https://github.com/MixedNuts-Dev/fatal-frame2-remake-native-120fps/issues

併せて、次の情報をいただけると助かります。

- ゲームのバージョン
- GPU とディスプレイのリフレッシュレート
- 発生した状況（どの画面で、何をしたとき）

## 仕組み

ゲームのファイルは変更しません。すべて実行時のメモリ操作です。

本体の実行ファイルは Steam の DRM で保護されており、ディスク上ではコード部分が
暗号化されています。そのためファイルを直接書き換えることができず、起動後に
復号されたメモリへパッチを当てています。

行っていることは次の 3 つだけです。

1. メニューの最大フレームレート項目が、選択インデックス 0 と 1 しか受け付けない
   ハードコードを解除する（8 バイト）
2. 選択肢の定義テーブルを 2 択から 3 択に拡張する
3. 未使用のまま残っていた文字列枠を「120」に書き換え、3 つ目のラベルとして使う

パッチの適用が終わると走査は完全に停止し、以降ゲームには一切触れません。

---

# English

## What this does

Adds **120** as a selectable option for the maximum frame rate in the in-game
graphics settings. By default only 30 and 60 are available.

The game already supports running at 120 FPS internally. The menu handler simply
hardcodes the choice to two entries, and this mod removes that restriction.

## Requirements

- FATAL FRAME II: Crimson Butterfly REMAKE (Steam)
- A display capable of 120Hz or higher
- A PC able to sustain 120 FPS

No game files are modified, so this will not trip Steam's file integrity verification.

## What's included

| File | Role |
|---|---|
| `dinput8.dll` | loader |
| `Mods\native120fps\native120fps.dll` | the mod itself |
| `Mods\native120fps\native120fps.ini` | configuration |
| `Mods\native120fps\README.md` | this file |

You copy two things: **`dinput8.dll` and the `Mods` folder.**

## Installation

1. Close the game.

2. Copy `dinput8.dll` and the `Mods` folder into the game's root directory
   (the folder containing `FatalFrameII.exe`).

   ```
   ...\steamapps\common\FatalFrameII\FatalFrameII.exe
   ...\steamapps\common\FatalFrameII\dinput8.dll         <- added
   ...\steamapps\common\FatalFrameII\Mods\native120fps\  <- added
   ```

   To open the game folder: right-click the title in your Steam library →
   **Manage** → **Browse local files**

3. Launch the game.

4. Open **Options → Screen Settings → Max FPS**.
   The option now has three entries: **30 / 60 / 120**.

5. **Select 120 and return to the title screen** to apply it.
   (This setting takes effect when you return to the title screen.)

6. Also **turn V-Sync off.** With it on, the frame rate is capped at your
   monitor's refresh rate.

## Uninstallation

Simply delete `dinput8.dll` and the `Mods` folder. No game files are modified,
so removal restores the original state completely.

To disable temporarily without deleting anything, set `Enabled` to `0` in
`native120fps.ini`.

## Configuration

`native120fps.ini` exposes the following:

| Key | Meaning |
|---|---|
| `Enabled` | `1` = on / `0` = off |
| `Log` | `1` = write a log file / `0` = no log |

There is also a `LabelId` entry. It is an internal value and normally does not
need to be changed.

## Disclaimer

**This mod is provided as-is, without any warranty. The author accepts no liability
for any damage arising from its use,** including but not limited to corruption or
loss of save data, game malfunction, or any other problem. Use it at your own risk.

**Before installing, always back up your save data and the game folder.**
Save data is located at:

```
%LOCALAPPDATA%\KoeiTecmo\FatalFrameII\Savedata\
```

## Notes

- Running at 120 FPS requires sufficient PC performance and a display capable of
  120Hz or higher
- Turn V-Sync off, otherwise the frame rate is capped at your monitor's refresh rate
- A game update may break this mod. Please wait for an updated release if that happens
- Antivirus software may flag this mod. It writes to the memory of another process,
  which is a common false-positive trigger. This mod performs no network activity
  and modifies no files

## If it doesn't work

If the option still shows only two entries, check the following:

1. Is `dinput8.dll` in the game's root folder (next to `FatalFrameII.exe`)?
   **It does not go inside the `Mods` folder**
2. Is `native120fps.dll` present in `Mods\native120fps\`?
   Have the folder or file names been changed?
3. Is `Enabled` set to `1` in `native120fps.ini`?
4. Has `native120fps.log` been created in `Mods\native120fps\`?
   If not, `dinput8.dll` is not being loaded at all

If the log contains these lines, the patch was applied correctly:

```
[OK] メニューハンドラを解除
[OK] 選択肢を 3 つに拡張
[OK] ラベルを "120" に差し替え
=== 完了 ===
```

Note that the mod takes a few seconds after launch to apply. **Reach the title
screen** before opening the options menu.

## Reporting issues

If you run into a problem, please open a GitHub Issue. **Be sure to attach
`native120fps.log`.** Without the log the cause usually cannot be identified,
and the issue may not be actionable.

https://github.com/MixedNuts-Dev/fatal-frame2-remake-native-120fps/issues

The following details also help:

- Game version
- GPU and display refresh rate
- What you were doing when it happened

## How it works

No game files are modified. Everything is done in memory at runtime.

The game executable is protected by Steam DRM and its code section is encrypted on
disk, so it cannot be patched as a file. The patch is applied to the decrypted code
in memory after the game starts.

Only three changes are made:

1. Remove the hardcoded check in the frame rate menu handler that accepts only
   selection index 0 and 1 (8 bytes).
2. Extend the choice definition table from two entries to three.
3. Rewrite an unused placeholder string to `120` and use it as the label for the
   third choice.

Once the patch is applied, scanning stops completely and the mod no longer touches
the game.

---

## License

MIT License — Copyright (c) 2026 MixedNuts

本ソフトウェアは MIT ライセンスで提供されます。再配布・改変は自由ですが、
著作権表示とライセンス文を必ず残してください。

This software is provided under the MIT License. You are free to redistribute and
modify it, but the copyright notice and the license text must be retained.

https://github.com/MixedNuts-Dev/fatal-frame2-remake-native-120fps

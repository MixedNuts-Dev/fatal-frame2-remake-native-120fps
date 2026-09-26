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
- 空きディスク容量 約 10MB（初回起動時に作業用ファイルを生成します）

ゲームのファイルは一切変更しないため、Steam のファイル整合性チェックに
引っかかることはありません。

### 対応言語

**公式にサポートするのは日本語と英語です。** この 2 つは動作を確認しています。

その他の言語でも 3 つ目の選択肢は選べますが、ラベルの表示までは保証しません。
イタリア語では 3 つ目のラベルに無関係な文字列が表示されます（選ぶこと自体は
でき、120FPS でも正常に動作します）。

## 同梱ファイル

| ファイル | 役割 |
|---|---|
| `dinput8.dll` | ローダー |
| `Mods\native120fps\native120fps.dll` | 本体 |
| `Mods\native120fps\native120fps.ini` | 設定ファイル |
| `Mods\native120fps\README.md` | このファイル |

このうち **`dinput8.dll` と `Mods` フォルダの 2 つ**をコピーします。

初回起動時に、Mod が `Mods\native120fps\` の中へ次のファイルを自動生成します。
どちらも削除して問題ありません（次回起動時に作り直されます）。

| ファイル | 役割 |
|---|---|
| `native120fps.log` | ログ |
| `archive_06.lnk` | 表示ラベル差し替え用の作業ファイル（約 9MB） |
| `archive_06.lnk.tag` | 上記の世代管理用 |

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
自動生成されたファイルも `Mods` フォルダの中にしかないので、一緒に消えます。

一時的に無効化したい場合は、`native120fps.ini` の `Enabled` を `0` にしてください。
ファイルを消さずに素の状態で起動できます。

## 設定ファイル

`native120fps.ini` で次の項目を変更できます。

| 項目 | 意味 |
|---|---|
| `Enabled` | `1` = 有効 / `0` = 無効 |
| `Log` | `1` = ログを出力 / `0` = 出力しない |
| `Diagnose` | `1` = 不具合報告用の追加診断を出力 / `0` = 出力しない（既定） |

`Diagnose` は、**不具合を報告するときだけ** `1` にしてください。パッチ後 15 分間、
ゲーム側のフレームレート番号を監視してログに記録します。読み取るのは 1 バイト
だけで、ゲームへの書き込みは一切行いません。

`LabelId` と `FpsIndexRva` という項目もありますが、これらは内部的な値なので
通常は変更する必要はありません。

## 免責事項

**この Mod は無保証で提供されます。使用によって生じたいかなる損害についても、
作者は一切の責任を負いません。** セーブデータの破損・消失、ゲームの動作不良、
その他の不具合を含みます。自己責任でご使用ください。

**導入前に、必ずセーブデータとゲームフォルダのバックアップを取ってください。**
セーブデータの場所は次のとおりです。

```
%LOCALAPPDATA%\KoeiTecmo\FatalFrameII\Savedata\
```

## 既知の問題

**120 を選んだあとに画面設定を開き直すと、カーソルが「30」に戻って見えます。**

表示だけの問題です。**実際のフレームレートは 120 のまま**で、その状態から他の
設定を変更しても 120 に戻ることはありません（実測で確認済み）。設定ファイルにも
120 が保存され続けます。

ゲーム本来の処理が、カーソル位置を復元するときに 3 つ目の選択肢を想定して
いないためです。実フレームレートは NVIDIA / AMD のオーバーレイなどで確認して
ください。

## 注意事項

- 120FPS を出すには相応の PC 性能と、120Hz 以上に対応したディスプレイが必要です
- 垂直同期を切らないとモニタのリフレッシュレートで頭打ちになります
- ゲームのアップデート後に動かなくなることがあります。その場合は Mod の更新をお待ちください
- 初回起動時に約 9MB の作業用ファイルを `Mods\native120fps\` の中に生成します。
  ゲーム側のファイルは読み取るだけで、書き換えません
- ウイルス対策ソフトが誤検知することがあります。他プロセスのメモリを書き換える
  仕組みのためで、この Mod はネットワーク通信を一切行わず、ファイルを書き込むのも
  自分のフォルダの中だけです

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
[OK] Generated patched archive_06.lnk
[OK] Menu handler unlocked
[OK] Choice table extended to 3 entries
=== Done (attempt 3). The FPS option now has three entries ===
```

ログは英語で出力されます。

なお Mod は起動から数秒かけて適用されます。**タイトル画面まで進んでから**
オプションを開いてください。

### 「120」を選んでも 30FPS のままの場合

まず上の「既知の問題」を確認してください。**カーソルが 30 に見えるだけで、実際は
120 で動いていることがあります。**オーバーレイなどで実測値を確認してください。

実測でも 30FPS だった場合、次の順に確認してください。**性能が足りていない場合は、
きりの良い数値に留まらず値が揺れます。きっちり 30 に張り付く場合は、どこかで
上限が掛かっています。**

1. `native120fps.log` の最終行が `=== Done ... ===` になっているか。
   `=== Gave up ... code: ... table: OK ===` のようになっていれば、パッチが
   片方しか当たっていません（この状態だと 3 つ目を選ぶと 30FPS になります）。
   その場合、ログに `[!!]` で始まる説明が出ています
2. NVIDIA コントロールパネルの **垂直同期**が「アダプティブ（ハーフリフレッシュ
   レート）」になっていないか。60Hz のディスプレイではちょうど 30FPS になります
3. 同じ画面の **「最大フレームレート」**が低い値に設定されていないか
4. Windows の画面設定で、リフレッシュレートが実際に 120Hz 以上になっているか
   （対応していても 60Hz のままになっていることがあります）
5. ゲーム内の **Vsync が無効**になっているか
6. 過去に `graphics_option.json` を書き換える Mod を使っていた場合、ファイルが
   **読み取り専用のまま残っていないか**。読み取り専用だと設定を保存できません
7. ムービーシーンで測っていないか（プリレンダのムービーは 30FPS が仕様です）

## 不具合の報告

不具合を見つけた場合は、GitHub の Issue でご報告ください。その際、**必ず
`native120fps.log` を添付してください。** ログが無いと原因を特定できず、
対応できない場合があります。

ログには、ゲームのバージョンと Steam のビルド番号、画面の解像度と
リフレッシュレート、GPU 名、OS のビルド、保存されている FPS 設定が
記録されます。ゲームのインストール先のパスも含まれるので、
気になる場合はその行を消してから添付してください。

https://github.com/MixedNuts-Dev/fatal-frame2-remake-native-120fps/issues

併せて、次の情報をいただけると助かります。

- ゲームのバージョン
- GPU とディスプレイのリフレッシュレート
- 発生した状況（どの画面で、何をしたとき）

フレームレートに関する不具合の場合は、`native120fps.ini` の `Diagnose` を `1` に
してから再現し、そのログを添付していただけると原因が特定しやすくなります。

## 仕組み

ゲームのファイルは変更しません。

本体の実行ファイルは Steam の DRM で保護されており、ディスク上ではコード部分が
暗号化されています。そのためファイルを直接書き換えることができず、起動後に
復号されたメモリへパッチを当てています。

行っていることは次の 3 つだけです。

1. メニューの最大フレームレート項目が、選択インデックス 0 と 1 しか受け付けない
   ハードコードを解除する（8 バイト）
2. 選択肢の定義テーブルを 2 択から 3 択に拡張する
3. 3 つ目の選択肢に「120」と表示させる

3 番目のラベルだけはメモリの書き換えでは足りません。ゲームがメッセージのかたまりを
同じ場所へ読み直すため、書き込んでもすぐ元に戻ってしまいます。そこで Mod は、
**お使いのゲームフォルダにある `archive\archive_06.lnk` を読み取り、未使用のまま
残っていた文字列枠を「120」に書き換えた複製を `Mods\native120fps\` の中に作り**、
ゲームがそちらを読むように差し替えています。ゲーム側のファイルは読むだけです。

この複製はお使いの環境で生成されるもので、配布物には改変済みのゲームデータは
一切含まれていません。

書き換える文字列枠は、空であるか既知のプレースホルダであることを確認してから
書き込みます。その枠が他の用途で使われている言語（イタリア語）では見送るため、
ゲームのテキストが壊れることはありません。

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
- About 10 MB of free disk space (a working file is generated on first launch)

No game files are modified, so this will not trip Steam's file integrity verification.

### Supported languages

**Japanese and English are officially supported**, and both are verified.

The third entry is selectable in the other languages too, but its label is not
guaranteed. In Italian the third entry shows an unrelated string — it can still be
selected and 120 FPS works correctly.

## What's included

| File | Role |
|---|---|
| `dinput8.dll` | loader |
| `Mods\native120fps\native120fps.dll` | the mod itself |
| `Mods\native120fps\native120fps.ini` | configuration |
| `Mods\native120fps\README.md` | this file |

You copy two things: **`dinput8.dll` and the `Mods` folder.**

On first launch the mod generates the following inside `Mods\native120fps\`.
Both are safe to delete; they are rebuilt on the next launch.

| File | Role |
|---|---|
| `native120fps.log` | log |
| `archive_06.lnk` | working file used to supply the third label (~9 MB) |
| `archive_06.lnk.tag` | version marker for the above |

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
so removal restores the original state completely. The generated files live only
inside the `Mods` folder, so they go with it.

To disable temporarily without deleting anything, set `Enabled` to `0` in
`native120fps.ini`.

## Configuration

`native120fps.ini` exposes the following:

| Key | Meaning |
|---|---|
| `Enabled` | `1` = on / `0` = off |
| `Log` | `1` = write a log file / `0` = no log |
| `Diagnose` | `1` = write extra diagnostics / `0` = don't (default) |

Set `Diagnose` to `1` **only when reporting a bug.** For 15 minutes after patching
it watches the game's frame rate index and logs it. It only reads a single byte and
never writes anything to the game.

There are also `LabelId` and `FpsIndexRva` entries. They are internal values and
normally do not need to be changed.

## Disclaimer

**This mod is provided as-is, without any warranty. The author accepts no liability
for any damage arising from its use,** including but not limited to corruption or
loss of save data, game malfunction, or any other problem. Use it at your own risk.

**Before installing, always back up your save data and the game folder.**
Save data is located at:

```
%LOCALAPPDATA%\KoeiTecmo\FatalFrameII\Savedata\
```

## Known issues

**After selecting 120, reopening the screen settings shows the cursor back on "30".**

This is a display issue only. **The actual frame rate stays at 120**, and changing
other settings from that state does not reset it (verified by measurement) — the
settings file keeps 120 as well.

The game's own code does not expect a third entry when it restores the cursor
position. Use an overlay (NVIDIA, AMD, Steam) to check the real frame rate.

## Notes

- Running at 120 FPS requires sufficient PC performance and a display capable of
  120Hz or higher
- Turn V-Sync off, otherwise the frame rate is capped at your monitor's refresh rate
- A game update may break this mod. Please wait for an updated release if that happens
- On first launch, about 9 MB of working data is generated inside
  `Mods\native120fps\`. The game's own files are only read, never written
- Antivirus software may flag this mod. It writes to the memory of another process,
  which is a common false-positive trigger. This mod performs no network activity,
  and the only files it writes are inside its own folder

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
[OK] Generated patched archive_06.lnk
[OK] Menu handler unlocked
[OK] Choice table extended to 3 entries
=== Done (attempt 3). The FPS option now has three entries ===
```

Note that the mod takes a few seconds after launch to apply. **Reach the title
screen** before opening the options menu.

### If you selected 120 but still get 30 FPS

First check "Known issues" above. **The cursor can look like it is on 30 while the
game is actually running at 120.** Confirm the real frame rate with an overlay.

If you really are measuring 30 FPS, check the following in order. **When a GPU
simply can't keep up, the frame rate fluctuates rather than sitting on a round
number; a rock-steady 30 means something is capping it.**

1. Does the last line of `native120fps.log` read `=== Done ... ===`? If it
   reads `=== Gave up ... code: ... table: OK ===` instead, only one of the two
   patches applied — in that state, picking the third entry does give you 30
   FPS. The log explains what happened on the lines starting with `[!!]`
2. In the NVIDIA Control Panel, is **Vertical sync** set to "Adaptive (half refresh
   rate)"? On a 60 Hz display that caps the game at exactly 30 FPS
3. On the same page, is **"Max Frame Rate"** set to a low value?
4. In Windows display settings, is the refresh rate actually set to 120 Hz or
   higher? (A display can support it while still running at 60 Hz)
5. Is **V-Sync off** in the game?
6. If you previously used a mod that edits `graphics_option.json`, is that file
   still **read-only**? The game cannot save your choice if it is
7. Were you measuring during a cutscene? Pre-rendered movies run at 30 FPS by design

## Reporting issues

If you run into a problem, please open a GitHub Issue. **Be sure to attach
`native120fps.log`.** Without the log the cause usually cannot be identified,
and the issue may not be actionable.

The log records the game version and Steam build number, your screen resolution
and refresh rate, your GPU name, the Windows build, and the frame rate value the
game has saved. It also contains the path the game is installed to — feel free to
delete that line before attaching it if you would rather not share it.

https://github.com/MixedNuts-Dev/fatal-frame2-remake-native-120fps/issues

The following details also help:

- Game version
- GPU and display refresh rate
- What you were doing when it happened

For frame rate problems, please set `Diagnose` to `1` in `native120fps.ini`,
reproduce the issue, and attach that log — it makes the cause much easier to find.

## How it works

No game files are modified.

The game executable is protected by Steam DRM and its code section is encrypted on
disk, so it cannot be patched as a file. The patch is applied to the decrypted code
in memory after the game starts.

Only three changes are made:

1. Remove the hardcoded check in the frame rate menu handler that accepts only
   selection index 0 and 1 (8 bytes).
2. Extend the choice definition table from two entries to three.
3. Make the third choice display `120`.

The third label alone cannot be handled in memory: the game reloads the message block
into the same address, so any write is quickly undone. Instead the mod **reads
`archive\archive_06.lnk` from your own game folder, writes a copy with the unused
string slot replaced by `120` into `Mods\native120fps\`**, and makes the game open
that copy instead. The game's own file is only read.

That copy is generated on your machine; no modified game data is included in the
download.

A string slot is only written when it is empty or holds the known placeholder.
Languages where the slot is already used for something else (Italian) are skipped,
so no game text is ever broken.

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

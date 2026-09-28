# Native120FPSOption

**FATAL FRAME II: Crimson Butterfly REMAKE** 用の Mod です。
A mod for FATAL FRAME / PROJECT ZERO II: Crimson Butterfly REMAKE.

Created by MixedNuts

**2.0.0 からは MixedNutsModLoader（1.0.0 以降）が必要です。** 1.x は単体で
動作していましたが、2.0.0 はローダーのプラグインになりました。
**Version 2.0.0 requires MixedNutsModLoader (1.0.0 or later).** 1.x ran on its own;
2.0.0 is a plugin for the loader.

Nexus Mods: https://www.nexusmods.com/fatalframe2crimsonbutterflyremake/mods/26
GitHub: https://github.com/MixedNuts-Dev/fatal-frame2-remake-mod-loader

---

# 日本語

## これは何か

ゲーム内のグラフィック設定で、最大フレームレートに **120** を選べるようにします。
標準では 30 と 60 の 2 つしか選べません。

ゲーム本体には 120FPS で動作する機能が最初から備わっています。メニュー側の処理が
選択肢を 2 つに固定しているだけなので、そこを解除しています。

## 動作環境

- FATAL FRAME II: Crimson Butterfly REMAKE（Steam 版）
- **MixedNutsModLoader 1.0.0 以降**（別途導入が必要です）
  Nexus Mods: https://www.nexusmods.com/fatalframe2crimsonbutterflyremake/mods/26
  GitHub: https://github.com/MixedNuts-Dev/fatal-frame2-remake-mod-loader
- 120Hz 以上に対応したディスプレイ
- 120FPS を維持できる PC 性能
- 空きディスク容量 約 20MB（初回起動時に作業用ファイルを生成します）

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
| `MixedNuts\Mods\native120fps\native120fps.dll` | 本体 |
| `MixedNuts\Mods\native120fps\native120fps.ini` | 設定ファイル |
| `MixedNuts\Mods\native120fps\README.md` | このファイル |
| `MixedNuts\Mods\native120fps\LICENSE.txt` | ライセンス |

**`dinput8.dll` は同梱していません。** ローダー（`dinput8.dll` と
`MixedNuts\MixedNutsLoader.dll`）は MixedNutsModLoader のものを使います。
この配布物からは **`MixedNuts` フォルダ**をコピーします。

起動時に、次のファイルが自動生成されます。いずれも削除して問題ありません
（次回起動時に作り直されます）。

| ファイル | 役割 |
|---|---|
| `MixedNuts\Mods\native120fps\native120fps.log` | この Mod のログ |
| `MixedNuts\cache\archive\archive_01.lnk` | 選択肢を 3 つにするための作業ファイル（約 8MB、ローダーが生成） |
| `MixedNuts\cache\archive\archive_06.lnk` | 表示ラベル差し替え用の作業ファイル（約 9MB、ローダーが生成） |

2 つの `.lnk` はローダーの共通キャッシュ `MixedNuts\cache\archive\` に置かれます。
キャッシュはゲームのファイル、導入している Mod、その設定が変わると自動で
作り直されます。

## 導入方法

1. ゲームを終了します

2. 先に **MixedNutsModLoader**（1.0.0 以降）を導入します。
   https://github.com/MixedNuts-Dev/fatal-frame2-remake-mod-loader の Releases から
   ダウンロードし、ローダーの README に従ってください

3. この配布物の `MixedNuts` フォルダを、ゲームのルートディレクトリ
   （`FatalFrameII.exe` と同じ場所）にそのままコピーします。
   ローダーの `MixedNuts` フォルダに統合されます

   ```
   ...\FatalFrameII\FatalFrameII.exe
   ...\FatalFrameII\dinput8.dll                                  ← MixedNutsModLoader
   ...\FatalFrameII\MixedNuts\MixedNutsLoader.dll                ← MixedNutsModLoader
   ...\FatalFrameII\MixedNuts\Mods\native120fps\native120fps.dll ← この Mod
   ...\FatalFrameII\MixedNuts\Mods\native120fps\native120fps.ini
   ...\FatalFrameII\MixedNuts\Mods\native120fps\native120fps.log ← 起動時に生成
   ...\FatalFrameII\MixedNuts\cache\archive\archive_01.lnk       ← 初回起動時にローダーが生成
   ...\FatalFrameII\MixedNuts\cache\archive\archive_06.lnk       ← 初回起動時にローダーが生成
   ```

   ゲームフォルダの開き方：Steam ライブラリでタイトルを右クリック →
   **管理** → **ローカルファイルを閲覧**

4. ゲームを起動します

5. **オプション → 画面設定 → 最大FPS** を開きます。
   選択肢が **「30 / 60 / 120」** の 3 つになっています

6. **「120」を選び、タイトル画面まで戻る**と反映されます
   （この設定はタイトルに戻ったタイミングで適用されます）

7. 併せて **Vsync（垂直同期）を無効**にしてください。
   有効のままだとモニタのリフレッシュレートで頭打ちになります

### 1.x から更新する場合

1. 導入前に、ゲームのルートにある古い `Mods\native120fps\` フォルダを削除します
2. ローダーを導入するときに、古い `dinput8.dll` はローダーの `dinput8.dll` で
   上書きします
3. 他の MixedNuts の Mod も 1.x を使っている場合は、まとめて更新してください
   （MouseWheelCameraSpeed = `version.dll` + `Mods\wheelspeed\`、
   TwinSwap = `xinput1_4.dll` + `Mods\twinswap\`）。詳しくはローダーの README を
   参照してください

古い 1.x の DLL がゲームのルートに残っていると、ローダーは対応する新しい Mod を
読み込まず、`MixedNuts\loader.log` に `[!!]` で始まるメッセージを書き出します。

### 他の Mod との併用

Native120FPSOption、MouseWheelCameraSpeed、TwinSwap の 2.0.0 はすべて同じ
ローダーの上で動くため、DLL を 1 つ共有し、互いに競合しません。

別の Mod がすでに `dinput8.dll` を使っている場合は、上書きしないでください。
ローダーの `dinput8.dll` は `version.dll` または `xinput1_4.dll` に名前を変えて
使えます（ローダーの README を参照してください）。

## 削除方法

`MixedNuts\Mods\native120fps\` フォルダを削除するだけです。
ゲームのファイルは一切変更していないため、完全に元に戻ります。

ローダーは他の Mod のために残しておいて構いません。すべて取り除く場合は、
ローダー（`dinput8.dll` と `MixedNuts` フォルダ）も削除してください。
自動生成された作業用ファイルも `MixedNuts` フォルダの中にしかないので、一緒に消えます。

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
- 初回起動時に約 17MB の作業用ファイルが `MixedNuts\cache\archive\` の中に生成されます。
  ゲーム側のファイルは読み取るだけで、書き換えません
- ウイルス対策ソフトが誤検知することがあります。他プロセスのメモリを書き換える
  仕組みのためで、この Mod はネットワーク通信を一切行わず、ファイルを書き込むのも
  `MixedNuts` フォルダの中（Mod 自身のフォルダとローダーのキャッシュ）だけです

## うまく動かないとき

選択肢が 2 つのままの場合、次を順に確認してください。

1. MixedNutsModLoader が導入されているか。`dinput8.dll` がゲームのルート
   （`FatalFrameII.exe` と同じ場所）にあり（**`MixedNuts` フォルダの中ではありません**）、
   `MixedNuts\MixedNutsLoader.dll` があるか
2. `MixedNuts\loader.log` が生成されていて、`[OK] native120fps: loaded` の行があるか。
   - `loader.log` 自体が無ければ、ローダーが読み込まれていません
   - その行が無ければ、フォルダ名・ファイル名が
     `MixedNuts\Mods\native120fps\native120fps.dll` になっているか確認し、
     `loader.log` に `[!!]` や `[NG]` で始まる行が無いか見てください
3. `native120fps.ini` の `Enabled` が `1` になっているか
4. `MixedNuts\Mods\native120fps\` に `native120fps.log` が生成されているか

`native120fps.log` に次の行が出ていれば正常に適用されています。

```
[OK] Menu handler unlocked (RVA ..., exact signature)
=== Done (attempt N). The FPS option now has three entries ===
```

キャッシュを作る（作り直す）起動では、次の 2 行も出ます。

```
[OK] Generated patched archive\archive_01.lnk (...)
[OK] Generated patched archive\archive_06.lnk (...)
```

2 回目以降の起動ではローダーがキャッシュを再利用するため、この 2 行は出ません。
その場合は `MixedNuts\loader.log` に `[OK] Using the cached files (N)` と出ています。
また、`loader.log` に `[OK] native120fps: loaded (2 file patches)` があれば、
Mod はローダーに読み込まれています。

ログは英語で出力されます。行の順序は前後することがあります（アーカイブの
生成は、ゲームがそのファイルを開いた時点で行われます）。

なお、選択肢が増えるのはゲームがアーカイブを読み込む起動処理の中なので、
**タイトル画面まで進んでから**オプションを開いてください。

### 「120」を選んでも 30FPS のままの場合

まず上の「既知の問題」を確認してください。**カーソルが 30 に見えるだけで、実際は
120 で動いていることがあります。**オーバーレイなどで実測値を確認してください。

実測でも 30FPS だった場合、次の順に確認してください。**性能が足りていない場合は、
きりの良い数値に留まらず値が揺れます。きっちり 30 に張り付く場合は、どこかで
上限が掛かっています。**

1. `native120fps.log` に `=== Done ... ===` の行があるか。
   `=== Gave up (N attempts / N ms, code: ...) ===` のあとに
   `[!!] The code patch did NOT apply. ...` と出ていれば、コードパッチが
   当たっていません。この状態でも 3 つ目の選択肢は表示されますが、選ぶと
   30FPS になります
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
`MixedNuts\Mods\native120fps\native120fps.log` と `MixedNuts\loader.log` の 2 つを
添付してください。** ログが無いと原因を特定できず、対応できない場合があります。

`native120fps.log` には、ゲームのバージョンと Steam のビルド番号、画面の解像度と
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

行っていることは次の 3 つだけです。

1. メニューの最大フレームレート項目が、選択インデックス 0 と 1 しか受け付けない
   ハードコードを解除する（8 バイト）
2. 選択肢の定義テーブルを 2 択から 3 択に拡張する
3. 3 つ目の選択肢に「120」と表示させる

### 1 はメモリへの書き込み

本体の実行ファイルは Steam の DRM で保護されており、ディスク上ではコード部分が
暗号化されています。そのためファイルを直接書き換えることができず、起動後に
復号されたメモリへパッチを当てています。**書き込むのは 8 バイトだけ**です。

### 2 と 3 はファイルの差し替え

こちらはアーカイブの中のデータなので、メモリの書き換えでは足りません。ゲームが
そのかたまりを同じ場所へ読み直すため、書き込んでもすぐ元に戻ってしまいます。

そこで Mod は、この 2 つの書き換えをローダーに登録しています。ゲームが各アーカイブを
初めて開くとき、ローダーは**お使いのゲームフォルダにある現在の内容を Mod に渡して
必要な箇所だけを書き換えさせ、その複製を `MixedNuts\cache\archive\` から**ゲームに
読ませます。ゲーム側のファイルは読むだけです。

| ファイル | 書き換える内容 |
|---|---|
| `archive\archive_01.lnk` | 選択肢の数を 2 から 3 にし、3 つ目に未使用の文字列 ID を入れる |
| `archive\archive_06.lnk` | 未使用のまま残っていた文字列枠を「120」にする |

この複製はお使いの環境で生成されるもので、配布物には改変済みのゲームデータは
一切含まれていません。

書き換える場所は、決まった位置を指定するのではなく**内容から探し、裏取りしてから
書き込みます。**選択肢の表は、想定どおりの並びになっていることを確認できた箇所が
ちょうど 1 つでなければ中止します。文字列枠は、空であるか既知のプレースホルダで
あることを確認してから書き込み、その枠が他の用途で使われている言語（イタリア語）
では見送ります。**そのため、ゲームのテキストが壊れることはありません。**

パッチの適用が終わると Mod は完全に停止し、以降ゲームには一切触れません。

---

# English

## What this does

Adds **120** as a selectable option for the maximum frame rate in the in-game
graphics settings. By default only 30 and 60 are available.

The game already supports running at 120 FPS internally. The menu handler simply
hardcodes the choice to two entries, and this mod removes that restriction.

## Requirements

- FATAL FRAME II: Crimson Butterfly REMAKE (Steam)
- **MixedNutsModLoader 1.0.0 or later** (installed separately)
  Nexus Mods: https://www.nexusmods.com/fatalframe2crimsonbutterflyremake/mods/26
  GitHub: https://github.com/MixedNuts-Dev/fatal-frame2-remake-mod-loader
- A display capable of 120Hz or higher
- A PC able to sustain 120 FPS
- About 20 MB of free disk space (working files are generated on first launch)

No game files are modified, so this will not trip Steam's file integrity verification.

### Supported languages

**Japanese and English are officially supported**, and both are verified.

The third entry is selectable in the other languages too, but its label is not
guaranteed. In Italian the third entry shows an unrelated string — it can still be
selected and 120 FPS works correctly.

## What's included

| File | Role |
|---|---|
| `MixedNuts\Mods\native120fps\native120fps.dll` | the mod itself |
| `MixedNuts\Mods\native120fps\native120fps.ini` | configuration |
| `MixedNuts\Mods\native120fps\README.md` | this file |
| `MixedNuts\Mods\native120fps\LICENSE.txt` | license |

**`dinput8.dll` is not included.** The loader files (`dinput8.dll` and
`MixedNuts\MixedNutsLoader.dll`) come from MixedNutsModLoader. From this download
you copy **the `MixedNuts` folder.**

The following files are generated at launch. All of them are safe to delete; they
are rebuilt on the next launch.

| File | Role |
|---|---|
| `MixedNuts\Mods\native120fps\native120fps.log` | this mod's log |
| `MixedNuts\cache\archive\archive_01.lnk` | working file that gives the option a third entry (~8 MB, generated by the loader) |
| `MixedNuts\cache\archive\archive_06.lnk` | working file that supplies the third label (~9 MB, generated by the loader) |

The two `.lnk` files live in the loader's shared cache, `MixedNuts\cache\archive\`.
The cache is rebuilt automatically when the game's files, the installed mods or
their settings change.

## Installation

1. Close the game.

2. First install **MixedNutsModLoader** (1.0.0 or later). Download it from the
   Releases of https://github.com/MixedNuts-Dev/fatal-frame2-remake-mod-loader
   and follow the loader's README.

3. Copy this download's `MixedNuts` folder into the game's root directory
   (the folder containing `FatalFrameII.exe`). It merges into the loader's
   `MixedNuts` folder.

   ```
   ...\FatalFrameII\FatalFrameII.exe
   ...\FatalFrameII\dinput8.dll                                  <- MixedNutsModLoader
   ...\FatalFrameII\MixedNuts\MixedNutsLoader.dll                <- MixedNutsModLoader
   ...\FatalFrameII\MixedNuts\Mods\native120fps\native120fps.dll <- this mod
   ...\FatalFrameII\MixedNuts\Mods\native120fps\native120fps.ini
   ...\FatalFrameII\MixedNuts\Mods\native120fps\native120fps.log <- generated at launch
   ...\FatalFrameII\MixedNuts\cache\archive\archive_01.lnk       <- generated on first launch (by the loader)
   ...\FatalFrameII\MixedNuts\cache\archive\archive_06.lnk       <- generated on first launch (by the loader)
   ```

   To open the game folder: right-click the title in your Steam library →
   **Manage** → **Browse local files**

4. Launch the game.

5. Open **Options → Screen Settings → Max FPS**.
   The option now has three entries: **30 / 60 / 120**.

6. **Select 120 and return to the title screen** to apply it.
   (This setting takes effect when you return to the title screen.)

7. Also **turn V-Sync off.** With it on, the frame rate is capped at your
   monitor's refresh rate.

### Upgrading from 1.x

1. Before installing, delete the old `Mods\native120fps\` folder from the game root.
2. When you install the loader, overwrite the old `dinput8.dll` with the loader's
   `dinput8.dll`.
3. If you also use the other MixedNuts mods at 1.x (MouseWheelCameraSpeed =
   `version.dll` + `Mods\wheelspeed\`, TwinSwap = `xinput1_4.dll` + `Mods\twinswap\`),
   update them all at once. See the loader's README for details.

If an old 1.x DLL is still in the game root, the loader does not load the
corresponding new mod and writes a message starting with `[!!]` to
`MixedNuts\loader.log`.

### Using with other mods

Native120FPSOption, MouseWheelCameraSpeed and TwinSwap 2.0.0 all run on the same
loader, so they share one DLL and never conflict.

If another mod already uses `dinput8.dll`, do not overwrite it. The loader's
`dinput8.dll` can be renamed to `version.dll` or `xinput1_4.dll` (see the loader's
README).

## Uninstallation

Simply delete the `MixedNuts\Mods\native120fps\` folder. No game files are modified,
so removal restores the original state completely.

The loader can stay for other mods. To remove everything, delete the loader too
(`dinput8.dll` and the `MixedNuts` folder). The generated working files live only
inside the `MixedNuts` folder, so they go with it.

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
- On first launch, about 17 MB of working data is generated inside
  `MixedNuts\cache\archive\`. The game's own files are only read, never written
- Antivirus software may flag this mod. It writes to the memory of another process,
  which is a common false-positive trigger. This mod performs no network activity,
  and the only files written are inside the `MixedNuts` folder (the mod's own folder
  and the loader's cache)

## If it doesn't work

If the option still shows only two entries, check the following:

1. Is MixedNutsModLoader installed? `dinput8.dll` must be in the game's root
   folder (next to `FatalFrameII.exe`, **not inside the `MixedNuts` folder**), and
   `MixedNuts\MixedNutsLoader.dll` must be present
2. Does `MixedNuts\loader.log` exist, and does it have the
   `[OK] native120fps: loaded` line?
   - If `loader.log` is missing, the loader is not being loaded at all
   - If the line is missing, check that the folder and file names are
     `MixedNuts\Mods\native120fps\native120fps.dll`, and look for lines starting
     with `[!!]` or `[NG]` in `loader.log`
3. Is `Enabled` set to `1` in `native120fps.ini`?
4. Has `native120fps.log` been created in `MixedNuts\Mods\native120fps\`?

If `native120fps.log` contains these lines, the patch was applied correctly:

```
[OK] Menu handler unlocked (RVA ..., exact signature)
=== Done (attempt N). The FPS option now has three entries ===
```

On a launch where the cache is built (or rebuilt), these two lines appear as well:

```
[OK] Generated patched archive\archive_01.lnk (...)
[OK] Generated patched archive\archive_06.lnk (...)
```

On later launches the loader reuses the cache, so those two lines do not appear;
`MixedNuts\loader.log` shows `[OK] Using the cached files (N)` instead. If
`loader.log` has `[OK] native120fps: loaded (2 file patches)`, the loader has
loaded the mod.

The order of those lines can vary: each archive is generated at the moment the
game first opens that file.

Note that the third entry is added while the game loads its archives during
startup, so **reach the title screen** before opening the options menu.

### If you selected 120 but still get 30 FPS

First check "Known issues" above. **The cursor can look like it is on 30 while the
game is actually running at 120.** Confirm the real frame rate with an overlay.

If you really are measuring 30 FPS, check the following in order. **When a GPU
simply can't keep up, the frame rate fluctuates rather than sitting on a round
number; a rock-steady 30 means something is capping it.**

1. Does `native120fps.log` have the `=== Done ... ===` line? If it shows
   `=== Gave up (N attempts / N ms, code: ...) ===` followed by
   `[!!] The code patch did NOT apply. ...`, the code patch did not apply. The
   third entry still appears in the menu in that state, but selecting it gives
   you 30 FPS
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

If you run into a problem, please open a GitHub Issue. **Be sure to attach both
`MixedNuts\Mods\native120fps\native120fps.log` and `MixedNuts\loader.log`.**
Without the logs the cause usually cannot be identified, and the issue may not be
actionable.

`native120fps.log` records the game version and Steam build number, your screen resolution
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

Only three changes are made:

1. Remove the hardcoded check in the frame rate menu handler that accepts only
   selection index 0 and 1 (8 bytes).
2. Extend the choice definition table from two entries to three.
3. Make the third choice display `120`.

### Change 1 is written to memory

The game executable is protected by Steam DRM and its code section is encrypted on
disk, so it cannot be patched as a file. The patch is applied to the decrypted code
in memory after the game starts. **Only 8 bytes are written.**

### Changes 2 and 3 are a file redirect

Those two live inside the game's archives, and writing to memory is not enough: the
game reloads those blocks into the same address, so any write is quickly undone.

Instead the mod registers those two changes with the loader. When the game first
opens each archive, the loader **has the mod change just the necessary bytes in the
current contents from your own game folder, and serves the result from
`MixedNuts\cache\archive\`.** The game's own files are only read.

| File | What is changed |
|---|---|
| `archive\archive_01.lnk` | the choice count goes from 2 to 3, and an unused string ID is put in the third entry |
| `archive\archive_06.lnk` | an unused string slot becomes `120` |

Those copies are generated on your machine; no modified game data is included in the
download.

Targets are located **by content rather than by fixed offsets, and verified before
anything is written.** For the choice table, the mod aborts unless there is exactly
one place that matches the expected layout. A string slot is only written when it is
empty or holds the known placeholder, and languages where it is already used for
something else (Italian) are skipped. **No game text is ever broken.**

Once the patch is applied the mod stops completely and no longer touches the game.

---

## License

MIT License — Copyright (c) 2026 MixedNuts

本ソフトウェアは MIT ライセンスで提供されます。再配布・改変は自由ですが、
著作権表示とライセンス文を必ず残してください。

This software is provided under the MIT License. You are free to redistribute and
modify it, but the copyright notice and the license text must be retained.

https://github.com/MixedNuts-Dev/fatal-frame2-remake-native-120fps

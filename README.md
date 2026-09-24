# Native 120FPS Option

**FATAL FRAME II: Crimson Butterfly REMAKE**（零 〜紅い蝶〜 REMAKE / Steam AppID 3920610）用の Mod です。
A mod for FATAL FRAME / PROJECT ZERO II: Crimson Butterfly REMAKE (Steam, AppID 3920610).

ゲーム内のグラフィック設定で、最大フレームレートに **120** を選べるようにします。
標準では 30 と 60 の 2 つしか選べません。

Adds **120** to the maximum frame rate setting in the in-game graphics options.
By default only 30 and 60 are selectable.

**ゲーム本体は最初から 120FPS で動作する機能を持っています。** メニュー側の処理が
選択肢を 2 つに固定しているだけなので、そこを解除しています。

**The game already supports 120 FPS internally.** The menu handler simply hardcodes
the choice list to two entries, and this mod removes that restriction.

## 他の jsonを直接書き換える場合との違い / How this differs from editing the json directly

設定ファイル（`graphics_option.json`）を直接書き換える方法とは、次の点が異なります。
This differs from editing the `graphics_option.json` settings file by hand:

- **ゲーム内のオプション画面に「120」が選択肢として現れます。** 設定ファイルを
  直接編集する必要はなく、他の設定と同じようにメニューから選べます。
  **"120" appears as a real option in the in-game settings screen.** No manual file
  editing — you pick it from the menu like any other setting.
- **ゲームのファイルを一切変更しません。** すべて実行時のメモリ操作で、
  導入した 2 つを消せば完全に元へ戻ります。
  **No game files are modified at all.** Everything happens in memory at runtime;
  deleting the two added items restores the original state completely.
- 直接書き換える方法では、ゲーム内でグラフィック設定を変更するたびに値が
  上書きされてしまいますが、この Mod ではその問題が起きません。
  When the file is edited by hand, the value is overwritten whenever you change any
  graphics setting in-game. That does not happen here.

## 導入 / Installation

`dist` をビルドし、`dinput8.dll` と `Mods` フォルダをゲームのルート
（`FatalFrameII.exe` と同じ場所）にコピーします。

Build `dist`, then copy `dinput8.dll` and the `Mods` folder into the game's root
directory (the folder containing `FatalFrameII.exe`).

```
FatalFrameII/
  FatalFrameII.exe
  dinput8.dll                          <- added
  Mods/native120fps/native120fps.dll   <- added
  Mods/native120fps/native120fps.ini
  Mods/native120fps/README.txt
```

ゲームを起動し、オプション → グラフィック設定を開くと、最大 FPS が 3 択になります。
3 つ目を選び、タイトル画面まで戻ると反映されます。

Launch the game and open Options → Graphics Settings; the max FPS option now has three
entries. Select the third one and return to the title screen to apply it.

削除は 2 つを消すだけです。 / To uninstall, just delete them.

## ビルド / Build

Visual Studio 2022 の C++ ツールセットが必要です。
Requires the Visual Studio 2022 C++ toolset.

```
build.bat
```

`dist\` に配布用の一式が出力されます。 / The distributable set is written to `dist\`.

## 仕組み / How it works

実行ファイルは Steam DRM により `.text` が暗号化されているため、**ファイルへの静的
パッチはできません。** 起動後に復号されたメモリへ実行時にパッチを当てています。

The executable's `.text` section is encrypted by Steam DRM, so **it cannot be patched
as a file.** The patch is applied to the decrypted code in memory after launch.

行っているのは次の 3 つだけです。 / Only three changes are made:

1. メニューの FPS 項目ハンドラが選択インデックス 0 と 1 しか受け付けない
   ハードコードを解除する（8 バイト）
   Remove the hardcoded check that accepts only selection index 0 and 1 (8 bytes).
2. 選択肢の定義テーブル（`OPTION_MENU_SELECT_ECB`）を 2 択から 3 択に拡張する
   Extend the choice table (`OPTION_MENU_SELECT_ECB`) from two entries to three.
3. 未使用のまま残っていた文字列枠を「120」に書き換え、3 つ目のラベルとして使う
   Rewrite an unused placeholder string to `120` and use it as the third label.

パッチ位置はアドレス直指定ではなく AOB スキャンで探します。錨にしているのは
`mov edx, 0x3B726180`（`OPTION_MENU_ITEM_ECB` の FPS 項目 ID）で、これは非常に
特徴的なため誤爆しません。複数一致した場合は安全側に倒して中止します。

Patch locations are found by AOB scan rather than hardcoded addresses. The anchor is
`mov edx, 0x3B726180` (the FPS item ID from `OPTION_MENU_ITEM_ECB`), which is highly
distinctive. If the signature matches more than once, the mod aborts instead of guessing.

走査は回数と実時間の両方で必ず打ち切られ、完了後はゲームに一切触れません。
Scanning is bounded by both attempt count and wall-clock time, and the mod stops
touching the process once done.

## 免責事項 / Disclaimer

**この Mod は無保証で提供されます。使用によって生じたいかなる損害についても、
作者は一切の責任を負いません。** セーブデータの破損・消失、ゲームの動作不良、
その他の不具合を含みます。自己責任でご使用ください。

**This mod is provided as-is, without any warranty. The author accepts no liability
for any damage arising from its use,** including but not limited to corruption or loss
of save data, game malfunction, or any other problem. Use it at your own risk.

**導入前に、必ずセーブデータとゲームフォルダのバックアップを取ってください。**
セーブデータの場所は次のとおりです。

**Before installing, always back up your save data and the game folder.**
Save data is located at:

```
%LOCALAPPDATA%\KoeiTecmo\FatalFrameII\Savedata\
```

## 注意 / Notes

- 120FPS を出すには相応の PC 性能と 120Hz 以上のディスプレイが必要です
  Requires sufficient PC performance and a 120Hz+ display.
- 垂直同期を切らないとリフレッシュレートで頭打ちになります
  Turn V-Sync off, otherwise the frame rate is capped at the refresh rate.
- ゲームのアップデートでシグネチャが変わると動作しなくなる場合があります
  A game update may change the signature and break this mod.
- 他プロセスのメモリを書き換えるため、ウイルス対策ソフトが誤検知することがあります
  Antivirus software may flag it, since it writes to another process's memory.

## 不具合の報告 / Reporting issues

不具合を見つけた場合は、GitHub の Issue でご報告ください。その際、**必ず
`Mods\native120fps\native120fps.log` を添付してください。** ログが無いと原因を
特定できず、対応できない場合があります。

If you run into a problem, please open a GitHub Issue. **Be sure to attach
`Mods\native120fps\native120fps.log`.** Without the log the cause usually cannot be
identified, and the issue may not be actionable.

併せて、次の情報をいただけると助かります。
The following details also help:

- ゲームのバージョン / Game version
- GPU とディスプレイのリフレッシュレート / GPU and display refresh rate
- 発生した状況（どの画面で、何をしたとき） / What you were doing when it happened

---

## クレジット / Credits

**Created by MixedNuts**

## ライセンス / License

MIT License — 詳細は [LICENSE](LICENSE) を参照してください。
See [LICENSE](LICENSE) for details.

再配布・改変は自由ですが、**著作権表示とライセンス文を必ず残してください。**
MIT ライセンスの条件です。

You are free to redistribute and modify this, but **the copyright notice and the
license text must be retained** — that is a condition of the MIT License.

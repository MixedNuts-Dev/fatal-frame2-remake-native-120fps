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
- **ゲームのファイルを一切変更しません。** 書き換えるのは実行中のメモリと、
  Mod 自身のフォルダ内に生成した作業用ファイルだけです。導入した 2 つを消せば
  完全に元へ戻ります。
  **No game files are modified at all.** The mod only writes to process memory at
  runtime and to a working file it generates inside its own folder; deleting the
  two added items restores the original state completely.
- 直接書き換える方法では、ゲーム内でグラフィック設定を変更するたびに値が
  上書きされてしまいますが、この Mod ではその問題が起きません。
  When the file is edited by hand, the value is overwritten whenever you change any
  graphics setting in-game. That does not happen here.

## 導入 / Installation

**ビルドは不要です。** [Releases](../../releases) から配布物をダウンロードし、
中身の `dinput8.dll` と `Mods` フォルダを、ゲームのルート（`FatalFrameII.exe` と
同じ場所）にそのままコピーするだけです。

**No build required.** Download the archive from [Releases](../../releases) and copy
`dinput8.dll` and the `Mods` folder into the game's root directory (the folder
containing `FatalFrameII.exe`).

```
FatalFrameII/
  FatalFrameII.exe
  dinput8.dll                          <- added
  Mods/native120fps/native120fps.dll   <- added
  Mods/native120fps/native120fps.ini
  Mods/native120fps/README.md
  Mods/native120fps/native120fps.log   <- 起動時に生成 / generated at launch
  Mods/native120fps/archive_01.lnk     <- 初回起動時に生成 / generated on first launch
  Mods/native120fps/archive_01.lnk.tag
  Mods/native120fps/archive_06.lnk     <- 初回起動時に生成 / generated on first launch
  Mods/native120fps/archive_06.lnk.tag
```

2 つの `.lnk` は初回起動時に Mod が自動生成します（合計 約 17MB）。ゲーム側の
同名ファイルには一切手を加えません。
The two `.lnk` files (about 17 MB in total) are generated automatically on first
launch. The game's own copies are never touched.

ゲームを起動し、オプション → グラフィック設定を開くと、最大 FPS が 3 択になります。
3 つ目を選び、タイトル画面まで戻ると反映されます。

Launch the game and open Options → Graphics Settings; the max FPS option now has three
entries. Select the third one and return to the title screen to apply it.

削除は 2 つを消すだけです。 / To uninstall, just delete them.

## 対応言語 / Language support

**公式にサポートするのは日本語と英語です。** この 2 つは動作を確認しています。
**Japanese and English are officially supported**, and both are verified.

その他の言語でも 3 つ目の選択肢は選べますが、ラベルの表示までは保証しません。
イタリア語では、3 つ目のラベルに無関係な文字列が表示されます（機能そのものは
正常に動作します）。
The third entry is selectable in the other languages as well, but its label is not
guaranteed. In Italian the third entry shows an unrelated string — the feature
itself still works correctly.

## 既知の問題 / Known issues

**120 を選んだあとに画面設定を開き直すと、カーソルが「30」に戻って見えます。**
表示だけの問題で、実際のフレームレートは 120 のままです。その状態で他の設定を
変更しても 120 に戻ることはありません（実測で確認済み）。ゲーム本来の処理が、
カーソル位置を復元するときに 3 つ目の選択肢を想定していないためです。

**After selecting 120, reopening the screen settings shows the cursor back on "30".**
This is a display issue only — the actual frame rate stays at 120, and changing other
settings from that state does not reset it (verified by measurement). The game's own
code does not expect a third entry when it restores the cursor position.

## ビルド / Build

Visual Studio 2022 の C++ ツールセットが必要です。
Requires the Visual Studio 2022 C++ toolset.

共通コード（[mod-loader](https://github.com/MixedNuts-Dev/fatal-frame2-remake-mod-loader)）を
submodule で取り込んでいるので、`--recursive` 付きで clone してください。
The shared code ([mod-loader](https://github.com/MixedNuts-Dev/fatal-frame2-remake-mod-loader))
is a git submodule, so clone with `--recursive`.

```
git clone --recursive https://github.com/MixedNuts-Dev/fatal-frame2-remake-native-120fps.git
build.bat
```

clone 済みなら / If already cloned: `git submodule update --init`

`dist\` に配布用の一式が出力されます。 / The distributable set is written to `dist\`.

## 仕組み / How it works

行っているのは次の 3 つだけです。 / Only three changes are made:

1. メニューの FPS 項目ハンドラが選択インデックス 0 と 1 しか受け付けない
   ハードコードを解除する（8 バイト。メモリ上）
   Remove the hardcoded check that accepts only selection index 0 and 1
   (8 bytes, in memory).
2. 選択肢の定義テーブル（`OPTION_MENU_SELECT_ECB`）を 2 択から 3 択に拡張する
   Extend the choice table (`OPTION_MENU_SELECT_ECB`) from two entries to three.
3. 3 つ目のラベルを「120」と表示させる
   Make the third entry display `120`.

### コードパッチ / The code patch

実行ファイルは Steam DRM により `.text` が暗号化されているため、**ファイルへの静的
パッチはできません。** 起動後に復号されたメモリへ実行時にパッチを当てています。
書き込むのは **8 バイトだけ**です。

The executable's `.text` section is encrypted by Steam DRM, so **it cannot be patched
as a file.** The patch is applied to the decrypted code in memory after launch.
**Only 8 bytes** are written.

### データ側はファイルの差し替え / The data side is a file redirect

上記 2 と 3 はアーカイブ内のデータなので、メモリ上への書き込みでは足りません。
ゲームがブロックを同じアドレスへ読み直すため、書き込んでも元へ戻ってしまいます。

Changes 2 and 3 are archive data, and writing to memory is not enough: the game
reloads those blocks into the same address, so any write is undone.

そこで、**ユーザー自身のゲームフォルダから読み取った内容を書き換えた複製を
`Mods\native120fps\` 内に生成**し、`CreateFileW` を横取りしてそちらを読ませています。
ゲーム側のファイルは読むだけで、書き換えません。

Instead the mod **reads the files from the user's own game folder, writes modified
copies into `Mods\native120fps\`**, and hooks `CreateFileW` so the game opens those
copies. The game's own files are only read, never written.

| ファイル / File | 書き換える内容 / What is changed |
|---|---|
| `archive\archive_01.lnk` | 選択肢数を 2 → 3、3 つ目に未使用の文字列 ID を入れる / choice count 2 to 3, plus an unused string ID for the third entry |
| `archive\archive_06.lnk` | 未使用の文字列枠を「120」にする / an unused string slot becomes `120` |

配布物に改変済みのゲームデータは含まれません。複製はユーザーの環境で生成されます。
No modified game data is redistributed; the copies are generated on the user's machine.

書き換え先は、決め打ちのアドレスではなく**内容で探し、裏取りしてから書き込みます。**
選択肢テーブルは ID の並びを探して ECB ブロック先頭の `ecb\0` で確認し、**候補が
ちょうど 1 件でなければ中止**します。文字列枠は、空であるか既知のプレースホルダで
あることを確認してから書き込み、他の用途で使われている言語（イタリア語）では
見送ります。

Targets are located **by content rather than hardcoded offsets, and verified before
anything is written.** The choice table is found by its ID sequence and confirmed
against the `ecb\0` block header; **if there is not exactly one candidate, the mod
aborts.** A string slot is only written when it is empty or holds the known
placeholder, and languages where it is already in use (Italian) are skipped.

> 1.0.2 までは選択肢テーブルをメモリ上で探していました。ゲームが確保する 4〜8GB の
> 領域を走査する必要があり、ロード中に踏むと 1 パスに 13 秒かかるうえ、走査が
> ゲームの作業セットを強制的に常駐させていました。1.1.0 でファイル差し替えに
> 変更し、走査そのものが不要になりました。
>
> Up to 1.0.2 the choice table was located by scanning memory. That meant sweeping the
> 4-8 GB the game allocates, a single pass could take 13 seconds if it landed while the
> game was loading, and the scan forced the game's working set resident. 1.1.0 moved it
> to the file redirect, removing the scan entirely.

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
- 初回起動時に約 17MB の作業用ファイルを Mod 自身のフォルダ内に生成します
  About 17 MB of working data is generated inside the mod's own folder on first launch.

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

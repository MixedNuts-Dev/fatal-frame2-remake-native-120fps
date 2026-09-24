================================================================
 FATAL FRAME II: Crimson Butterfly REMAKE
 Native 120FPS Option
================================================================


----------------------------------------------------------------
 日本語
----------------------------------------------------------------

■ これは何か

ゲーム内のグラフィック設定で、最大フレームレートに「120」を
選べるようにします。標準では 30 と 60 の 2 つしか選べません。

ゲーム本体には 120FPS で動作する機能が最初から備わっています。
メニュー側の処理が選択肢を 2 つに固定しているだけなので、
そこを解除しています。


■ 導入方法

1. ゲームを終了します
2. 同梱の dinput8.dll と Mods フォルダを、
   ゲームのルートディレクトリ（FatalFrameII.exe と同じ場所）に
   そのままコピーします

   例:
     ...\steamapps\common\FatalFrameII\FatalFrameII.exe
     ...\steamapps\common\FatalFrameII\dinput8.dll        ← 追加
     ...\steamapps\common\FatalFrameII\Mods\native120fps\  ← 追加

3. ゲームを起動し、オプション → グラフィック設定を開きます
4. 最大フレームレートの項目が 3 択になっています。
   3 つ目を選び、タイトル画面まで戻ると 120FPS で動作します


■ 削除方法

dinput8.dll と Mods フォルダを削除するだけです。
ゲームのファイルは一切変更していないため、完全に元に戻ります。

一時的に無効化したい場合は、native120fps.ini の
Enabled を 0 にしてください。


■ 注意事項

・120FPS を出すには相応の PC 性能と、120Hz 以上に対応した
  ディスプレイが必要です。

・垂直同期を切らないとモニタのリフレッシュレートで頭打ちになります。

・ゲームのアップデート後に動かなくなることがあります。
  その場合は Mod の更新をお待ちください。

・ウイルス対策ソフトが誤検知することがあります。
  他プロセスのメモリを書き換える仕組みのためで、
  この Mod はネットワーク通信もファイル改変も行いません。


■ 仕組み

ゲームのファイルは変更しません。すべて実行時のメモリ操作です。

本体の実行ファイルは Steam の DRM で保護されており、ディスク上では
コード部分が暗号化されています。そのためファイルを直接書き換える
ことができず、起動後に復号されたメモリへパッチを当てています。

行っていることは次の 3 つだけです。

  1. メニューの最大フレームレート項目が、選択インデックス 0 と 1
     しか受け付けないハードコードを解除する（8 バイト）
  2. 選択肢の定義テーブルを 2 択から 3 択に拡張する
  3. 未使用のまま残っていた文字列枠を「120」に書き換え、
     3 つ目のラベルとして使う

問題が起きた場合は native120fps.log を確認してください。


================================================================


----------------------------------------------------------------
 English
----------------------------------------------------------------

■ What this does

Adds "120" as a selectable option for the maximum frame rate in the
in-game graphics settings. By default only 30 and 60 are available.

The game already supports running at 120 FPS internally. The menu
handler simply hardcodes the choice to two entries, and this mod
removes that restriction.


■ Installation

1. Close the game.
2. Copy dinput8.dll and the Mods folder into the game's root
   directory (the folder containing FatalFrameII.exe).

   Example:
     ...\steamapps\common\FatalFrameII\FatalFrameII.exe
     ...\steamapps\common\FatalFrameII\dinput8.dll           <- added
     ...\steamapps\common\FatalFrameII\Mods\native120fps\ <- added

3. Launch the game and open Options -> Graphics Settings.
4. The maximum frame rate option now has three entries. Select the
   third one and return to the title screen to apply it.


■ Uninstallation

Simply delete dinput8.dll and the Mods folder. No game files are
modified, so removal restores the original state completely.

To disable temporarily, set Enabled to 0 in native120fps.ini.


■ Notes

- Running at 120 FPS requires sufficient PC performance and a display
  capable of 120Hz or higher.

- Turn V-Sync off, otherwise the frame rate is capped at your
  monitor's refresh rate.

- A game update may break this mod. Please wait for an updated
  release if that happens.

- Antivirus software may flag this mod. It writes to the memory of
  another process, which is a common false-positive trigger. This mod
  performs no network activity and modifies no files.


■ How it works

No game files are modified. Everything is done in memory at runtime.

The game executable is protected by Steam DRM and its code section is
encrypted on disk, so it cannot be patched as a file. The patch is
applied to the decrypted code in memory after the game starts.

Only three changes are made:

  1. Remove the hardcoded check in the frame rate menu handler that
     accepts only selection index 0 and 1 (8 bytes).
  2. Extend the choice definition table from two entries to three.
  3. Rewrite an unused placeholder string to "120" and use it as the
     label for the third choice.

If something goes wrong, check native120fps.log.


================================================================

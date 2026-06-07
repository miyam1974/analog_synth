# NEXUS OSC

**Languages:** [日本語](README.md) | [English](README.en.md)

[License: MIT](https://opensource.org/licenses/MIT)

Windows 向けのアナログ系シンセサイザー（Standalone）。USB MIDI キーボードで演奏でき、内蔵の仮想キーボードでも試せます。

## スクリーンショット

### 実行画面

NEXUS OSC — 実行画面（1080×680）

画面サイズ: 1080×680

上部: プリセット・MONO・ALL OFF・SAVE / SAVE AS・LOAD・RESET・DIFF・MASTER

中央: シンセモジュール（コンパクトヘッダー + 5 パネル）

下部: 仮想キーボード（マウス演奏）＋ **ト音記号の五線譜**（発音中の音程表示）＋ **移調**（♭/♮/♯ + 音名）＋右端に PC キーボード図（ASDF 演奏）と **ON / OFF**

### 演奏時

NEXUS OSC — 演奏中

仮想キーボードで押下中のキーが点灯

EG のグラフ上の再生位置に点を表示

FILTER / AMPLIFIER の EG グラフ上の再生位置に点を表示

---

## 機能

| カテゴリ  | 内容                                                                         |
| ----- | -------------------------------------------------------------------------- |
| オシレータ | OSC1 / OSC2（4 波形。OSC2 は同波形再クリックで OFF）、Sub（-1 / -2 oct）、TUNE / FINE / DET2  |
| ミキサー  | OSC1 / OSC2 / SUB / NOISE レベル、Glide、V-A / V-F（OSC2 OFF 時は OSC2 / DET2 非活性） |
| フィルタ  | ローパス（CUT / RES / ENV / KEY）、Filter EG グラフ                                  |
| アンプ   | Amp ADSR グラフ                                                               |
| LFO   | LFO1 / LFO2（RATE / DEPTH、Pitch / Filter / Amp、RATE 連動 LED）                 |
| 演奏    | 16 ボイス、MONO、ALL OFF（全音停止）。仮想キーボード（マウス）・**PC キーボード（ASDF 等）**・外部 MIDI        |
| 移調    | **移調** パネル。上段 **♭ / ♮ / ♯**（既定 ♮）、下段音名コンボ（F〜G、既定 **C**）。変更時は全音停止           |
| 五線譜   | **ト音記号の五線譜**。発音中の音名をリアルタイム表示。**♯ / ♭** 表記切替（既定 ♭）                          |
| プリセット | 内蔵 4 種類 + ユーザープリセット。SAVE（上書き）/ SAVE AS / LOAD / RESET                      |
| 比較    | **DIFF** — 起動時または最後の RESET / LOAD 時の音色と A/B 比較（Space キーでも切替）               |
| セッション | 終了時に音色・プリセット・MIDI・ウィンドウ位置を `%APPDATA%` に保存                                 |
| ヘルプ   | ホバー時に日本語でヘルプ表示（SYSTEM フッター、14pt）                                           |
| アイコン  | Windows exe: 黄緑背景に黒字 **Nex**（`Resources/Icons/`）                           |

**未実装（予定）**: VST3 / CLAP、FX（コーラス / ディレイ / リバーブ）、ピッチベンド・Mod ホイール、ASIO 有効化、
SmoothedValue・実効 Cutoff Hz 表示、アルペジエータ・MPE・ポリフォニー表示、オーディオデバイス設定 UI。
詳細は [ARCHITECTURE.md](ARCHITECTURE.md) の「現状の制約と今後の拡張」を参照。

---

## ダウンロード

Visual Studio などのビルド環境がなくても、[GitHub Releases](https://github.com/miyam1974/analog_synth/releases) から Windows 用の実行ファイルを取得できます。

1. [Releases](https://github.com/miyam1974/analog_synth/releases) を開き、最新のバージョン（例: `v0.1.0`）を選ぶ
2. `NEXUS-OSC-Windows-x64-v*.zip` をダウンロードする
3. ZIP を任意のフォルダに展開する
4. 展開したフォルダ内の `AnalogSynth.exe` を起動する

ZIP には `AnalogSynth.exe` と `LICENSE` が含まれます。前回終了時の設定は `%APPDATA%/NEXUS OSC/session.json` に自動保存されます。

---

## 必要条件

### 実行時（ダウンロード版）

- **OS**: Windows 10 / 11（64-bit）
- **MIDI 入力**（任意）
- **オーディオ出力**（WASAPI がデフォルト）

### ソースからビルドする場合

- 上記に加え、[Visual Studio 2019 以降](https://visualstudio.microsoft.com/)（ワークロード「C++ によるデスクトップ開発」）
- **CMake**: 3.22 以上
- **Git**: JUCE 8.0.6 を FetchContent で取得

---

## ビルド

リポジトリをクローンしたディレクトリで:

```powershell
cmake -S . -B build -G "Visual Studio 16 2019" -A x64
cmake --build build --config Release
```

Visual Studio 2022 を使う場合は `-G "Visual Studio 17 2022"` に読み替えてください。

### 出力

```text
build/AnalogSynth_artefacts/Release/AnalogSynth.exe
```

再ビルド時は実行中のアプリを終了してください（リンクエラー防止）。

MSVC では `/utf-8` を有効にしており、日本語ヘルプ（`HelpStrings.h`）を UTF-8 で保持しています。

配布用 EXE の公開手順は [配布用 EXE の更新（開発者向け）](#配布用-exe-の更新開発者向け) を参照してください。

---

## 配布用 EXE の更新（開発者向け）

[ダウンロード](#ダウンロード) セクションで配布している `AnalogSynth.exe` は、開発者 PC 上の `build/` フォルダから手動でアップロードするのではなく、
**バージョンタグを GitHub に push したタイミングで、GitHub のサーバー上で自動ビルド・公開** されます。

### どこで動くか

| 操作                               | 実行場所                                      |
| -------------------------------- | ----------------------------------------- |
| ソース編集・`git commit`・`git push`    | 開発者 PC                                    |
| Release ビルド・ZIP 作成・Releases への添付 | **GitHub Actions**（`windows-latest` ランナー） |

ローカルで `cmake --build` しても Releases の ZIP は更新されません。`main` への通常 push だけでも更新されません。

### トリガー

`[.github/workflows/release.yml](.github/workflows/release.yml)` が、`v`* 形式のタグ（例: `v0.1.0`）の push を検知したときだけ起動します。

### サーバー上で行われる処理

1. リポジトリのソースをチェックアウト
2. CMake で Visual Studio 2026 / x64 の Release ビルドを実行（`windows-latest` 上）
3. `AnalogSynth.exe` と `LICENSE` を `NEXUS-OSC-Windows-x64-<タグ名>.zip` にまとめる
4. [GitHub Releases](https://github.com/miyam1974/analog_synth/releases) に ZIP を添付（Release ページを自動作成）

### 開発者の手順（新しい EXE を公開するとき）

1. 必要なら `CMakeLists.txt` の `project(AnalogSynth VERSION …)` を更新する（タグと揃える）
2. 変更を `main` に commit / push する
3. タグを付けて push する（例: `git tag v0.1.0` → `git push origin v0.1.0`）

### 公開後の確認

- GitHub の **Actions** タブで `Release` ワークフローが成功していること
- **Releases** タブに新しい ZIP が表示されていること
- Configure ログに `Visual Studio 18 2026` と表示されていること
（`Ninja` や `Visual Studio 17 2022` の場合は古いタグ／Re-run の可能性あり）

同じタグを付け直す場合は、先に GitHub 上の Release / タグを削除し、
**修正済み workflow が入った最新の `main` 先端** でタグを付け直してください。
失敗した Run の **Re-run** だけでは workflow 定義は更新されません。

### 利用者側の見え方

利用者は [Releases](https://github.com/miyam1974/analog_synth/releases) から最新の `NEXUS-OSC-Windows-x64-v*.zip` を取得します。
開発者が新しいタグを push して Actions が成功すると、ここに新しい EXE が追加されます。

---

## 使い方

1. USB MIDI キーボードを接続（任意）
2. [ダウンロード](#ダウンロード) した、または [ビルド](#ビルド) した `AnalogSynth.exe` を起動する
3. **SYSTEM** の **MIDI IN** で入力を選択（`All Inputs` で全デバイス）
4. 各モジュールで音色を調整し、下部の仮想キーボード・**PC キーボード（ON 時）** または外部 MIDI で演奏する（五線譜に押下中の音程が表示される）
5. **移調**: 五線譜の右にある **♭ / ♮ / ♯** と音名コンボで移調先を選ぶ（既定 **C♮**）。仮想鍵盤・PC キー・外部 MIDI すべてに適用。設定変更時は発音が止まる
6. **PC キーボード演奏**: 鍵盤右端の **ON / OFF** で有効化（起動時は **ON**）。
   **ON** または PC キー図をクリックすると演奏用フォーカスが当たり、ASDF 等で鳴る（下表）。**OFF** では PC キー割当を解除
7. コントロールにマウスを合わせると、SYSTEM 領域に日本語の説明が表示される
8. **RESET** で工場出荷状態（INIT 相当）に戻す。RESET / LOAD 後は **DIFF** の比較基準も更新される
9. 音色を編集したあと **DIFF** で基準音色と切り替えて比較できる（比較中は ALL OFF / MASTER / MIDI IN / DIFF のみ操作可）。**Space** キーでも DIFF 切替
10. ユーザープリセットは **SAVE AS** で新規保存。**SAVE** は読み込み済みユーザープリセットの上書き（編集時のみ有効）
11. アプリを終了すると、音色・プリセット選択・MIDI 選択・ウィンドウ位置が次回起動時に復元される（**移調設定は復元されない**）

### PC キーボード（ASDF）配列

起動時 **ON** のとき、PC キーで仮想鍵盤と同様に演奏できます（JUCE 標準の QWERTY マッピング）。

| 上段（黒鍵相当） | （空） | S   | D   | （空） | T   | Y   | U   | （空） | O   | P   |
| -------- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 押すキー     |     | W   | E   |     | T   | Y   | U   |     | O   | P   |
| 下段（白鍵相当） | A   | S   | D   | F   | G   | H   | J   | K   | L   | +   |
| 押すキー     | A   | S   | D   | F   | G   | H   | J   | K   | L   | ;   |

- 図上の **S / D**（上段）は物理キー **W / E** に対応（黒鍵位置の表示）
- **+** は **;**（セミコロン）キー
- 右端の図は押下中のキーをハイライト（フォーカス不要）。**発音**には **ON** かつ仮想鍵盤側へのキーボードフォーカスが必要
- エディタ操作後に鳴らなくなった場合は **ON** または PC キー図をクリック

### ユーザープリセット保存先

```text
%APPDATA%/NEXUS OSC/Presets/*.json
```

### セッション保存先（終了時の状態）

```text
%APPDATA%/NEXUS OSC/session.json
```

---

## プロジェクト構成

```text
analog_synth/
├── CMakeLists.txt
├── README.md
├── README.en.md
├── ARCHITECTURE.md
├── ARCHITECTURE.en.md
├── SPEC.md
├── LICENSE
├── Resources/
│   ├── Bravura.otf           # 五線譜（ト音記号・臨時記号）用 SMuFL フォント
│   └── Icons/
│       ├── app_icon.png          # Windows アイコン元画像（Nex）
│       ├── AppPrimaryIcon.rc     # exe アイコン（リソース ID 1）
│       └── generate_app_icon.py
├── docs/
│   └── images/
│       ├── nexus-osc-ui.png  # メイン画面（README）
│       └── playing.png       # 演奏時（README）
└── Source/
    ├── Main.cpp
    ├── AppState.*
    ├── SynthEditor.*
    ├── SynthVoice.*
    ├── SynthSound.*
    ├── SynthParameters.h
    ├── AdsrEnvelope.h
    ├── GlobalLfo.h
    ├── EnvelopePlayhead.*
    ├── PresetManager.*
    ├── Waveform.h
    ├── HelpStrings.h
    └── UI/
        ├── AdsrDisplay.*
        ├── LfoRateLed.*
        ├── ModulePanel.*
        ├── WaveformButton.*
        ├── SubOctGroupFrame.h
        ├── FuturisticLookAndFeel.*
        ├── PcKeyboardDisplay.*
        ├── TrebleStaffDisplay.*
        ├── TransposeControl.*
        └── SynthTheme.h
```

---

## ドキュメント

| ファイル                                                                          | 内容                                  |
| ----------------------------------------------------------------------------- | ----------------------------------- |
| [README.md](README.md) / [README.en.md](README.en.md)                         | 概要・ビルド・使い方                          |
| [ARCHITECTURE.md](ARCHITECTURE.md) / [ARCHITECTURE.en.md](ARCHITECTURE.en.md) | 詳細設計（レイヤ、信号フロー、WASAPI/ASIO、スレッド、UI） |
| [SPEC.md](SPEC.md)                                                            | UI 機能仕様（画面上のコントロール網羅）               |

開発・改修時は **[ARCHITECTURE.md](ARCHITECTURE.md)** を参照してください。

---

## 依存関係

- [JUCE](https://github.com/juce-framework/JUCE) **8.0.6**（CMake FetchContent）
- リンク: `juce_audio_utils`, `juce_dsp`

JUCE の利用・配布には [JUCE ライセンス](https://github.com/juce-framework/JUCE/blob/master/LICENSE.md) に従ってください（本リポジトリの MIT ライセンスとは別です）。

---

## ライセンス

本リポジトリのソースコードは **[MIT License](LICENSE)** です。

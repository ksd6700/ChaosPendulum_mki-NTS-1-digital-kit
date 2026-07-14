# ChaosPendulum_mki-NTS-1-digital-kit

**Pendy** is a double-pendulum custom oscillator for the KORG logue SDK family. It was written first for the Nu:Tekt NTS-1 digital kit mkI, then built for the other binary-compatible logue SDK v1.1 oscillator targets: minilogue xd and prologue.

日本語版はこのREADMEの後半にあります。

## Download

Choose the file that matches your hardware:

| Hardware | Download this file | Unit format |
| --- | --- | --- |
| KORG Nu:Tekt NTS-1 digital kit mkI | [pendy.ntkdigunit](https://github.com/ksd6700/ChaosPendulum_mki-NTS-1-digital-kit/releases/download/v0.1.0/pendy.ntkdigunit) | `.ntkdigunit` |
| KORG minilogue xd / xd module | [pendy.mnlgxdunit](https://github.com/ksd6700/ChaosPendulum_mki-NTS-1-digital-kit/releases/download/v0.1.0/pendy.mnlgxdunit) | `.mnlgxdunit` |
| KORG prologue | [pendy.prlgunit](https://github.com/ksd6700/ChaosPendulum_mki-NTS-1-digital-kit/releases/download/v0.1.0/pendy.prlgunit) | `.prlgunit` |

The same files are committed in [dist/](dist/) and listed in [Releases](../../releases).

Developers should start with:

| File | Purpose |
| --- | --- |
| [pendy_osc.cpp](pendy_osc.cpp) | The oscillator DSP and double-pendulum model |
| [manifest.json](manifest.json) | NTS-1 mkI manifest |
| [manifests/minilogue-xd/manifest.json](manifests/minilogue-xd/manifest.json) | minilogue xd manifest |
| [manifests/prologue/manifest.json](manifests/prologue/manifest.json) | prologue manifest |
| [project.mk](project.mk) | logue SDK project source list |

## What It Is

Pendy turns a physical simulation into an oscillator waveform. Imagine plotting a double pendulum over time:

```text
x-axis = time
y-axis = vertical position of the second mass
```

That y curve becomes the raw oscillator. Instead of reading a wavetable or running a normal phase accumulator, the oscillator advances a small chaotic mechanical system every audio block and listens to the motion of the second mass.

The core output is:

```text
y = l1 * cos(theta1) + l2 * cos(theta2)
```

That value is normalized, DC-blocked, mixed with a little short-term velocity detail, softly clipped, and written as Q31 audio for the logue SDK runtime.

## Why A Double Pendulum?

A double pendulum is deterministic but sensitive. There is no random generator in the sound source, but tiny changes in pitch, initial angle, excitation, damping, gravity, and link length push the orbit into different regions. That makes it useful as an oscillator because it sits between several familiar synthesis ideas:

- Like physical modeling, it has a state that keeps moving.
- Like FM, pitch and timbre are intertwined rather than completely separate.
- Like wavetable synthesis, it produces a repeatable waveform-like signal, but the shape evolves because the underlying system moves.
- Like chaos synthesis, it can hover near a pattern, then suddenly fold into a rougher motion.

The NTS-1 is a good home for this because its MULTI ENGINE user oscillator API lets a tiny model replace the stock oscillator while still using the NTS-1 filter, envelope, modulation, delay, reverb, and performance controls.

## Sound Engine

The state variables are:

```text
theta1, theta2 = link angles
omega1, omega2 = angular velocities
```

For each sample, Pendy:

1. Reads pitch and user parameters.
2. Converts pitch into a physics time step.
3. Adds a small sinusoidal excitation so the motion can keep speaking.
4. Advances the double-pendulum equations with a lightweight RK2 midpoint integrator.
5. Measures the vertical position of the second mass.
6. Removes DC drift.
7. Blends position with a small velocity term for bite.
8. Applies soft clipping and outputs Q31 audio.

`Damp` defaults to zero. With no damping, the pendulum keeps moving instead of fading into silence. Raising `Damp` makes the model more controlled and less endless.

## Controls

| Control | Range | What it does |
| --- | --- | --- |
| `Speed` | 0-100% | Scales the physics time step. Higher values are brighter and more unstable. |
| `Chaos` | 0-100% | Increases coupling and excitation sensitivity. |
| `Len2` | 0-100% | Changes the second link length. This strongly changes the waveform contour. |
| `Gravity` | 0-100% | Pulls the simulated masses harder, shifting motion speed and curvature. |
| `Drive` | 0-100% | Adds excitation and soft saturation. |
| `Damp` | 0-100% | Adds velocity damping. Keep low for endless motion. |
| Shape | 10-bit | Extra speed and motion pressure from the instrument's Shape control. |
| Shift+Shape | 10-bit | Extra excitation amount. |

## Install

1. Download the unit file for your instrument from the table above.
2. Install the matching KORG Sound Librarian and, if needed, the KORG USB-MIDI driver.
3. Connect your instrument over USB.
4. Open the Sound Librarian custom oscillator list.
5. Drag the unit file into the list.
6. Press **SEND ALL** to upload it to the instrument.

Official KORG pages:

- [NTS-1 digital kit Librarian and contents](https://www.korg.com/us/products/dj/nts_1/librarian_contents.php)
- [minilogue xd Librarian and Contents](https://www.korg.com/us/products/synthesizers/minilogue_xd/librarian_contents.php)
- [prologue Librarian and Contents](https://www.korg.com/us/products/synthesizers/prologue/librarian_contents.php)
- [KORG logue SDK](https://github.com/korginc/logue-sdk)

Back up your user units before uploading third-party content.

## Build

This project targets the logue SDK v1.1 oscillator ABI used by NTS-1 mkI, minilogue xd, and prologue.

```sh
git clone https://github.com/korginc/logue-sdk.git
cd logue-sdk
git submodule update --init
```

For NTS-1 mkI:

```sh
cp -R /path/to/ChaosPendulum_mki-NTS-1-digital-kit platform/nutekt-digital/pendy
cd platform/nutekt-digital/pendy
make install
```

For minilogue xd or prologue, copy the project into that platform's `dummy-osc` style location and replace `manifest.json` with the matching file from `manifests/`.

Build notes and checksums are in [docs/builds.md](docs/builds.md).

## Compatibility

The included binaries are for the logue SDK v1.1 Cortex-M4 generation:

- NTS-1 digital kit mkI
- minilogue xd
- prologue

The current KORG logue SDK also contains newer v2 platforms such as NTS-1 digital kit mkII and microKORG2. Those use a different dynamic unit ABI, so they are not the same build target as the mkI/minilogue/prologue files in this repository.

## Status

- NTS-1 mkI package builds successfully.
- minilogue xd package builds successfully.
- prologue package builds successfully.
- Host smoke test produces non-zero oscillator output.
- Hardware upload testing is still welcome from users with each instrument.

## 日本語

**Pendy** は、KORG logue SDK向けの二重振り子カスタム・オシレーターです。最初にNu:Tekt NTS-1 digital kit mkI向けに作り、同じlogue SDK v1.1系で互換性のあるminilogue xd / prologue向けにもビルドしています。

## ダウンロード

使っている機材に合うファイルを選んでください。

| 機材 | ダウンロードするファイル | 形式 |
| --- | --- | --- |
| KORG Nu:Tekt NTS-1 digital kit mkI | [pendy.ntkdigunit](https://github.com/ksd6700/ChaosPendulum_mki-NTS-1-digital-kit/releases/download/v0.1.0/pendy.ntkdigunit) | `.ntkdigunit` |
| KORG minilogue xd / xd module | [pendy.mnlgxdunit](https://github.com/ksd6700/ChaosPendulum_mki-NTS-1-digital-kit/releases/download/v0.1.0/pendy.mnlgxdunit) | `.mnlgxdunit` |
| KORG prologue | [pendy.prlgunit](https://github.com/ksd6700/ChaosPendulum_mki-NTS-1-digital-kit/releases/download/v0.1.0/pendy.prlgunit) | `.prlgunit` |

同じファイルは [dist/](dist/) にもコミットしてあり、[Releases](../../releases) からも確認できます。

開発者向けの主なファイル:

| ファイル | 内容 |
| --- | --- |
| [pendy_osc.cpp](pendy_osc.cpp) | 発音処理と二重振り子モデル |
| [manifest.json](manifest.json) | NTS-1 mkI用manifest |
| [manifests/minilogue-xd/manifest.json](manifests/minilogue-xd/manifest.json) | minilogue xd用manifest |
| [manifests/prologue/manifest.json](manifests/prologue/manifest.json) | prologue用manifest |
| [project.mk](project.mk) | logue SDK用のソース指定 |

## これは何か

Pendyは、物理シミュレーションをそのまま波形にするオシレーターです。二重振り子の動きを次のようなグラフとして見ます。

```text
x軸 = 時間
y軸 = 2つ目の錘の縦位置
```

このy方向のカーブをオーディオ波形として使います。普通のオシレーターのように固定波形を読み出すのではなく、毎サンプルごとに小さな機械系を進め、その運動を音として聴く発想です。

中心になる値は次の式です。

```text
y = l1 * cos(theta1) + l2 * cos(theta2)
```

この値を正規化し、DC成分を取り除き、少しだけ速度成分を混ぜ、ソフトクリップして、logue SDKのQ31形式で出力します。

## なぜ二重振り子なのか

二重振り子はランダムではありません。決まった式で動いています。それでも、初期角度、ピッチ、重力、励起、減衰、リンク長の少しの変化で、動き方が大きく変わります。この性質がオシレーターとして面白いところです。

- 物理モデリングのように、内部状態が動き続けます。
- FMのように、ピッチと音色が強く結びつきます。
- ウェーブテーブルのように波形として聴けますが、形は少しずつ変化します。
- カオス合成のように、規則的な動きから急に荒れた動きへ移ります。

NTS-1のMULTI ENGINEは、こういう小さな物理モデルをカスタム・オシレーターとして差し替えられるので、相性が良いです。NTS-1側のフィルター、エンベロープ、モジュレーション、ディレイ、リバーブをそのまま使いながら、音の出発点だけを二重振り子にできます。

## 発音方式

内部状態は次の4つです。

```text
theta1, theta2 = 2本のリンクの角度
omega1, omega2 = 角速度
```

各サンプルで次の処理をします。

1. ピッチとパラメータを読む。
2. ピッチから物理シミュレーションの時間刻みを作る。
3. 小さな周期的励起を加えて、運動が止まりきらないようにする。
4. RK2 midpoint法で二重振り子を進める。
5. 2つ目の錘の縦位置を取り出す。
6. DC成分を取り除く。
7. 位置に少し速度成分を混ぜて、音のエッジを出す。
8. ソフトクリップしてQ31音声として出力する。

`Damp` の初期値は0です。減衰が無いと、振り子は止まらず動き続けます。`Damp` を上げると制御しやすくなり、荒れ方も少し落ち着きます。

## パラメータ

| パラメータ | 範囲 | 効果 |
| --- | --- | --- |
| `Speed` | 0-100% | 物理時間の進み方。上げると明るく不安定になります。 |
| `Chaos` | 0-100% | 結合と励起への反応を強めます。 |
| `Len2` | 0-100% | 2本目のリンク長。波形の輪郭が大きく変わります。 |
| `Gravity` | 0-100% | 錘を引く力。運動の速さと曲がり方が変わります。 |
| `Drive` | 0-100% | 励起とサチュレーション。 |
| `Damp` | 0-100% | 速度抵抗。永遠に振らせたい場合は低めにします。 |
| Shape | 10-bit | 速度と動きの圧を足します。 |
| Shift+Shape | 10-bit | 励起量を足します。 |

## インストール

1. 上の表から、自分の機材に合うunitファイルをダウンロードします。
2. KORG Sound Librarianと、必要ならKORG USB-MIDI Driverをインストールします。
3. 機材をUSBで接続します。
4. Sound LibrarianでCustom Oscillatorのリストを開きます。
5. unitファイルをドラッグ&ドロップします。
6. **SEND ALL** で機材へ送ります。

公式リンク:

- [NTS-1 digital kit Librarian and contents](https://www.korg.com/us/products/dj/nts_1/librarian_contents.php)
- [minilogue xd Librarian and Contents](https://www.korg.com/us/products/synthesizers/minilogue_xd/librarian_contents.php)
- [prologue Librarian and Contents](https://www.korg.com/us/products/synthesizers/prologue/librarian_contents.php)
- [KORG logue SDK](https://github.com/korginc/logue-sdk)

サードパーティ製unitを入れる前に、既存のユーザーunitはバックアップしておくのがおすすめです。

## ビルド

このプロジェクトは、NTS-1 mkI / minilogue xd / prologueで使われるlogue SDK v1.1のoscillator ABIを対象にしています。

```sh
git clone https://github.com/korginc/logue-sdk.git
cd logue-sdk
git submodule update --init
```

NTS-1 mkI向け:

```sh
cp -R /path/to/ChaosPendulum_mki-NTS-1-digital-kit platform/nutekt-digital/pendy
cd platform/nutekt-digital/pendy
make install
```

minilogue xd / prologue向けにビルドする場合は、それぞれのplatform配下にコピーして、`manifest.json` を `manifests/` の該当ファイルに差し替えてください。

ビルドログとチェックサムは [docs/builds.md](docs/builds.md) にまとめています。

## 互換性

このrepoに入っている成果物は、logue SDK v1.1 / Cortex-M4世代向けです。

- NTS-1 digital kit mkI
- minilogue xd
- prologue

現在のKORG logue SDKには、NTS-1 digital kit mkIIやmicroKORG2などのv2系platformもあります。これらは動的unit ABIが異なるため、このrepoのmkI/minilogue/prologue用バイナリとは別ターゲットとして移植する必要があります。

## 状態

- NTS-1 mkI用パッケージはビルド済みです。
- minilogue xd用パッケージはビルド済みです。
- prologue用パッケージはビルド済みです。
- ホスト上のスモークテストで非ゼロ出力を確認しています。
- 実機アップロードの追加検証は歓迎です。

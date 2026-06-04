#pragma once

#include <juce_core/juce_core.h>

namespace HelpText
{
// C++17 では u8"..." が const char* となり juce::String(const char*) は ASCII 扱いのため、
// UTF-8 ソース内の日本語は fromUTF8 経由で juce::String に渡す。
inline juce::String utf8(const char* text)
{
    return juce::String::fromUTF8(text);
}

inline juce::String defaultStatus()
{
    return utf8(u8"コントロールにマウスを合わせると説明が表示されます。");
}

inline juce::String tune()
{
    return utf8(u8"基本音程を半音単位でずらします（±12半音）。");
}

inline juce::String fine()
{
    return utf8(u8"音程をセント単位で微調整します（100セント=1半音）。");
}

inline juce::String tuneReset()
{
    return utf8(u8"TUNEを0半音にリセットします。RESETボタンで常に0に戻します。");
}

inline juce::String fineReset()
{
    return utf8(u8"FINEを0セントにリセットします。RESETボタンで常に0に戻します。");
}

inline juce::String noise()
{
    return utf8(u8"ホワイトノイズをミックスします。スネアや効果音向き。");
}

inline juce::String cutoff()
{
    return utf8(u8"ローパスフィルタのカットオフ周波数。低いほどこもった音になります。");
}

inline juce::String resonance()
{
    return utf8(u8"フィルタのレゾナンス（鋭さ）。上げるとカットオフ付近が強調されます。");
}

inline juce::String filterEnvAmount()
{
    return utf8(u8"フィルタエンベロープの効き量。鍵を押している間カットオフを動かします。");
}

inline juce::String filterKeyTrack()
{
    return utf8(u8"キートラッキング。高い音程でカットオフが上がり、低い音程で下がります。0で無効。");
}

inline juce::String osc2Detune()
{
    return utf8(u8"OSC2のデチューン（セント）。OSC1との差でユニゾンを厚くできます。"
                  u8"TUNE/FINEは両OSCに共通です。");
}

inline juce::String osc2DetuneReset()
{
    return utf8(u8"DET2を0セントにリセットします。");
}

inline juce::String filterAttack()
{
    return utf8(u8"フィルタEG：アタック。音が立ち上がるまでの時間。");
}

inline juce::String filterDecay()
{
    return utf8(u8"フィルタEG：ディケイ。ピークからサスティンへ下がる時間。");
}

inline juce::String filterSustain()
{
    return utf8(u8"フィルタEG：サスティン。鍵を押し続けている間のレベル。");
}

inline juce::String filterRelease()
{
    return utf8(u8"フィルタEG：リリース。鍵を離してから消えるまでの時間。");
}

inline juce::String ampEgGraph()
{
    return utf8(u8"アンプエンベロープの山の形。A/D/S/R ノブで曲線が変化します。");
}

inline juce::String filterEgGraph()
{
    return utf8(u8"フィルタエンベロープの山の形。A/D/S/R ノブで曲線が変化します。");
}

inline juce::String ampAttack()
{
    return utf8(u8"アンプEG：アタック。音量が立ち上がるまでの時間。");
}

inline juce::String ampDecay()
{
    return utf8(u8"アンプEG：ディケイ。最大音量からサスティンへ下がる時間。");
}

inline juce::String ampSustain()
{
    return utf8(u8"アンプEG：サスティン。鍵を押し続けている間の音量レベル。");
}

inline juce::String ampRelease()
{
    return utf8(u8"アンプEG：リリース。鍵を離してから無音になるまでの時間。");
}

inline juce::String lfoRate()
{
    return utf8(u8"LFO1の速度（Hz）。ビブラートやわくわく感の速さを決めます。");
}

inline juce::String lfoRateLed()
{
    return utf8(u8"LFO1の速度に合わせて点滅するLEDです。RATEを変えると点滅の速さが変わります。");
}

inline juce::String lfo2Rate()
{
    return utf8(u8"LFO2の速度（Hz）。LFO1と独立した周波数で変調できます。");
}

inline juce::String lfo2RateLed()
{
    return utf8(u8"LFO2の速度に合わせて点滅するLEDです。");
}

inline juce::String lfo2Depth()
{
    return utf8(u8"LFO2の深さ。LFO1と合わせて音程・フィルタ・音量を変調できます。");
}

inline juce::String lfo2Pitch()
{
    return utf8(u8"LFO2を音程（ピッチ）に送ります。");
}

inline juce::String lfo2Filter()
{
    return utf8(u8"LFO2をフィルタに送ります。");
}

inline juce::String lfo2Amp()
{
    return utf8(u8"LFO2を音量に送ります。");
}

inline juce::String lfoDepth()
{
    return utf8(u8"LFO1の深さ。変調の強さを調整します。");
}

inline juce::String lfoPitch()
{
    return utf8(u8"LFO1を音程（ピッチ）に送ります。ビブラート効果になります。");
}

inline juce::String lfoFilter()
{
    return utf8(u8"LFO1をフィルタに送ります。わくわくするフィルタ変調になります。");
}

inline juce::String lfoAmp()
{
    return utf8(u8"LFO1を音量に送ります。トレモロ（音量のゆらぎ）になります。");
}

inline juce::String panelOsc()
{
    return utf8(u8"オシレータ：OSC1/OSC2の波形とチューンを設定します。");
}

inline juce::String panelMixer()
{
    return utf8(u8"ミキサー：OSC1/OSC2/Sub/Noiseのレベルとグライド・ベロシティを調整します。");
}

inline juce::String osc1Level()
{
    return utf8(u8"OSC1の音量レベル。");
}

inline juce::String osc2Level()
{
    return utf8(u8"OSC2の音量レベル。");
}

inline juce::String subLevel()
{
    return utf8(u8"サブオシレータ（-1/-2オクターブ）の音量。");
}

inline juce::String subOctave()
{
    return utf8(u8"サブオシのオクターブ幅（-1 または -2）。");
}

inline juce::String osc2Wave()
{
    return utf8(u8"OSC2の波形を選択します。");
}

inline juce::String glide()
{
    return utf8(u8"グライド（ポルタメント）。音程が滑らかに変化します。MONO時に効果的。");
}

inline juce::String velocityAmp()
{
    return utf8(u8"ベロシティが音量に影響する強さ。");
}

inline juce::String velocityFilter()
{
    return utf8(u8"ベロシティがフィルタカットオフに影響する強さ。");
}

inline juce::String monoMode()
{
    return utf8(u8"モノフォニックモード。同時に1音のみ発音します。");
}

inline juce::String presetCombo()
{
    return utf8(u8"プリセットを選択して音色を切り替えます。");
}

inline juce::String presetSave()
{
    return utf8(u8"現在の設定をユーザープリセットとして保存します。");
}

inline juce::String presetLoad()
{
    return utf8(u8"保存済みプリセットファイルを読み込みます。");
}

inline juce::String virtualKeyboard()
{
    return utf8(u8"マウスで操作できる仮想キーボード。MIDIがなくても音を確認できます。");
}

inline juce::String panelFilter()
{
    return utf8(u8"フィルタ：音色の明るさと、フィルタエンベロープを設定します。");
}

inline juce::String panelAmp()
{
    return utf8(u8"アンプ：音量の立ち上がり・減衰（ADSR）を設定します。");
}

inline juce::String panelLfo()
{
    return utf8(u8"LFO：LFO1/LFO2の2段構成。"
                  u8"各自独立した速度と深さで音程・フィルタ・音量を周期的に変調します。");
}

inline juce::String panelSystem()
{
    return utf8(u8"システム：MIDI入力とステータス表示エリアです。");
}

inline juce::String panic()
{
    return utf8(u8"すべての発音を即座に停止します。音が残ったときに使います。");
}

inline juce::String master()
{
    return utf8(u8"マスター音量。シンセ全体の出力レベルを調整します。");
}

inline juce::String midiLabelHelp()
{
    return utf8(u8"MIDIキーボードなどの入力デバイスを選択します。");
}

inline juce::String midiCombo()
{
    return utf8(u8"接続するMIDI入力を選びます。\"All Inputs\" ですべて受信。");
}

inline juce::String waveSine()
{
    return utf8(u8"サイン波：滑らかで柔らかい基礎音色。");
}

inline juce::String waveSaw()
{
    return utf8(u8"のこぎり波：倍音が豊かな典型的なシンセ音色。");
}

inline juce::String waveSquare()
{
    return utf8(u8"矩形波：中空感のあるクラシックなデジタル音色。");
}

inline juce::String waveTriangle()
{
    return utf8(u8"三角波：サインに近いがやや明るい音色。");
}
} // namespace HelpText

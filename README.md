# PS2to98keyboard

PS/2キーボードをArduinoNanoを用いて、98用キーボードに変換するスケッチとなります。

## 必要なライブラリ
[SoftwareSerialParity](https://github.com/ljbeng/SoftwareSerialParity)

## ArduinoNanoとの接続
//98側(miniDin8)ピン定義<br>
#define RST 4 //リセット要求<br>
#define RDY 5 //送信可能<br>
#define RXD 9 //データ送信<br>
#define RTY 6 //再送要求<br>
![md8_pc98key](https://github.com/maru-dinagon/PS2to98keyboard/assets/66824091/eea9943b-d760-45ba-8d83-b2662c325f7a)

//PS2側ピン定義<br>
#define PS2_DATA    A3 // PS2キーボードのDATAピンに接続<br>
#define PS2_CLK     3  // PS2キーボードのCLKピンに接続(割込み可能PIN必須Nanoなら2or3)<br>
![md6_ps2](https://github.com/maru-dinagon/PS2to98keyboard/assets/66824091/fc120802-ff57-40b0-b6bb-cf172dff656e)

※5VとGNDの接続も忘れずに!



## 紹介と仕様解説ブログ

 

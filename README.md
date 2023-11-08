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
![md8_pc98key](/img/md8_pc98key.png)

//PS2側ピン定義<br>
#define PS2_DATA    A3 // PS2キーボードのDATAピンに接続<br>
#define PS2_CLK     3  // PS2キーボードのCLKピンに接続(割込み可能PIN必須Nanoなら2or3)<br>
![md6_ps2](/img/md6_ps2.png)

※5VとGNDの接続も忘れずに!



## 紹介と仕様解説ブログ

 

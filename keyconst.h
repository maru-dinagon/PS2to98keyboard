#define NA_CODE 0xFF //98キーボードスキャンコードの未定義初期値

#define R_CODE 0x80 // 98キーコードのリリースの値

//98スキャンコード定義(必要なものだけ)
#define KEY98_STOP  0x60 //STOPキー
#define KEY98_COPY  0x61 //COPYキー
#define KEY98_SHIFT 0x70 //シフトキー
#define KEY98_CTRL  0x74 //CTRLキー
#define KEY98_GRPH  0x73 //GRPHキー
#define KEY98_CPSLK 0x71 //CapsLock
#define KEY98_KANA  0x72 //かなキー
#define KEY98_NFER  0x35 //NFERキー


//PS2キーボードスキャンコード定義(必要なものだけ)
#define PS2_NUMLOCK         0x77 //NumLockキー
#define PS2_CAPSLOCK        0x58 //CapsLockキー
#define PS2_HANKAKUZENKAKU  0x0E //半角全角漢字キー
#define PS2_KATAHIRA        0x13 //カタカナひらがなローマ字キー

uint8_t codeArray[0xFF];
uint8_t codeArrayNotLk[0xFF];    //NumLock解除時のコード変換テーブル
uint8_t codeArrayE0[0xFF];

uint8_t pause_code[8] = {0xE1, 0x14, 0x77,0xE1,0xF0,0x14,0xF0,0x77};

void setCodeArray()
{
  for(int i = 0 ; i < 0xFF ; i++){
    codeArray[i]   = NA_CODE;
    codeArrayNotLk[i] = NA_CODE;
    codeArrayE0[i] = NA_CODE;
    
  }

  codeArray[0x16] = 0x01;    // 1
  codeArray[0x1E] = 0x02;    // 2
  codeArray[0x26] = 0x03;    // 3
  codeArray[0x25] = 0x04;    // 4
  codeArray[0x2E] = 0x05;    // 5
  codeArray[0x36] = 0x06;    // 6
  codeArray[0x3D] = 0x07;    // 7
  codeArray[0x3E] = 0x08;    // 8
  codeArray[0x46] = 0x09;    // 9
  codeArray[0x45] = 0x0A;    // 0
  codeArray[0x1C] = 0x1D;    // a
  codeArray[0x32] = 0x2D;    // b
  codeArray[0x21] = 0x2B;    // c
  codeArray[0x23] = 0x1F;    // d
  codeArray[0x24] = 0x12;    // e
  codeArray[0x2B] = 0x20;    // f
  codeArray[0x34] = 0x21;    // g
  codeArray[0x33] = 0x22;    // h
  codeArray[0x43] = 0x17;    // i
  codeArray[0x3B] = 0x23;    // j
  codeArray[0x42] = 0x24;    // k
  codeArray[0x4B] = 0x25;    // l
  codeArray[0x3A] = 0x2F;    // m
  codeArray[0x31] = 0x2E;    // n
  codeArray[0x44] = 0x18;    // o
  codeArray[0x4D] = 0x19;    // p
  codeArray[0x15] = 0x10;    // q
  codeArray[0x2D] = 0x13;    // r
  codeArray[0x1B] = 0x1E;    // s
  codeArray[0x2C] = 0x14;    // t
  codeArray[0x3C] = 0x16;    // u
  codeArray[0x2A] = 0x2C;    // v
  codeArray[0x1D] = 0x11;    // w
  codeArray[0x22] = 0x2A;    // x
  codeArray[0x35] = 0x15;    // y
  codeArray[0x1A] = 0x29;    // z
  codeArray[0x4E] = 0x0B;    // -
  codeArray[0x55] = 0x0C;    // ^
  codeArray[0x6A] = 0x0D;    // ￥
  codeArray[0x54] = 0x1A;    // @
  codeArray[0x5B] = 0x1B;    // [
  codeArray[0x4C] = 0x26;    // ;
  codeArray[0x52] = 0x27;    // :
  codeArray[0x5D] = 0x28;    // ]
  codeArray[0x41] = 0x30;    // ,
  codeArray[0x49] = 0x31;    // .
  codeArray[0x4A] = 0x32;    // /
  codeArray[0x51] = 0x33;    // \

  //ファンクションキー
  codeArray[0x05] = 0x62;    // F1 
  codeArray[0x06] = 0x63;    // F2 
  codeArray[0x04] = 0x64;    // F3 
  codeArray[0x0C] = 0x65;    // F4 
  codeArray[0x03] = 0x66;    // F5 
  codeArray[0x0B] = 0x67;    // F6 
  codeArray[0x83] = 0x68;    // F7 
  codeArray[0x0A] = 0x69;    // F8 
  codeArray[0x01] = 0x6A;    // F9 
  codeArray[0x09] = 0x6B;    // F10 
  codeArray[0x78] = 0x52;    // F11 -> Vf1
  codeArray[0x07] = 0x53;    // F12 -> Vf2
  
  //機能キー
  codeArray[0x66] = 0x0E;    // バックスペース  
  codeArray[0x5A] = 0x1C;    // エンター 
  codeArray[0x29] = 0x34;    //  スペース
  codeArray[0x76] = 0x00;    //  ESC
  //codeArray[0x0E] = 0x;    //  半角/全角 漢字(IME切り替えコード内処理)
  codeArray[0x0D] = 0x0F;    //  タブ
  //codeArray[0x58] = 0x;    //  CAPS(shift + CapsLockでコード内切り替え処理)
  codeArray[0x12] = 0x70;    // 左シフト 
  codeArray[0x14] = 0x74;    //  左Ctrl
  codeArrayE0[0x1F] = 0x77;    // 左Win
  codeArray[0x11] = 0x73;    //  左Alt
  codeArray[0x59] = 0x70;    //  右シフト
  codeArrayE0[0x14] = 0x74;    // 右Ctrl
  codeArrayE0[0x11] = 0x73;    // 右Alt ->GRPH
  codeArrayE0[0x27] = 0x78;    // 右Win
  codeArray[0x67] = 0x51;    //  無変換 -> XFER
  codeArray[0x64] = 0x35;    //  変換 -> NFER
  //codeArray[0x13] = 0x72;    //  カタひら ローマ字 -> カナ
  codeArrayE0[0x2F] = 0x79;    // アプリケーション
  codeArrayE0[0x75] = 0x3A;    // ↑
  codeArrayE0[0x72] = 0x3D;    // ↓ 
  codeArrayE0[0x6B] = 0x3B;    // ←
  codeArrayE0[0x74] = 0x3C;    // →
  codeArrayE0[0x7D] = 0x36;    // PgUp 
  codeArrayE0[0x7A] = 0x37;    // PgDn
  codeArrayE0[0x6C] = 0x3E;    // Home -> HOMECLR
  codeArrayE0[0x69] = 0x3F;    // End -> HELP
  codeArrayE0[0x71] = 0x39;    // Delete
  codeArrayE0[0x70] = 0x38;    // Insert

  //テンキー
  codeArray[0x70] = 0x4E;    // 0 
  codeArray[0x69] = 0x4A;    // 1 
  codeArray[0x72] = 0x4B;    // 2 
  codeArray[0x7A] = 0x4C;    // 3 
  codeArray[0x6B] = 0x46;    // 4 
  codeArray[0x73] = 0x47;    // 5 
  codeArray[0x74] = 0x48;    // 6 
  codeArray[0x6C] = 0x42;    // 7 
  codeArray[0x75] = 0x43;    // 8 
  codeArray[0x7D] = 0x44;    // 9 
  codeArray[0x71] = 0x50;    // . 
  codeArrayE0[0x4A] = 0x41;    // /
  codeArray[0x7C] = 0x45;    // * 
  codeArray[0x7B] = 0x40;    // - 
  codeArray[0x79] = 0x49;    // + 
  codeArrayE0[0x5A] = 0x1C;    // エンター 
  //codeArray[0x77] = 0x53;    //  NumLock

  //PrintScreen検出用(通常時は下の二つのキーがonになったら検出する)
  codeArrayE0[0x12] = 0xFE;    //ps_fir 
  codeArrayE0[0x7C] = 0xFD;    //ps_sec 
  codeArray[0x84] = 0x61;    //Altと同時押しの時にこのスキャンコードが送られてくる
    
  // NumLock解除時の変換キーコード
  memcpy(codeArrayNotLk, codeArray, sizeof(codeArray));

  codeArrayNotLk[0x70] = 0x38;    // 0 -> Insert
  codeArrayNotLk[0x69] = 0x3F;    // 1 -> End(Help)
  codeArrayNotLk[0x72] = 0x3D;    // 2 -> ↓ 
  codeArrayNotLk[0x7A] = 0x37;    // 3 -> PgDn
  codeArrayNotLk[0x6B] = 0x3B;    // 4 -> ←
  codeArrayNotLk[0x73] = 0xFF;    // 5 -> N/A
  codeArrayNotLk[0x74] = 0x3C;    // 6 -> →
  codeArrayNotLk[0x6C] = 0x3E;    // 7 -> HOMECLR
  codeArrayNotLk[0x75] = 0x3A;    // 8 -> ↑
  codeArrayNotLk[0x7D] = 0x36;    // 9 -> PgUp 
  codeArrayNotLk[0x71] = 0x39;    // . -> Delete

  codeArrayNotLk[0x05] = 0x52;    // F1 -> Vf1 
  codeArrayNotLk[0x06] = 0x53;    // F2 -> Vf2
  codeArrayNotLk[0x04] = 0x54;    // F3 -> Vf3
  codeArrayNotLk[0x0C] = 0x55;    // F4 -> Vf4
  codeArrayNotLk[0x03] = 0x56;    // F5 -> Vf5 
  
}

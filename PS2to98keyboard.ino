#include "keyconst.h"
#include <SoftwareSerialParity.h>

#define DEBUG

//98側(miniDin8)ピン定義
#define RST 4 //リセット要求
#define RDY 5 //送信可能
#define RXD 9 //データ送信
#define RTY 6 //再送要求

//PS2側ピン定義
#define PS2_DATA    A3 // PS2キーボードのDATAピンに接続
#define PS2_CLK     3  // PS2キーボードのCLKピンに接続(割込み可能PIN必須Nanoなら2or3)
#define CLK_INT     1  // 割り込み順位
//PS2 LED値
#define PS2_LED_CAPSLOCK  0x04 //CapsLock時
#define PS2_LED_NUMLOCK   0x02 //NumLock
#define PS2_LED_SCRLOCK   0x01 //スクロールロックはカナ

#include "ps2ScanCode.h"　//PS/2スキャンコード取得&LED制御コード https://okiraku-camera.tokyo/blog/?p=8246

//98キーボードに対して送るコマンド定義
#define ACK              0xFA
#define NACK             0xFC

//PS2スキャンコードの接頭定義
#define E_HEAD_NONE -1
#define E_HEAD_E0    0
#define E_HEAD_E1    1

SoftwareSerialParity mySerial(RST, RXD);    // RX,TXの割り当て

bool keybreak = false;
bool shift = false;
bool ctrl = false;
bool alt = false;

int pause_pos = -1;
int pr_sc_count = 0;
int e_head = E_HEAD_NONE;  // PS2スキャンコードの接頭
uint8_t prev  = 0x00; //直前の送信キーの保持
int kana_f, caps_f,  num_f;
uint8_t last_LED = 0x00;

//PS/2キーボードLED制御
void setLED(){
  uint8_t led = 0x00;
  if(caps_f) led = led | PS2_LED_CAPSLOCK;
  if(num_f ) led = led | PS2_LED_NUMLOCK;
  if(kana_f) led = led | PS2_LED_SCRLOCK;  //スクロールロックはカナモードOn/Offとする
  
  //変更があった時のみキーボードへ送信する
  if(last_LED != led){
    kbd_led(led);
    last_LED = led;
  }
}

//キーリピートをPC-98のデフォルト[delay=500ms Rate=24MakeCode/Sec(約40ms/Stroke)]で固定
// https://www.technoveins.co.jp/technical/keyboard/
bool setKeyRepeat(){
  uint8_t tmp[1];
  send_command(0xF3,1,tmp);
  if (tmp[0] != 0xfa) return false;
  send_command(0b00100010,1,tmp);
  if (tmp[0] != 0xfa) return false;
  send_command(0xF4,1,tmp);
  if (tmp[0] != 0xfa) return false;
  return true;
}

void setup(){
  
#ifdef DEBUG
    Serial.begin(115200) ;
    Serial.println("Keyboard Debug :") ;
#endif    

  //pinMode(RST, INPUT);
    pinMode(RDY, INPUT);
    pinMode(RTY, INPUT);
    //PC-98側ハードウェアシリアル初期化
    //19.2kbps 8ビット 奇数パリティ ストップビット1ビット
    mySerial.begin(19200,ODD);


    //NUM LOCKはデフォルトでON
    kana_f = 0;
    caps_f = 0;
    num_f = 1;
    
    //PS2キーコード -> 98キーコード変換　テーブルのセット
    setCodeArray();

    //PS/2キーボード初期化
    pinMode(PS2_DATA, INPUT);
    pinMode(PS2_CLK, INPUT);
    attachInterrupt(CLK_INT, clk_interrupt, FALLING);
    delay(50);
    if (kbd_reset()){
#ifdef DEBUG
      Serial.println("kbd_reset OK");
#endif

    //キーリピートのセット
      if(setKeyRepeat()){
#ifdef DEBUG
        Serial.println("keyRepeat set OK");
#endif
      }else{
#ifdef DEBUG
        Serial.println("keyRepeat set Error");
#endif      
      }
      delay(10);
      
      //キーボードのLEDのセット
      setLED();
          
    }else{
#ifdef DEBUG
      Serial.println("kbd_reset ERROR");
#endif
    }

}
void loop()
{
      uint8_t ps2code ;
      
      uint8_t sendCode = 0;
      uint8_t keyCode = NA_CODE;

      pc98key_command();
      setLED();
     
      ps2code = get_buffer();
      if(ps2code == 0x00) return;

#ifdef DEBUG
      Serial.print("Get Ps2Code : ");
      Serial.print(ps2code,HEX);
      Serial.println(""); 
#endif

      //ポーズキーの検出(0xE1を通過した後)
      if(pause_pos > 0){
        if(ps2code == pause_code[pause_pos]){
          if(pause_pos == 7){
            //Pause -> STOP
            pc98key_send(KEY98_STOP);
            pause_pos = -1;
          }else{
            pause_pos++;
            return;
          }
        }else{
          pause_pos = -1;
        }
      }
      
      //E0系スキャンコード
      if(ps2code == 0xE0){
          e_head = E_HEAD_E0;
          return;
      }
      //E1系スキャンコード(Pauseしかない)
      if(ps2code == 0xE1){
          pause_pos = 1; //E1の次の位置を指定する
          return;
      }
      //キーリリース
      if(ps2code == 0xF0){
          keybreak = true;
          return;
      }

      //E_HEADがないスキャンコード変換処理と特殊コマンド処理
      if(e_head == E_HEAD_NONE){
        
        //半角・全角キーでIMEのONOFF
        if(ps2code == PS2_HANKAKUZENKAKU){
          if(!keybreak){
            pc98key_send(KEY98_CTRL);
            pc98key_send(KEY98_NFER);
            pc98key_send(KEY98_NFER | R_CODE);
            pc98key_send(KEY98_CTRL | R_CODE);
          }else{
            keybreak = false;
          }
        }else 
        
        //Shift+CapsLockキーでロック・解除
        if(ps2code == PS2_CAPSLOCK && shift){
          if(caps_f){
            pc98key_send(KEY98_CPSLK | R_CODE);
            caps_f = false;
          }else{
            pc98key_send(KEY98_CPSLK);
            caps_f = true;
          }
        }else

        //カタカナ・ひらがな・ローマ字キーでのかな状態のセット・解除
        if(ps2code == PS2_KATAHIRA){
          if(!keybreak){
            if(kana_f){
              pc98key_send(KEY98_KANA | R_CODE);
              kana_f = false;
            }else{
              pc98key_send(KEY98_KANA);
              kana_f = true;
            }
          }
        }else
        
        //Numlockキーでのセット・解除(98でNumlockは不要なのでVFキー切り替えで利用)
        if(ps2code == PS2_NUMLOCK){
          if(!keybreak){
            if(num_f){
              num_f = false;
            }else{
              num_f = true;
            }
          }
        
        }else{
          if(num_f){
            keyCode = codeArray[ps2code];
          }else{
            keyCode = codeArrayNotLk[ps2code];
          }              
        }
        
      }else
      //E0系スキャンコード変換処理
      if(e_head == E_HEAD_E0){
        keyCode = codeArrayE0[ps2code];

        if(keyCode == 0xFD || keyCode == 0xFE){
          if(!keybreak){
            pr_sc_count ++;
            if(pr_sc_count >2) pr_sc_count = 2;  
          }else{
            pr_sc_count --;
            if(pr_sc_count < 0) pr_sc_count = 0;  
          }
            
          //通常時は PrScは0x12,0x7CのPS2スキャンコードを2つ受信したら98へコピーキーを送る
          if(pr_sc_count == 2){
            pc98key_send(KEY98_COPY);
            keyCode = NA_CODE;
          }else
          //Shift,Ctrlとの同時押しは PrScは0x7CのみのPS2スキャンコードを受信したら98へコピーキーを送る
          if(pr_sc_count == 1 && (shift || ctrl)){
            pc98key_send(KEY98_COPY);
            keyCode = NA_CODE;
          }else
          //PrScキーのリリース
          if(pr_sc_count == 0){
            pc98key_send(KEY98_COPY | R_CODE);
            keyCode = NA_CODE;
          }else{
            keyCode = NA_CODE;
          }
        }
      }

      if(keyCode == NA_CODE){
        keybreak = false;
        shift = false;
        ctrl = false;
        alt = false;
        e_head = E_HEAD_NONE;
        return;   
      }
      e_head = E_HEAD_NONE;

      if(keybreak){
        sendCode = keyCode | R_CODE;  
        keybreak = false;
      }else{
        sendCode = keyCode;  
      }

      //シフト状態の記録
      if(sendCode == KEY98_SHIFT){
        shift = true;
      }
      if(sendCode == (KEY98_SHIFT | R_CODE)){
        shift = false;   
      }

      //ctrl状態の記録
      if(sendCode == KEY98_CTRL){
        ctrl = true;
      }
      if(sendCode == (KEY98_CTRL | R_CODE)){
        ctrl = false;   
      }

      //alt状態の記録
      if(sendCode == KEY98_GRPH){
        alt = true;
      }
      if(sendCode == (KEY98_GRPH | R_CODE)){
        alt = false;   
      }
      
      //mySerial.write(sendCode);
      pc98key_send(sendCode);
}

void pc98key_send(uint8_t data){
  
  while(digitalRead(RDY) == HIGH) delayMicroseconds(1); //送信不可より待機
  
  //if(digitalRead(RST) == LOW) delayMicroseconds(30);  //リセット要求
  
  if(digitalRead(RTY) == LOW){ // RTYオンなら直前のキーを再送信する
      mySerial.write(prev);
  }else{           //  現在キーを直前のキーに保存
      mySerial.write(data);
      prev = data;

#ifdef DEBUG
      Serial.print("Send : ");
      Serial.print(data,HEX);
      Serial.println("");
#endif    
  }

  delayMicroseconds(500);

 
}

void pc98key_command(void){

  uint8_t c,up_c; //cの上位4bit保持用

  while(mySerial.available()>0){

    c = mySerial.read();
    up_c = c & 0xf0; //上位4bit

#ifdef DEBUG
      Serial.print("Read : ");
      Serial.print(c,HEX);
      Serial.println("");
#endif     

    //コマンドが0x9_以外はリセットかける。 ログでは0xFF,0x7_などが起動時に流れているが委細不明
    if(up_c != 0x90){
      kana_f = 0;
      caps_f = 0;
      num_f = 1;
      
      continue;   
    }

    // コマンド処理
    switch(c){
    case 0x95: // Windows,Appliキー制御
        pc98key_send(ACK);
        while(!mySerial.available());
        c = mySerial.read();
        // c=0x00 通常
        // c=0x03 Windowsキー,Applicationキー有効
        pc98key_send(ACK);
        break;
        
    case 0x96: // モード識別　取り敢えず通常モードを送信しておく.
        pc98key_send(ACK);
        delayMicroseconds(500);
        pc98key_send(0xA0);
        delayMicroseconds(500);
        pc98key_send(0x85);   // 0x85=通常変換モード, 0x86=自動変換モード
        break;
        
    case 0x9C:{ // キーボードリピート制御
        pc98key_send(ACK);
        while(!mySerial.available());
        c = mySerial.read();
#ifdef DEBUG
        Serial.print("9C-DATA : ");
        Serial.print(c,HEX);
        Serial.println("");
#endif       
        pc98key_send(ACK);
        break;
    }
    case 0x9D:   // LED 制御
        pc98key_send(ACK);
        while(!mySerial.available());
        c = mySerial.read();
        up_c = c & 0xf0; //上位4bit

        if(up_c == 0x70){ //LED状態の通知
            //状態の記録
            //num_f  = ((c & 0x01) == 0x01);
            caps_f = ((c & 0x04) == 0x04);
            kana_f = ((c & 0x08) == 0x08);
            
            pc98key_send(ACK);
        
        }else if(up_c == 0x60){ // LED状態読み出し
            c = 0;
            if(num_f)  c |= 0x01;
            if(caps_f) c |= 0x04;
            if(kana_f) c |= 0x08;
            pc98key_send(c);
        }else{                     
            pc98key_send(NACK);
        }
        break;
        
    case 0x9F:  // check keyboard type
        pc98key_send(ACK);
        delayMicroseconds(500);
        pc98key_send(0xA0);
        delayMicroseconds(500);
        pc98key_send(0x80);  // 新キーボードを送信
        break;

    default:
        // 不明コマンドは NACK で応答
        pc98key_send(NACK);
        break;            
    }    
    return;
  }

}

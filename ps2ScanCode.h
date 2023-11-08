 
static const uint8_t KBD_BUFFER_SIZE = 12;
volatile uint8_t kbd_in, kbd_out;
uint8_t kbd_buffer[KBD_BUFFER_SIZE];  //  fifo key buffer 
 
void clear_buffer() {
  cli();
  kbd_in = kbd_out = 0;
  sei();  
}
 
bool put_buffer(uint8_t s) {
  uint8_t rx = kbd_in + 1;
  if (rx >= KBD_BUFFER_SIZE) rx = 0;
  if (rx == kbd_out) return false; // buffer is full.  
  kbd_buffer[kbd_in] = s;
  kbd_in = rx;    
  return true;  
}
 
uint8_t get_buffer() {
  if (kbd_in == kbd_out) return 0;  // empty.
  uint8_t s = kbd_buffer[kbd_out];
  cli();
  if (++kbd_out >= KBD_BUFFER_SIZE) kbd_out = 0;
  sei();    
  return s;    
}
 
uint8_t cmd_to_send;
 
typedef enum { None = 0, Idle, Receiving, WaitForStart, Sending }  bus_state_t;
volatile bus_state_t bus_state;
 
void  clk_interrupt() {
  volatile static uint8_t data = 0;
  volatile static uint8_t clocks = 0;
  volatile static uint8_t par = 0;
  switch(bus_state) {
  case Idle:
    clocks = 0;
    data = 0;
    //digitalWrite(LED, 1);
    bus_state = Receiving; // found Start bit 
    break;
  case Receiving:
    clocks++;
    if (clocks < 9) {
      data = data >> 1 | (digitalRead(PS2_DATA) ? 0x80 : 0);
      if (clocks == 8)   // パリティとストップビットは無視。
        put_buffer(data); // とりあえずバッファフルは見ない。
    } else if (clocks == 10) {    // STOP bit.
      bus_state = Idle;
      //digitalWrite(LED, 0);
    }
    break;
  case WaitForStart:   // Start bit will be fetched. (DATA is LOW)
    par = 0;
    clocks = 0;
    data = cmd_to_send;
    bus_state = Sending;
    break;      
  case Sending:
    clocks++;
    if (clocks == 9)  // parity
      digitalWrite(PS2_DATA, par & 1 ? LOW : HIGH);
    else if (clocks == 10)  // STOP bit timing.
      pinMode(PS2_DATA, INPUT); // release DATA
    else if (clocks == 11)  // ACK bit from keyboard.
      bus_state = Idle;            
    else if (clocks > 0 && clocks < 9) {  // send data bits.
      if (data & 1) {
        digitalWrite(PS2_DATA, HIGH);
        par++;
      } else 
        digitalWrite(PS2_DATA, LOW);
      data = data >> 1;
    }    
    break;
  }    
}
 
bool send_command(uint8_t cmd, uint8_t resp_count = 1, uint8_t* resp = 0) {
  int timeout = 15;  
  unsigned long start = millis();
  while(bus_state != Idle && millis() - start < timeout)  
    delayMicroseconds(200);
  clear_buffer();    
  cmd_to_send = cmd;
  bus_state = WaitForStart;
  pinMode(PS2_CLK, OUTPUT);  // 
  digitalWrite(PS2_CLK, 0);  // drive LOW.
  delayMicroseconds(100);    // at least 100usec.
  pinMode(PS2_DATA, OUTPUT);  // drive LOW (START bit)
  digitalWrite(PS2_DATA ,0);
  delayMicroseconds(50);
  pinMode(PS2_CLK, INPUT);  // release clock. CLK goes to HIGH.
  
  start = millis();
  while(bus_state != Idle && millis() - start < timeout) 
    delayMicroseconds(100);
 
  uint8_t ret = 0;
  for(int8_t i = 0; i < resp_count; i++) {
    char tmp[20];
    start = millis();
    if (cmd == 0xff && i == 1)
      timeout = 500;  // for Basic Assuarance Test.
    while((ret = get_buffer()) == 0 && millis() - start < timeout)  // タイムアウトチェックする
      ;
    if (resp)
      resp[i] = ret;      

#ifdef DEBUG    
    //sprintf(tmp, "cmd=%02X, resp=%02X", cmd, ret); //sprintfは遅くて処理落ちするので
    //Serial.println(tmp);
    Serial.print("PS2Send : cmd=");
    Serial.print(cmd,HEX);
    Serial.print(" resp=");
    Serial.println(ret,HEX);
#endif

  }

  return (ret == 0xfa);
}
 
bool kbd_reset() {
  uint8_t tmp[2];
  send_command(0xff, 2, tmp); // reset keyboard. 
  if (tmp[0] == 0xfa && tmp[1] == 0xaa)
    return true;
  return false;    
}
 
uint8_t kbd_led_state = 0;
bool kbd_led(uint8_t led) {
  bool f = send_command(0xed); // LED
  if (f) {
    kbd_led_state = led;
    send_command(led); // LED parameter.
  } else
    kbd_reset();
  return f;
}
 
void toggle_led(uint8_t led) {
  uint8_t new_led = kbd_led_state;
  if (new_led & led)  new_led &= ~led;
  else                new_led |= led;
  kbd_led(new_led);  
}
 

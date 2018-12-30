/*
 * IRIG-B Signal Generator sample code
 * 12/31/2018  Y.Kobayashi(@kobayutapon)
 */

#include <MsTimer2.h>
#include <DS3232RTC.h>

#define IRIG_OUT        8
#define RTC_INT_1SEC    9

enum {
  SEND_STATE_PR = 0,
  SEND_STATE_P1,
  SEND_STATE_P2,
  SEND_STATE_P3,
  SEND_STATE_P4,
  SEND_STATE_P5,
  SEND_STATE_P6,
  SEND_STATE_P7,
  SEND_STATE_P8,
  SEND_STATE_P9,
  SEND_STATE_P0,
};

unsigned int g_time_count = 0;
unsigned int pulse_width_count = 0;

unsigned short send_byte[10];
unsigned char g_send_state = SEND_STATE_PR;
unsigned int g_bitmask = 0x0200;
unsigned char g_pulse_high_width = 0;
bool          g_pulse_low = false;
bool          g_update_time = false;



void putTime( tmElements_t tm )
{
  Serial.print(tm.Year + 1970, DEC); //年は1970年からの年数で記録されるので1970を足して表示
  Serial.print("年");
  Serial.print(tm.Month, DEC);//月を表示
  Serial.print("月");
  Serial.print(tm.Day, DEC);//日を表示
  Serial.print("日");
  Serial.print(tm.Hour, DEC);//時を表示
  Serial.print("時");
  Serial.print(tm.Minute, DEC);//分を表示
  Serial.print("分");
  Serial.println(tm.Second, DEC);//秒を表示して改行
  
}

unsigned short days[12] = { 31, 28, 31, 30,31,30, 31,31,30,31,30, 31};

void setTimeBit( tmElements_t tm )
{
  int year = (tm.Year + 1970) % 100;  // 西暦の下2桁をセット
  int dayofyear = 0;

  for( int i=0; i<tm.Month - 1; i++ ) {
    dayofyear += days[i]; 
  }
  dayofyear += tm.Day;

  // うるう年の計算
  if ((tm.Year % 4) == 0 ) {
    if ((tm.Year % 100) == 0 ) {
      if ((tm.Year % 400) == 0 ) {
        // うるう年
        dayofyear++;      
      }
    } else {
      // うるう年
      dayofyear++;
    }
  }
  
  send_byte[0] = ((tm.Second / 10) << 5) | (tm.Second % 10);  // 秒をセット
  send_byte[1] = ((tm.Minute / 10) << 5) | (tm.Minute % 10);  //  分をセット
  send_byte[2] = ((tm.Hour / 10) << 5) | (tm.Hour % 10);      // 時をセット
  send_byte[3] = (((dayofyear % 100) / 10) << 5) | (dayofyear % 10);
  send_byte[4] = (0x00 << 5) | (dayofyear / 100 );
  send_byte[5] = ((year / 10) << 5) | (year % 10);    // 年をセット
  send_byte[6] = 0x0000;
  send_byte[7] = 0x0000;
  send_byte[8] = 0x0000;
  send_byte[9] = 0x0000;

}

// 1msec interrupt handler(IRIG Code generate)
void SendPulse()
{
  if ( g_time_count % 10 == 0 ){
    // 9ビット分送り終わったら次のバイトに進む
    if ( g_bitmask >= 0x0400) {
      g_bitmask = 0x0001;
      g_send_state++;
      if ( g_send_state > SEND_STATE_P0 ) {
        g_send_state = SEND_STATE_PR;
        g_bitmask = 0x0200;

        MsTimer2::stop();

      }
    }
    
    // 
    if ( g_bitmask == 0x0200 ) {
      g_pulse_high_width = 8;
    } else {
      if ( (send_byte[g_send_state - 1] & g_bitmask) != 0 ) {
        g_pulse_high_width = 5;
      } else {
        g_pulse_high_width = 2;
      }
    }
    g_pulse_low = false;
    digitalWrite(IRIG_OUT, HIGH);

     g_bitmask <<= 1;
     if (g_send_state == SEND_STATE_P1 & g_bitmask == 0x0100 ) {
       g_bitmask <<= 1;    
     }
     pulse_width_count = 0;
        
  } else {
    if (( pulse_width_count >= g_pulse_high_width ) && ( g_pulse_low == false)) {
      digitalWrite(IRIG_OUT, LOW);
      g_pulse_low = true;     
    }
  }
  
  g_time_count++;
  pulse_width_count++;
}




// 1sec interrupt handler(PIN2)
void Interrupt1sec( void )
{
  g_update_time = true;
}


// Setup
void setup() {
  // put your setup code here, to run once:
  pinMode(IRIG_OUT, OUTPUT);
  pinMode(RTC_INT_1SEC, INPUT);

  // デフォルト値設定
  send_byte[0] = (0x03 << 5) | 0x09;
  send_byte[1] = (0x05 << 5) | 0x09;
  send_byte[2] = (0x02 << 5) | 0x03;
  send_byte[3] = (0x06 << 5) | 0x04;
  send_byte[4] = (0x00 << 5) | 0x03;
  send_byte[5] = (0x01 << 5) | 0x08;;
  send_byte[6] = 0x0000;
  send_byte[7] = 0x0000;
  send_byte[8] = 0x0000;
  send_byte[9] = 0x0000;

  g_bitmask = 0x0100;
  g_send_state = SEND_STATE_PR;

  Serial.begin(115200);
  Serial.println("IRIG-B Generator Start....");

  // RTC Time の設定
  setSyncProvider(RTC.get);
  setTime(RTC.get());

  RTC.squareWave(SQWAVE_1_HZ);

  attachInterrupt(0, Interrupt1sec, RISING );

//--------- --------
  tmElements_t tm;
  RTC.read(tm);

  if ((tm.Year + 1970) < 2018 ) {
    // ここに来た場合はRTCが設定されていない状態なので適切に処理すること。
  }

  putTime(tm);
  
// ------------------
  MsTimer2::set(1, SendPulse );  
}

void loop() {
  if ( g_update_time == true ) {
    g_update_time = false;
    tmElements_t tm;
    RTC.read(tm);  
    setTimeBit(tm);
    MsTimer2::start();
    putTime(tm);
  
  }
}

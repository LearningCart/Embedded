
#define SPI_BUS_FREQ (100 * 1024)
// CLK Pulse width in microseconds 
#define SPI_BUS_CLK_WIDTH ((1/SPI_BUS_FREQ) * 1000000)

#define IO0     2
#define IO1     3
#define IO2     4
#define IO3     5

#define RESET   IO3

#define CS      6
#define CLK     7

void CS_Low(void)
{
  digitalWrite(CS, LOW);
}

void CS_High(void)
{
  digitalWrite(CS, HIGH);
}

void resetChip(void)
{
  pinMode(RESET, OUTPUT);
  CS_Low();
  digitalWrite(RESET, HIGH);
  delay(10);
  digitalWrite(RESET, LOW);
  delay(10);
  digitalWrite(RESET, HIGH);
  CS_High();
}

void ClkPulse(void)
{
  digitalWrite(CLK, LOW);
  delayMicroseconds(SPI_BUS_CLK_WIDTH);
  digitalWrite(CLK, HIGH);
  delayMicroseconds(SPI_BUS_CLK_WIDTH);
}

#define IN  (0xAA)
#define OUT  (0x55)
// default Out
void changeDirection(uint8_t direction = OUT)
{
  if(IN == direction)
  {
    pinMode(IO3, INPUT); pinMode(IO2, INPUT);   pinMode(IO1, INPUT);  pinMode(IO0, INPUT);
  }else
  {
    pinMode(IO3, OUTPUT); pinMode(IO2, OUTPUT); pinMode(IO1, OUTPUT); pinMode(IO0, OUTPUT);
  }
}

#define SINGLE_SPI   (0x11)
#define QUAD_SPI     (0x22)
#define QUAD_SPI_OPI (0x33)

uint8_t mask[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

int datapins[] = { IO3, IO2, IO1, IO0 };
void sendByte( uint8_t data, uint8_t mode = SINGLE_SPI)
{
  uint8_t index;
  CS_Low();
  if(SINGLE_SPI == mode)
  {
    for(index = 0; index < 8; index++)
    {
      digitalWrite(IO0, (data & mask[index]) ? HIGH : LOW);
      ClkPulse();
    }
  } else if( QUAD_SPI == mode )
  {
    for(index = 0; index < 4; index++)    
    {
      digitalWrite ( datapins[index] , (data & mask [index]) ? HIGH : LOW);
    }
    ClkPulse();
    // Send lower nibble (+ 4 )
    for(index = 0; index < 4; index++)    
    {
      digitalWrite ( datapins[index] , (data & mask [index + 4 ]) ? HIGH : LOW);
    }
    ClkPulse();
  }else 
  {
    // TBD
  }
  CS_High();
}

uint8_t getByte(uint8_t mode = SINGLE_SPI)
{
  uint8_t index;
  uint8_t data;
  uint8_t lower;
  uint8_t upper;

  changeDirection(IN); // Change direction to input TBD: Internal pull up?
  CS_Low();
  if(SINGLE_SPI == mode)
  {
    data = 0x00;
    for(index = 0; index < 8; index++)
    {
      data = digitalRead(IO1) != 0 ? 1 : 0;
      ClkPulse();
      data <<= 1;
    }
  } else if( QUAD_SPI == mode )
  {
    data = lower = upper = 0x00;
    for(index = 0; index < 4; index++)    
    {
      upper |= (digitalRead( datapins[index] ) != 0 ? 1 : 0);
      upper <<= 1;
    }
    ClkPulse();
    for(index = 0; index < 4; index++)    
    {
      lower |= digitalRead( datapins[index] ) != 0 ? 1 : 0;
      lower <<= 1;
    }
    ClkPulse();
  }else 
  {
    // TBD
  }
  data = (((upper << 4) | lower) & 0xFF);
  CS_High();
  changeDirection(); // Change back to output direction
  return data;
}


void sendDummy(void)
{
  uint8_t index;
  for(index = 0; index < 4; index++)
  {
    digitalWrite(datapins[index], LOW);
  }
  /* Generate 4 dummy clock pulse */
  ClkPulse();
  ClkPulse();
  ClkPulse();
  ClkPulse();
}

void sendAddress(uint32_t address )
{
  uint8_t byte;
  address &= 0x00FFFFFF;

  byte = (address >> 16) & 0xFF;
  sendByte(byte);
  byte = (address >> 8) & 0xFF;
  sendByte(byte);
  byte = (address) & 0xFF;
  sendByte(byte);
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting QSPI emulator");
  pinMode(CS,  OUTPUT);
  pinMode(CLK, OUTPUT);
  resetChip();
  changeDirection();
  CS_High();
  
}

void loop() {
  static unsigned long lastwrite = 0;
  static unsigned long lastread  = 0;
  unsigned long current;
  uint8_t data;

  current = millis();
  if(current > (lastwrite + 500))
  {
    lastwrite = current;
    sendByte(0x9F);
  }else if( current > (lastread + 800))
  {
    lastread = current;
    data = getByte();
    Serial.print("0x");
    Serial.print(data < 16 ? "0" : "");
    Serial.print(data, HEX);
    Serial.print(" ");
  }
  // put your main code here, to run repeatedly:

}

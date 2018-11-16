#include	"msp430.h"
#include	<string.h>
 
// rev 1.4
//#define RXD        BIT2 
//#define TXD        BIT1 


// rev 1.5
#define RXD        BIT1
#define TXD        BIT2 

#define TXLED      BIT0
#define RXLED      BIT6

int BUFFER_SIZE = 300;

void setup(){
  // Stop Watch dog timer
  WDTCTL = WDTPW + WDTHOLD;         

   // Set DCO to 1 MHz
  BCSCTL1 = CALBC1_1MHZ;            
  DCOCTL = CALDCO_1MHZ;

   P1DIR &= ~RXD;   
   P1OUT =  0;                 

   // set up LEDs:
   P1DIR |= TXLED + RXLED;                          
   P1OUT &= ~(TXLED + RXLED); 

   // Select TX and RX functionality for P1.1 & P1.2
   P1SEL  = RXD + TXD ;                     
   P1SEL2 = RXD + TXD; 

   // Have USCI use System Master Clock: AKA core clk 1MHz
   UCA0CTL1 |= UCSSEL_2; 
   // 1MHz 4800, see user manual                  
   UCA0BR0 = 0xD0;                          
   UCA0BR1 = 0;  

   // Modulation UCBRSx = 1 and start USCI state machine
   UCA0MCTL = UCBRS0;                      
   UCA0CTL1 &= ~UCSWRST;   
   
   // Enable interrupts
   __bis_SR_register(GIE);     
}

// Accepts a character pointer to an array and prints in over serial
void UART_TX(char * tx_data){
  unsigned int i=0;
  while(tx_data[i]){ // Increment through array, look for null pointer (0) at end of string
      while ((UCA0STAT & UCBUSY)); // Wait if line TX/RX module is busy with data
      UCA0TXBUF = tx_data[i]; // Send out element i of tx_data array on UART bus
      if (UCA0TXBUF == '\n'){ break; }
      i++; // Increment variable for array address
    }
}


void main(){
  char byteGPS = -1;                 // Current read byte
  char line[BUFFER_SIZE];              // Buffer to hold GPS raw string
  char fieldBuffer[80] = "";                  // buffer to hold a field in GPS raw string 
  int fieldLength = 0;
  char commandGPS[7] = "$GPGGA";    // String related to messages
  int commandLength = 6;
  int counter=0;
  int correctness=0;
  int lineCounter=0;
  int numOfFieldsInterested = 8;    // the first n fields of interest in GPGGA message (refer to NMEA specification for details)
  int index[numOfFieldsInterested];

  setup();
  memset(line, 0, BUFFER_SIZE);

  while(1){
    // check if the board have recieved a new byte from GPS module
    if ((IFG2 & UCA0RXIFG)){
      byteGPS = UCA0RXBUF;

      line[lineCounter] = byteGPS;   // put data read in the buffer
      lineCounter++;

      IFG2 &= ~UCA0RXIFG;          // tell the board that we have read the byte

      // if the transmission has finished, start parsing
      if (byteGPS == '\n'){
        counter=0;
        correctness=0;      	
      	// test if the received command starts by $GPGGA
      	for (int i =0 ; i < commandLength; i++){
      		if (line[i] == commandGPS[i]){
      			correctness++;
      		}
      	}

      	if (correctness==commandLength){
          int k = 0;
          while(line[k] && k < BUFFER_SIZE){
            if (line[k]==',' || line[k]=='*'){
              index[counter]=k;
              counter++;
            }
            k++;       
          }

          UART_TX("GPS raw string: ");
          UART_TX(line);        // transmit data to laptop        

          for (int i=0;i<numOfFieldsInterested;i++){
            fieldLength = index[i+1] - index[i] - 1;

            switch(i){
              case 0 :
                UART_TX("Time in UTC (HhMmSs): ");
                break;
              case 1 :
                UART_TX("Latitude: ");
                break;
              case 2 :
                UART_TX("Direction (N/S): ");
                break;
              case 3 :
                UART_TX("Longitude: ");
                break;
              case 4 :
                UART_TX("Direction (E/W): ");
                break;
              case 5:
                UART_TX("GPS Quality: ");                
                break;
              case 6:
                UART_TX("Number of satellites used: ");
                break;
              default:
                continue;
            }
            
            strncpy(fieldBuffer, line+index[i]+1, fieldLength);
            UART_TX(fieldBuffer);
            UART_TX("\n");

            memset(fieldBuffer, 0, 80);
          }

          __delay_cycles(10000000);  // delay for 10s before next transmission, the unit of delay_cycle is microsecond
      	}

        lineCounter=0;              	
      }
    }
  }
}
#include <stdint.h>
#include "PLL.h"
#include "SysTick.h"
#include "uart.h"
#include "tm4c1294ncpdt.h"
#include "VL53L1X_api.h"
#include <stdio.h>

// I2C address for VL53L1X
#define SENSOR_ADDRESS 0x29

// States
volatile uint8_t isScanning = 0;
volatile uint8_t stepMode = 0;
volatile int currentPos = 0;
volatile uint8_t directionFlag = 0;
volatile int scanningBackwards = 0;

// Sensor and Motor
uint16_t lidar = SENSOR_ADDRESS;

//---------------------------------------- GPIO Initialization ----------------------------------------
void InitPorts(void);
void InitI2C(void);
void StopMotor(void);
void motor(int steps, int dir);
void LED0_Update(int blink);
void LED1_Update(void);
void LED2_Update(void);
void BlinkActivityLED(int interval);

//---------------------------------------- PORTS ----------------------------------------
void InitPorts(void) {
    // H: Stepper output
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R7;
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R7) == 0) {}
    GPIO_PORTH_DIR_R |= 0x0F;
    GPIO_PORTH_DEN_R |= 0x0F;

    // J: Buttons
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R8;
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R8) == 0) {}
    GPIO_PORTJ_DIR_R &= ~0x03;
    GPIO_PORTJ_DEN_R |= 0x03;
    GPIO_PORTJ_PUR_R |= 0x03;

    // N: LED indicators
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12;
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R12) == 0) {}
    GPIO_PORTN_DIR_R |= 0x03;
    GPIO_PORTN_DEN_R |= 0x03;

    // F: More LEDs
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R5) == 0) {}
    GPIO_PORTF_DIR_R |= 0x11;
    GPIO_PORTF_DEN_R |= 0x11;
			
		// E: To Showcase the PWM on AD3 (output)
		SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
		while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R4) == 0) {}
		GPIO_PORTE_DIR_R |= 0x01;
		GPIO_PORTE_DEN_R |= 0x01;

}

//---------------------------------------- I2C CONFIG ----------------------------------------
void InitI2C(void) {
		// Enable I2C0 and GPIOB
    SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
    while ((SYSCTL_PRGPIO_R & 0x02) == 0) {}
		// Configure PB2 (SCL) and PB3 (SDA) for I2C
    GPIO_PORTB_AFSEL_R |= 0x0C;
    GPIO_PORTB_ODR_R |= 0x08;
    GPIO_PORTB_DEN_R |= 0x0C;
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & ~0xFF00) | 0x2200;
		// Initialize I2C master function and clock
    I2C0_MCR_R = 0x10;
    I2C0_MTPR_R = 0x15;
}

//---------------------------------------- PWM ----------------------------------------
void PWM(void) {
    while (1) {
        GPIO_PORTE_DATA_R ^= 0x01; // Toggle PE0
        SysTick_Wait1us(1);       // 1ms delay -> period = 2ms for a 10kHz bus clock
    }
}


//---------------------------------------- LEDS ----------------------------------------
// LED values are based on student number - 2nd LSB -> 7

// LED 0 - PN1
// Blinks for every measurement (so it only blinks in CW direction)
void LED0_Update(int blink) {
    int blink_interval = blink;  // Blink every 16 steps for both 11.25° and 45°

    if (currentPos % blink_interval == 0) {  
        GPIO_PORTN_DATA_R |= 0x02;
        SysTick_Wait1us(10000);  
        GPIO_PORTN_DATA_R &= ~0x02;
				SysTick_Wait1us(10000);
    }
}

// LED 1 - PN0
// Flashes at beginning of scannning block (beginning of a scanning revolution)
void LED1_Update(){
		GPIO_PORTN_DATA_R |= 0x01;
    SysTick_Wait1us(10000); // 10 ms ON
    GPIO_PORTN_DATA_R &= ~0x01;
}	

// LED 2 - PF4
// Added functinality - ON if scanning (moving CW), otherwise OFF
void LED2_Update(){
		if (directionFlag == 0) {
			GPIO_PORTF_DATA_R |= 0x10;
    }else {
			GPIO_PORTF_DATA_R &= ~0x10;
		}
}

//---------------------------------------- MOTOR ----------------------------------------
void motor(int steps, int direction) {
    uint32_t delay = 1000; // Keep your original delay
		

    if (direction == 0) { // Clockwise
        for (int i = 0; i < steps; i++) {
            if (!isScanning) return; // Stop motor if toggled OFF
            
						LED0_Update(16);
						currentPos += 1;
						GPIO_PORTH_DATA_R = 0b00000011;
            SysTick_Wait1us(delay);
            GPIO_PORTH_DATA_R = 0b00000110;
            SysTick_Wait1us(delay);
            GPIO_PORTH_DATA_R = 0b00001100;
            SysTick_Wait1us(delay);
            GPIO_PORTH_DATA_R = 0b00001001;
            SysTick_Wait1us(delay);
        }
    } else if (direction == 1) { // Counterclockwise
        for (int i = 0; i < steps; i++) {
            if (!isScanning) return; // Stop motor if toggled OFF
						
						currentPos -= 1; 
						GPIO_PORTH_DATA_R = 0b00001001;
            SysTick_Wait1us(delay);
            GPIO_PORTH_DATA_R = 0b00001100;
            SysTick_Wait1us(delay);
            GPIO_PORTH_DATA_R = 0b00000110;
            SysTick_Wait1us(delay);
            GPIO_PORTH_DATA_R = 0b00000011;
            SysTick_Wait1us(delay);
        }
    }
}

// Motor Control
void StopMotor(void) {
    GPIO_PORTH_DATA_R = 0;
}

//---------------------------------------- MAIN ----------------------------------------
int main(void) {
    PLL_Init();
    SysTick_Init();
    UART_Init();
    InitPorts();
    InitI2C();

    // Initialize sensor
    uint8_t initCheck = 0;
    VL53L1X_BootState(lidar, &initCheck);
    VL53L1X_SensorInit(lidar);
    VL53L1X_StartRanging(lidar);

    // Scan control variables
    uint8_t previousButton = 1;
    uint16_t distance;
    float scanAngle = 0;
    int tick = 0;
    int scanningBackwards = 0;
    uint8_t ready = 0;

		// PWM check with AD3 
		//-> change SysTickWait before running
		//-> comment out while loop below
		//PWM();
		
    // Main execution loop
    while (1) {
			// BUTTON HANDLING
			uint8_t currentButton = GPIO_PORTJ_DATA_R & 0x01;
			if (currentButton == 0 && previousButton == 1) {
					// toggle scanning state
					isScanning = !isScanning;
					tick = 0;
					currentPos = 0;
					scanAngle = 0;
					LED2_Update(); // Shows state change
			}
			previousButton = currentButton;

			// Handle scan process
			if (isScanning) {
					directionFlag = scanningBackwards ? 1 : 0;
					LED2_Update();

					// Flash LED1 at the beginning of a scanning revolution
					if (tick == 0 && directionFlag == 0) {
							LED1_Update();
					}

					// Move motor
					motor(1, directionFlag);
					++tick;
					currentPos += (directionFlag == 0) ? 1 : -1;

					// SENSOR READ ONLY WHEN MOVING CLOCKWISE
					if ((tick & 0x0F) == 0 && directionFlag == 0) {
							do {
									VL53L1X_CheckForDataReady(lidar, &ready);
									VL53L1_WaitMs(lidar, 5);
							} while (!ready);

							// get and send distance over UART
							VL53L1X_GetDistance(lidar, &distance);
							VL53L1X_ClearInterrupt(lidar);

							scanAngle = (tick >> 4) * 11.25f;

							sprintf(printf_buffer, "%u\n", distance);
							UART_printf(printf_buffer);
							SysTick_Wait1us(100);
					}

					// End of scan (360° = 512 microsteps)
					if (tick == 512) {
							StopMotor();
							SysTick_Wait1us(1000000); 
							tick = 0;
							scanAngle = 0;
							scanningBackwards ^= 1; // Flip direction
					}
			}
			
			// Motor disabled
			else if (!isScanning) {
					StopMotor();
			}

	}
}

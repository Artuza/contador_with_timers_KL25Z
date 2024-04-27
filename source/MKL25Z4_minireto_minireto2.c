#include "MKL25Z4.h"

#define RS 0x02 // Define el pin RS del LCD (PTA1)
#define RW 0x04 // Define el pin RW del LCD (PTA2)
#define EN 0x10 // Define el pin EN del LCD (PTA4)
#define LCD_DATA GPIOD // Define el puerto de datos del LCD


void delayMs(int n);
void LCD_command(unsigned char command);
void LCD_data(unsigned char data);
void LCD_init(void);

int main (void) {
    int pulse_count = 0;
    LCD_init(); // Inicializa la LCD
    TPM0_init(); // Inicializa el TPM0 para contar pulsos

    while(1) {
        pulse_count = TPM0->CNT; // Lee el contador de pulsos
        LCD_display_number(pulse_count); // Muestra el conteo en la LCD
        delayMs(1000); // Actualiza cada segundo
    }
}



void LCD_init(void) {
	SIM->SCGC5 |= 0x200;  // Habilita el reloj para el puerto A
	SIM->SCGC5 |= 0x1000; // Habilita el reloj para el puerto D
	PORTA->PCR[1] = 0x100;  // Pone el pin PTA1 como GPIO para RS
	PORTA->PCR[2] = 0x100;  // Pone el pin PTA2 como GPIO para RW
	PORTA->PCR[4] = 0x100;  // Pone el pin PTA4 como GPIO para EN
	PORTD->PCR[0] = 0x100;  // Pone el pin PTD0 como GPIO para D4
	PORTD->PCR[1] = 0x100;  // Pone el pin PTD1 como GPIO para D5
	PORTD->PCR[2] = 0x100;  // Pone el pin PTD2 como GPIO para D6
	PORTD->PCR[3] = 0x100;  // Pone el pin PTD3 como GPIO para D7
	GPIOD->PDDR |= 0x0F;    // Pone los pines PTD0-PTD3 como salida para datos
	GPIOA->PDDR |= RS | RW | EN; // Pone los pines PTA1, PTA2 y PTA4 como salida para control
    delayMs(30);  // Espera por más de 15 ms después de VCC sube a 4.5V
    LCD_command(0x03);  // Función set: modo de 8 bits
    delayMs(10);  // Espera 5ms
    LCD_command(0x03);  // Función set: modo de 8 bits
    delayMs(1);  // Espera 160us
    LCD_command(0x03);  // Función set: modo de 8 bits
    LCD_command(0x02);  // Función set: cambia a modo de 4 bits

    // Configuración del LCD
    LCD_command(0x28);  // DL=0 (4 bits), N=1 (2 líneas), F=0 (5x8 puntos)
    LCD_command(0x06);  // ID=1 (incrementa cursor), S=0 (no desplaza pantalla)
    LCD_command(0x01);  // Limpia pantalla
    LCD_command(0x0F);  // Enciende display, cursor parpadeante
}

void LCD_command(unsigned char command) {
    LCD_DATA->PDOR &= ~0xF;  // Limpia los datos
    LCD_DATA->PDOR |= (command & 0xF0) >> 4;  // Envía los 4 bits más significativos
    GPIOA->PCOR = RS | RW;  // RS = 0, RW = 0
    GPIOA->PSOR = EN;  // EN = 1
    delayMs(0);  // Espera 1ms
    GPIOA->PCOR = EN;  // EN = 0
    delayMs(1);  // Espera 1ms

    LCD_DATA->PDOR &= ~0xF;  // Limpia los datos
    LCD_DATA->PDOR |= (command & 0x0F);  // Envía los 4 bits menos significativos
    GPIOA->PSOR = EN;  // EN = 1
    delayMs(0);  // Espera 1ms
    GPIOA->PCOR = EN;  // EN = 0
    delayMs(1);  // Espera 1ms
}

void LCD_data(unsigned char data) {
    LCD_DATA->PDOR &= ~0xF;  // Limpia los datos
    LCD_DATA->PDOR |= (data & 0xF0) >> 4;  // Envía los 4 bits más significativos
    GPIOA->PSOR = RS;  // RS = 1
    GPIOA->PCOR = RW;  // RW = 0
    GPIOA->PSOR = EN;  // EN = 1
    delayMs(0);  // Espera 1ms
    GPIOA->PCOR = EN;  // EN = 0

    LCD_DATA->PDOR &= ~0xF;  // Limpia los datos
    LCD_DATA->PDOR |= (data & 0x0F);  // Envía los 4 bits menos significativos
    GPIOA->PSOR = EN;  // EN = 1
    delayMs(0);  // Espera 1ms
    GPIOA->PCOR = EN;  // EN = 0
    delayMs(1);  // Espera 1ms
}

void TPM0_init(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK; // Habilita el reloj para el puerto C
    PORTC->PCR[12] = PORT_PCR_MUX(4); // Configura el PTC12 como entrada de TPM

    SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK; // Habilita el reloj para TPM0
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); // Configura la fuente del reloj para TPM0
    TPM0->SC = TPM_SC_CMOD(2) | TPM_SC_PS(0); // Configura el TPM para contar en flancos ascendentes
    TPM0->MOD = 0xFFFF; // Establece el valor máximo del contador
}


void LCD_display_number(int number) {
    char buffer[16];
    sprintf(buffer, "%d", number); // Convierte el número a una cadena
    LCD_command(1); // Limpia la pantalla
    for(int i = 0; buffer[i] != '\0'; i++) {
        LCD_data(buffer[i]); // Muestra cada carácter en la LCD
    }
}


void delayMs(int n) {
    int i;
    int j;
    for(i = 0 ; i < n; i++)
        for(j = 0 ; j < 7000; j++) {}
}

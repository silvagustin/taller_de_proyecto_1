/* Copyright 2014, 2015 Mariano Cerdeiro
 * Copyright 2014, Pablo Ridolfi
 * Copyright 2014, Juan Cecconi
 * Copyright 2014, Gustavo Muro
 * All rights reserved.
 *
 * This file is part of CIAA Firmware.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/** \brief Bare Metal example source file
 **
 ** This is a mini example of the CIAA Firmware.
 **
 **/

/** \addtogroup CIAA_Firmware CIAA Firmware
 ** @{ */
/** \addtogroup Examples CIAA Firmware Examples
 ** @{ */
/** \addtogroup Baremetal Bare Metal example source file
 ** @{ */

/*
 * Initials     Name
 * ---------------------------
 *
 *
 *
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * yyyymmdd v0.0.1 initials initial version
 */

/*==================[inclusions]=============================================*/
//#include "os.h"               /* <= operating system header */
//#include "ciaaPOSIX_stdio.h"  /* <= device handler header */
//#include "ciaaPOSIX_string.h" /* <= string header */
//#include "ciaak.h"            /* <= ciaa kernel header */
#include "poncho.h"       /* <= own header */
#include "letrasMatriz.h" //Contiene las constantes de todas las letras representadas en la matriz de leds

/*==================[macros and definitions]=================================*/
#define CPU lpc4337
#ifndef CPU
#error CPU shall be defined
#endif
#if (lpc4337 == CPU)
#include "chip.h"
#elif (mk60fx512vlq15 == CPU)
#else
#endif

#define defnum		3	//caracteres numeros
#define defAO		4	//caracteres letras mayusculas de A-O
#define defPZ		5	//caracteres letras mayusculas de P-Z
#define defao		6	//caracteres letras minusculas de a-o
#define defpz		7	//caracteres letras minusculas de p-z

#define letrasPorPalabra 2	//Tamaño maximo de palabra en la matriz (deberia ser 16 como minimo)
#define dimPalabraX (dimLetraX*letrasPorPalabra)+letrasPorPalabra
#define dimMatriz 8

#define dimTexto 16
#define cantFunc 3
#define cantOpciones 3

/*--------------------- Constantes de Funcion -------------------------*/
const char funciones[cantFunc][dimTexto] = {
		{"Escribir Texto  "},
		{"Enviar Texto    "},
		{"Velocidad Matriz"},
};

const char ModoEscritura[cantOpciones][dimTexto] = {
		{"Escribir Texto  "},
		{"Borrar Texto    "},
		{"Desea Salir?    "},
};

//"Escribir Texto  " escribe sobre el buffer de texto de la matriz
//"Enviar Texto    " refresca el texto de la matriz           <---------------- necesario?
//"Desplazar Texto " mueve el contenido de la matriz de izquierda a derecha rotativamente
//"Dibujo Continuo " moviendose con las flechitas dibuja en todo momento, excepto si se presiona la tecla 5.
//"Dibujo Pulsante " moviendose con las flechitas dibuja solo se dibuja si se presiona la tecla 5.

/*==================[internal data declaration]==============================*/

/*Variables globales*/
int x,y,fil,col;
int desp=150;
char despHab=0;

/*Variables de matriz*/
uint8_t matriz[dimLetraY][dimPalabraX];
uint8_t dibujo[dimMatriz][dimMatriz];

/*Variables de LCD*/
int posicion=0x00;
char Texto[dimTexto];
char c=' ';

/*Variables de Teclado*/
int caracteres=0;
char cambio=0;
long velMatriz=199999;
int velLCD;


/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
void inicializarGPIO(void){
	/* Switches */
		Chip_SCU_PinMux(1,0,MD_PUP|MD_EZI|MD_ZI,FUNC0); /* GPIO0[4], SW1 */
		Chip_SCU_PinMux(1,1,MD_PUP|MD_EZI|MD_ZI,FUNC0); /* GPIO0[8], SW2 */
		Chip_SCU_PinMux(1,2,MD_PUP|MD_EZI|MD_ZI,FUNC0); /* GPIO0[9], SW3 */
		Chip_SCU_PinMux(1,6,MD_PUP|MD_EZI|MD_ZI,FUNC0); /* GPIO1[9], SW4 */

		Chip_GPIO_SetDir(LPC_GPIO_PORT, 0,(1<<4)|(1<<8)|(1<<9),0);
		Chip_GPIO_SetDir(LPC_GPIO_PORT, 1,(1<<9),0);
	/* LEDs */
		Chip_SCU_PinMux(2,0,MD_PUP|MD_EZI,FUNC4);  /* GPIO5[0], LED0R */
		Chip_SCU_PinMux(2,1,MD_PUP|MD_EZI,FUNC4);  /* GPIO5[1], LED0G */
		Chip_SCU_PinMux(2,2,MD_PUP|MD_EZI,FUNC4);  /* GPIO5[2], LED0B */
		Chip_SCU_PinMux(2,10,MD_PUP|MD_EZI,FUNC0); /* GPIO0[14], LED1 */
		Chip_SCU_PinMux(2,11,MD_PUP|MD_EZI,FUNC0); /* GPIO1[11], LED2 */
		Chip_SCU_PinMux(2,12,MD_PUP|MD_EZI,FUNC0); /* GPIO1[12], LED3 */

		Chip_GPIO_SetDir(LPC_GPIO_PORT, 5,(1<<0)|(1<<1)|(1<<2),1);
		Chip_GPIO_SetDir(LPC_GPIO_PORT, 0,(1<<14),1);
		Chip_GPIO_SetDir(LPC_GPIO_PORT, 1,(1<<11)|(1<<12),1);

		Chip_GPIO_ClearValue(LPC_GPIO_PORT, 5,(1<<0)|(1<<1)|(1<<2));
		Chip_GPIO_ClearValue(LPC_GPIO_PORT, 0,(1<<14));
		Chip_GPIO_ClearValue(LPC_GPIO_PORT, 1,(1<<11)|(1<<12));
	//----------------------------------------------------------------------
	//Matriz 8x8
	//PinMux
		Chip_SCU_PinMux(1,3,MD_PUP|MD_EZI,FUNC0);	/* GPIO0[10], SPI_SCK	*/
		Chip_SCU_PinMux(6,4,MD_PUP|MD_EZI,FUNC0);  /* GPIO3[3], GPIO1 */
		Chip_SCU_PinMux(6,7,MD_PUP|MD_EZI,FUNC4);  /* GPIO5[15], GPIO3 */
		Chip_SCU_PinMux(6,9,MD_PUP|MD_EZI,FUNC0);  /* GPIO3[5], GPIO5 */

		Chip_SCU_PinMux(6,8,MD_PUP|MD_EZI,FUNC4);	/*	GPIO5[16], GPIO04 */
		Chip_SCU_PinMux(6,10,MD_PUP|MD_EZI,FUNC0);  /* GPIO3[6], GPIO6 */
		Chip_SCU_PinMux(6,11,MD_PUP|MD_EZI,FUNC0);  /* GPIO3[7], GPIO7 */
		Chip_SCU_PinMux(6,12,MD_PUP|MD_EZI,FUNC0);  /* GPIO2[8], GPIO8 */

	//SetDir
		Chip_GPIO_SetDir(LPC_GPIO_PORT, 3,(1<<3)|(1<<5)|(1<<6)|(1<<7),1); //GPIO1;5;6;7
		Chip_GPIO_SetDir(LPC_GPIO_PORT, 5,(1<<15)|(1<<16),1); //GPIO3 y GPIO4
		Chip_GPIO_SetDir(LPC_GPIO_PORT, 2,(1<<8),1); //GPIO8
		Chip_GPIO_SetDir(LPC_GPIO_PORT, 0,(1<<10),1);//SPI_MISO

	//Teclado matricial de 4x3

	// PinMUX
		Chip_SCU_PinMux(4,0,MD_PUP|MD_EZI|MD_ZI,FUNC0);
		Chip_SCU_PinMux(4,1,MD_PUP|MD_EZI|MD_ZI,FUNC0);
		Chip_SCU_PinMux(4,2,MD_PUP|MD_EZI|MD_ZI,FUNC0);
		Chip_SCU_PinMux(4,3,MD_PUP|MD_EZI|MD_ZI,FUNC0);
		Chip_SCU_PinMux(1,5,MD_PUP|MD_EZI|MD_ZI,FUNC0);
		Chip_SCU_PinMux(7,4,MD_PUP|MD_EZI|MD_ZI,FUNC0);
		Chip_SCU_PinMux(7,5,MD_PUP|MD_EZI|MD_ZI,FUNC0);

	// SetDIR
		Chip_GPIO_SetDir(LPC_GPIO_PORT, 2, (1<<0)|(1<<1)|(1<<2)|(1<<3), 1); // T_FIL0-3
		Chip_GPIO_SetDir(LPC_GPIO_PORT, 1, (1<<8), 0); // T_COL0
		Chip_GPIO_SetDir(LPC_GPIO_PORT, 3, (1<<13), 0); // T_COL2
		Chip_GPIO_SetDir(LPC_GPIO_PORT, 3, (1<<12), 0); // T_COL1

	//----------------------------------------------------------------------------
	//LCD
	//PinMux
		Chip_SCU_PinMux(4,4,MD_PUP|MD_EZI,FUNC0);//LCD1
		Chip_SCU_PinMux(4,5,MD_PUP|MD_EZI,FUNC0);//LCD2
		Chip_SCU_PinMux(4,6,MD_PUP|MD_EZI,FUNC0);//LCD3
		Chip_SCU_PinMux(4,10,MD_PUP|MD_EZI,FUNC4);//LCD4
		Chip_SCU_PinMux(4,8,MD_PUP|MD_EZI,FUNC4);//LCD_RS
		Chip_SCU_PinMux(4,9,MD_PUP|MD_EZI,FUNC4);//LCD_EN
	//SetDir
		Chip_GPIO_SetDir(LPC_GPIO_PORT, 2, (1<<4)|(1<<5)|(1<<6), 1);//LCD1-3
		Chip_GPIO_SetDir(LPC_GPIO_PORT, 5, (1<<12)|(1<<13)|(1<<14), 1);//RS;EN;LCD4
	//Limpiar
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  5, (1<<12)|(1<<13)|(1<<14));
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  2, (1<<4)|(1<<5)|(1<<6));
}

void delay(int n){ //Retardo de 10 ms base
	long retardo;
	int m;
	for(m=0;m<n;m++){
		for( retardo=0;retardo<velMatriz;retardo++);
	}
}

void datosLCD(int valor){
	if((valor&0x01)==0x01){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 2, 1<<4);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  2, (1<<4));
	}
	if((valor&0x02)==0x02){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 2, 1<<5);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  2, (1<<5));
	}
	if((valor&0x04)==0x04){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 2, 1<<6);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  2, (1<<6));
	}
	if((valor&0x08)==0x08){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 5, 1<<14);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  5, (1<<14));
	}
	if((valor&0x10)==0x10){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 5, 1<<12);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  5, (1<<12));
	}
	if((valor&0x20)==0x20){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 5, 1<<13);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  5, (1<<13));
	}
}

void lcd_send(void){
	Chip_GPIO_SetValue(LPC_GPIO_PORT, 5, 1<<13);
	delay(1);
	Chip_GPIO_ClearValue(LPC_GPIO_PORT, 5, 1<<13);
}

void lcd_clear(void){
	datosLCD(0x00);
	lcd_send();
	datosLCD(0x01);
	lcd_send();
}

void lcd_init(void){
	delay(5);

	datosLCD(0x03); //Function set ( Interface is 8 bits long. )
	lcd_send();

	delay(5);

	datosLCD(0x02);	//Set interface data length [0 0 1 DL | N F － －]
	lcd_send();		//(DL:8-bit/4-bit), numbers of display line
	datosLCD(0x08);	//(N:2-line/1-line) and, display font type
	lcd_send();		//(F:5×11 dots/5×8 dots)

	delay(1);

	datosLCD(0x00);	//Display ON/OFF Control
	lcd_send();		//The number of display lines and character font can not be changed after this point.
	datosLCD(0x0F); //1|D|C|B - Sets On/Off of all display (D), cursor On/Off (C) and blink of cursor position character (B).
	lcd_send();

	delay(1);

	datosLCD(0x00);	//EntryMode	Set
	lcd_send();		//Assign cursor moving direction and enable the shift of entire display.
	datosLCD(0x06); //0 | 1 | I/D | SH --- (Increment / Decrement)
	lcd_send();

	delay(1);

	datosLCD(0x00); //Return Home
	datosLCD(0x02);
	lcd_send();

	delay(1);

	posicion=0x00;//indice inicializado
	lcd_clear();

}

void lcd_printchar(char C){
	switch(C){
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
			datosLCD(defAO|0x10);
			lcd_send();
			C = C - 0x40;
			datosLCD(C|0x10);
			break;

		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
			datosLCD(defPZ|0x10);
			lcd_send();
			C = C - 0x40;
			datosLCD(C|0x10);
			break;

		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
			datosLCD(defao|0x10);
			lcd_send();
			C = C - 0x60;
			datosLCD(C|0x10);
		break;

		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
			datosLCD(defpz|0x10);
			lcd_send();
			C = C - 0x60;
			datosLCD(C|0x10);
		break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			datosLCD(defnum|0x10);
			lcd_send();
			C = C - 0x30;
			datosLCD(C|0x10);
		break;

		case ' ':
			datosLCD(0x18);
			lcd_send();
			datosLCD(0x18);
		break;

		case '.':
			datosLCD(0x12);
			lcd_send();
			datosLCD(0x1E);

		break;

		case ',':
			datosLCD(0x12);
			lcd_send();
			datosLCD(0x1C);
		break;

		case '(':
			datosLCD(0x12);
			lcd_send();
			datosLCD(0x18);
		break;

		case ')':
			datosLCD(0x12);
			lcd_send();
			datosLCD(0x19);

		break;

		case '*':
			datosLCD(0x12);
			lcd_send();
			datosLCD(0x1A);
		break;

		case '+':
			datosLCD(0x12);
			lcd_send();
			datosLCD(0x1B);

		break;

		case '-':
			datosLCD(0x12);
			lcd_send();
			datosLCD(0x1D);
		break;

		case '/':
			datosLCD(0x12);
			lcd_send();
			datosLCD(0x1F);

		break;

		case '!':
			datosLCD(0x12);
			lcd_send();
			datosLCD(0x11);
		break;

		case '#':
			datosLCD(0x12);
			lcd_send();
			datosLCD(0x13);
		break;

		case '$':
			datosLCD(0x12);
			lcd_send();
			datosLCD(0x14);

		break;

		case '&':
			datosLCD(0x12);
			lcd_send();
			datosLCD(0x16);
		break;

		case '\'':
		case '"':
			datosLCD(0x12);
			lcd_send();
			datosLCD(0x17);
		break;

		case '%':
			datosLCD(0x12);
			lcd_send();
			datosLCD(0x15);
		break;

		case ':':
			datosLCD(0x13);
			lcd_send();
			datosLCD(0x1A);
		break;

		case ';':
			datosLCD(0x13);
			lcd_send();
			datosLCD(0x1B);
		break;

		case '<':
			datosLCD(0x13);
			lcd_send();
			datosLCD(0x1C);
		break;

		case '=':
			datosLCD(0x13);
			lcd_send();
			datosLCD(0x1D);
		break;

		case '>':
			datosLCD(0x13);
			lcd_send();
			datosLCD(0x1E);
		break;

		case '?':
			datosLCD(0x13);
			lcd_send();
			datosLCD(0x1F);
		break;

		case '_':
			datosLCD(0x15);
			lcd_send();
			datosLCD(0x1F);
		break;
		default:
			//no hace nada
			break;
	}
	lcd_send();
}

void lcd_cursor(uint8_t ddramAddress) {
    int ddaux = ddramAddress >> 4;
    ddaux |= 0x08;
    ddaux = ddaux & 0x0F;

    datosLCD(ddaux);
	lcd_send();

	ddaux = ddramAddress & 0x0F;
	datosLCD(ddaux);
	lcd_send();
}

void lcd_L1(void) {
	posicion=0x00;
	lcd_cursor(posicion);
}

void lcd_L2(void) {
	posicion=0x40;
	lcd_cursor(posicion);

}

void lcd_imprimir8(const char* s){
	int i;
	for(i=0; i<8;i++){
		lcd_printchar(s[i]);
		posicion++;
	}
}

void lcd_imprimir16(const char* s){
	int i;
	for(i=0; i<16;i++){
		lcd_printchar(s[i]);
		posicion++;
	}
}

void lcd_imprimir32(char* s){
	int i;
	lcd_L1();
	for(i=0; i<16;i++){
		lcd_printchar(s[i]);
		posicion++;
	}

	lcd_L2();
	for(i=16; i<32;i++){
		lcd_printchar(s[i]);
		posicion++;
	}
}

void datosLEDs(int valor){
	if((valor&0x01)==0x01){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 5, 1<<0);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  5, (1<<0));
	}
	if((valor&0x02)==0x02){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 5, 1<<1);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  5, (1<<1));
	}
	if((valor&0x04)==0x04){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 5, 1<<2);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  5, (1<<2));
	}
	if((valor&0x08)==0x08){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 0, 1<<14);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  0, (1<<14));
	}
	if((valor&0x10)==0x10){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 1, 1<<11);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  1, (1<<11));
	}
	if((valor&0x20)==0x20){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 1, 1<<12);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  1, (1<<12));
	}
}

void salidasTECLADO(int valor){
	if((valor&0x01)==0x01){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 2, 1<<0);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  2, (1<<0));
	}
	if((valor&0x02)==0x02){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 2, 1<<1);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  2, (1<<1));
	}
	if((valor&0x04)==0x04){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 2, 1<<2);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  2, (1<<2));
	}
	if((valor&0x08)==0x08){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 2, 1<<3);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  2, (1<<3));
	}
}

int entradasTECLADO(void){//lectura fisica de los datos del teclado
	int valorR=0;
	if((Chip_GPIO_ReadValue(LPC_GPIO_PORT,3)&(1<<13))==0){//Columna 1
		valorR += 0x01;
	}
	if((Chip_GPIO_ReadValue(LPC_GPIO_PORT,1)&(1<<8))==0){//Columna 2
		valorR += 0x02;
	}
	if((Chip_GPIO_ReadValue(LPC_GPIO_PORT,3)&(1<<12))==0){//Columna 3
		valorR += 0x04;
	}
	return valorR;
}

int leerTeclado(void){//interpretación por software de los datos del teclado
	int valor=0;
	int tecla=0;
	switch (fil){
	case 0:
		salidasTECLADO(0x0D); //Fila 1
		tecla=entradasTECLADO();
		if(tecla==1){
			valor = 1;
		}
		if(tecla==2){
			valor = 2;
		}
		if(tecla==4){
			valor = 3;
		}
		break;
	case 1:
		salidasTECLADO(0x07);//Fila 2
		tecla=entradasTECLADO();
		if(tecla==1){
			valor = 4;
		}
		if(tecla==2){
			valor = 5;
		}
		if(tecla==4){
			valor = 6;
		}
		break;
	case 2:
		salidasTECLADO(0x0B);
		tecla=entradasTECLADO();
		if(tecla==1){
			valor = 7;
		}
		if(tecla==2){
			valor = 8;
		}
		if(tecla==4){
			valor = 9;
		}
		break;
	case 3:
		salidasTECLADO(0x0E);
		tecla=entradasTECLADO();
		if(tecla==1){
			valor = 10;
		}
		if(tecla==2){

			valor = 11;
		}
		if(tecla==4){
			valor = 12;
		}
		break;
	case 4:
		fil=-1;
		break;
	default:
		break;
	}
	fil++;
	return valor;
}

void escribirLCD(int posicion){
	char funcionActiva=1;
	char contador=0;
	int boton=0;
	int tecla = leerTeclado();
	cambio=0;

	for(caracteres=0;caracteres<dimTexto;caracteres++){//Reinicializa el texto
		Texto[caracteres]=' ';
	}
	caracteres=0;

	while(funcionActiva){
		lcd_cursor(posicion);
		while(!tecla){
			tecla = leerTeclado();
		}
		switch(leerTeclado()){
			case 1://funcion 1: simbolos-1
				if(boton!=1){
					contador=0;
					boton=1;
				}
				if(contador < 15){
					c=('!'+contador);
					contador++;
				}
				else{
					c='1';
					contador=0;
				}
				lcd_printchar(c);
				break;

			case 2://funcion 2: a-b-c-A-B-C-2
				if(boton!=2){
					contador=0;
					boton=2;
				}
				if(contador < 6){
					if(contador < 3){
						c=('a'+contador);
					}
					else{
						c=('A'+contador-3);
					}
					contador++;
				}
				else{
					c=('2');
					contador=0;
				}
				lcd_printchar(c);
				break;

			case 3://funcion 3: d-e-f-D-E-F-3
				if(boton!=3){
					contador=0;
					boton=3;
				}
				if(contador < 6){
					if(contador < 3){
						c=('d'+contador);
					}
					else{
						c=('D'+contador-3);
					}
					contador++;
				}
				else{
					c=('3');
					contador=0;
				}
				lcd_printchar(c);
				break;

			case 4://funcion 7: p-q-r-s-P-Q-R-S-7
				if(boton!=4){
					contador=0;
					boton=4;
				}
				if(contador < 8){
					if(contador < 4){
						c=('p'+contador);
					}
					else{
						c=('P'+contador-4);
					}
					contador++;
				}
				else{
					c=('7');
					contador=0;
				}
				lcd_printchar(c);
				break;

			case 5://funcion 8: t-u-v-w-T-U-V-W-8
				if(boton!=5){
					contador=0;
					boton=5;
				}
				if(contador < 8){
					if(contador < 4){
						c=('t'+contador);
					}
					else{
						c=('T'+contador-4);
					}
					contador++;
				}
				else{
					c=('8');
					contador=0;
				}
				lcd_printchar(c);
				break;

			case 6://funcion 9: x-y-z-X-Y-Z-9
				if(boton!=6){
					contador=0;
					boton=6;
				}
				if(contador < 6){
					if(contador < 3){
						c=('x'+contador);
					}
					else{
						c=('X'+contador-3);
					}
					contador++;
				}
				else{
					c=('9');
					contador=0;
				}
				lcd_printchar(c);
				break;

			case 7://funcion 4: g-h-i-G-H-I-4
				if(boton!=7){
					contador=0;
					boton=7;
				}
				if(contador < 6){
					if(contador < 3){
						c=('g'+contador);
					}
					else{
						c=('G'+contador-3);
					}
					contador++;
				}
				else{
					c=('4');
					contador=0;
				}
				lcd_printchar(c);
				break;

			case 8://funcion 5: j-k-l-J-K-L-5
				if(boton!=8){
					contador=0;
					boton=8;
				}
				if(contador < 6){
					if(contador < 3){
						c=('j'+contador);
					}
					else{
						c=('J'+contador-3);
					}
					contador++;
				}
				else{
					c=('5');
					contador=0;
				}
				lcd_printchar(c);
				break;

			case 9://funcion 6: m-n-o-M-N-O-6
				if(boton!=9){
					contador=0;
					boton=9;
				}
				if(contador < 6){
					if(contador < 3){
						c=('m'+contador);
					}
					else{
						c=('M'+contador-3);
					}
					contador++;
				}
				else{
					c=('6');
					contador=0;
				}
				lcd_printchar(c);
				break;

			case 10://funcion 10: BOTON ACEPTAR
				if(boton!=10){
					contador=0;
					boton=10;
				}
				if(cambio==2){//si antes se presiono ESCAPE, sale de la funcion escribir
					funcionActiva=0;
				}
				else{//escribe caracter
					if(cambio!=0){
						lcd_L1();
						lcd_imprimir16(ModoEscritura[0]);
						cambio=0;
						lcd_cursor(posicion);
					}
					if(c!='\0'){
						Texto[caracteres]= c;
						if(caracteres<dimTexto){
							posicion++;
							caracteres++;
						}
						c='\0';
					}
				}
				break;

			case 11://funcion 11: espacio-simbolos-0
				if(boton!=11){
					contador=0;
					boton=11;
				}
				if(contador<6){
					if(contador==0){
						c=(' ');
						contador++;
					}
					else{
						c=('0'+9+contador);
						contador++;
					}
				}
				else{
					c=('0');
					contador=0;
				}
				lcd_printchar(c);
				break;

			case 12://funcion 12: BOTON ESCAPE
				if(boton!=12){
					if(cambio!=2){
						lcd_L1();
						lcd_imprimir16(ModoEscritura[2]);
						lcd_cursor(posicion);
						cambio=2;
					}
					boton=12;
					contador=0;
				}
				else{
					if(caracteres > 0){
						if(cambio!=1){
							lcd_L1();
							lcd_imprimir16(ModoEscritura[1]);
							cambio=1;
						}

						if(posicion>0){
							lcd_cursor(posicion-1);
							lcd_printchar(' ');
							posicion--;
							caracteres--;
						}
						else{
							lcd_printchar(' ');
							posicion=0;
							caracteres=0;
						}
					}
					else{
						if(cambio == 2){
							lcd_L1();
							lcd_imprimir16(ModoEscritura[0]);
							cambio=0;
							lcd_cursor(posicion);
						}
						else{
							lcd_L1();
							lcd_imprimir16(ModoEscritura[2]);
							lcd_cursor(posicion);
							cambio=2;
						}
					}
				}
				break;

			default:
				//No hace nada - Ninguna tecla presionada
				break;
		}
	}
}

int elegirFuncion(){
	int i=0;
	int tecla=0;
	char ok=1;
	while(ok){//mientras no se presione la letra Ok
		lcd_L2();
		lcd_imprimir16(funciones[i]);

		while(tecla!=7&&tecla!=9&&tecla!=10){
			tecla=leerTeclado();
		}

		if(tecla==9){//Flecha Derecha
			if(i<(cantFunc-1)){
				i++;
			}
			else{
				i=0;
			}
		}
		else{
			if(tecla==7){//Flecha Izquierda
				if(i>0){
					i--;
				}
				else{
					i=(cantFunc-1);
				}
			}
			else{
				if(tecla==10){
					ok=0;
				}
			}
		}
		tecla=leerTeclado();
	}
	return i;
}

void imprimirPosicionM(const int x, const int y){
	lcd_L2();
	lcd_imprimir8("Posicion");

	lcd_printchar(' ');
	lcd_printchar('X');
	lcd_printchar(':');
	lcd_printchar(x+0x30);

	lcd_printchar(' ');
	lcd_printchar('Y');
	lcd_printchar(':');
	lcd_printchar(y+0x30);
}

void datosMATRIZ(int valor){
	if((valor&0x01)==0x01){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 3, 1<<3);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  3, (1<<3));
	}
	if((valor&0x02)==0x02){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 5, 1<<15);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  5, (1<<15));
	}
	if((valor&0x04)==0x04){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 3, 1<<5);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  3, (1<<5));
	}
	if((valor&0x08)==0x08){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 0, 1<<10);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  0, (1<<10));
	}

	if((valor&0x10)==0x10){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 3, 1<<7);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  3, (1<<7));
	}
	if((valor&0x20)==0x20){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 2, 1<<8);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  2, (1<<8));
	}
	if((valor&0x40)==0x40){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 3, 1<<6);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  3, (1<<6));
	}
	if((valor&0x80)==0x80){
		Chip_GPIO_SetValue(LPC_GPIO_PORT, 5, 1<<16);
	}else{
		Chip_GPIO_ClearValue(LPC_GPIO_PORT,  5, (1<<16));
	}
}

void cargarLetra(uint8_t const letra[dimLetraY][dimLetraX],uint8_t matriz[dimLetraY][dimPalabraX],const int pos){
	int y,x;
	int posInicial= (dimLetraX*pos);
	int posFinal=dimLetraX*(pos+1);//--------------------------pos

	for(y=0;y<dimLetraY;y++){
		for(x=posInicial;x<posFinal;x++){
			//matriz[dimLetraY-1-i][posFinal-1-j]=letra[i][j];
			matriz[y][x]=letra[y][x%6];
		}
		//matriz[dimLetraY-1-i][posFinal]=1;
	}
}

void imprimirDibujo(uint8_t dibujo[dimMatriz][dimMatriz]){
	int i,j,m,salida;
	for(i=0;i<dimMatriz;i++){
		for(j=0;j<dimMatriz;j++){
			salida = ((dibujo[i][j]*(i+2))<<4)|(dibujo[i][j]*j);// (((0 o 1) *(i+Altura))<<4) | (0 o 1) *j
			salida &= 0x77;//apaga los bits de enable 1 de los demultiplexores
			datosMATRIZ(salida);
			for(m=0;m<600;m++);
		}
	}
}

void setPosIndices(int PosX, int PosY){
	x=PosX;
	y=PosY;
}

void incY(){
	if(y<dimMatriz){
		y++;
	}
	else{
		y=0;
	}
}

void incX(){
	if(x<dimMatriz){
		x++;
	}
	else{
		x=0;
	}
}

void decY(){
	if(y>0){
		y--;
	}
	else{
		y=dimMatriz;
	}
}

void decX(){
	if(x>0){
		x--;
	}
	else{
		x=dimMatriz;
	}
}

void dibujoPulsante(){
	int m,n,salida;
	char funcionActiva=1;

	setPosIndices(0,0);
	imprimirPosicionM(x,y);

	int cuenta=0;
	int tecla= leerTeclado();
	int estado;
	cambio=0;
	while(funcionActiva){
		while(!tecla){
			tecla = leerTeclado();
		}
		switch(leerTeclado()){
			case 1://(X-1) (Y+1)
				incY();
				decX();
				cambio=1;
				break;

			case 2://(Y+1)
				incY();
				cambio=1;
				break;

			case 3://(X+1) (Y+1)
				incY();
				incX();
				cambio=1;
				break;

			case 4://(X-1) (Y-1)
				decY();
				decX();
				cambio=1;
				break;

			case 5://(Y-1)
				decY();
				cambio=1;
				break;

			case 6://(X+1) (Y-1)
				decY();
				incX();
				cambio=1;
				break;

			case 7://(X-1)
				decX();
				cambio=1;
				break;

			case 8://Cambia el estado de un Led
				if(dibujo[x][y]==1){
					dibujo[x][y] = 0;
				}else{
					dibujo[x][y]=1;
				}

				cambio=2;
				break;

			case 9://(X+1)
				incX();
				cambio=1;
				break;

			case 10://Ok borra el dibujo
				for(m=0;m<8;m++){
					for(n=0;n<8;n++){
						dibujo[m][n]=0;
					}
				}
				break;

			case 11://Posicionarse en 0
				setPosIndices(0,0);
				cambio=1;
				break;

			case 12://ESCAPE
				funcionActiva=0;
				break;

			default:
				//no hace nada - Ninguna tecla presionada
				break;
		}

		if(cambio==1){
			estado=dibujo[x][y];
			imprimirPosicionM(x,y);
			cambio=0;
		}

		cuenta++;


		for(m=0;m<8;m++){
			for(n=0;n<8;n++){
				salida = ((dibujo[m][n]*m)<<4)|(dibujo[m][n]*n);
				salida &= 0x77;
				datosMATRIZ(salida);
				//datosMATRIZ(((((dibujo[m][n])*m)<<4)|(dibujo[m][n]*n))&&(0x77));//datosMATRIZ(((((dibujo[x][y])*y)<<4)|(dibujo[x][y]*x))&&(0x77));
				//for(n=0;n<600;n++);
			}
		}


		if(cuenta == 80000){
			cuenta=0;
			if(cambio!=2){
				if(estado==0){
					estado=1;
				}else{
					estado=0;
				}
			}
			else{
				cambio=0;/*
				if(x<8){
					x++;
				}

				else{
					x=0;
					if(y<8){
						y++;
					}
					else{
						y=0;
					}
				}*/
				estado=dibujo[x][y];
			}
			datosMATRIZ((((estado)*y)<<4|(estado*x))&&(0x77));
		}
		//datosMATRIZ((((estado<<4)*y)|(estado*x))&&(0x77));
		//imprimirDibujo(dibujo);
	}
}

void siguienteLetra(int i, int pos){
	switch(Texto[i]){
		case 'A':
			cargarLetra(letraA,matriz,pos);
			break;
		case 'B':
			cargarLetra(letraB,matriz,pos);
			break;
		case 'C':
			cargarLetra(letraC,matriz,pos);
			break;
		case 'D':
			cargarLetra(letraD,matriz,pos);
			break;
		case 'E':
			cargarLetra(letraE,matriz,pos);
			break;
		case 'F':
			cargarLetra(letraF,matriz,pos);
			break;
		case 'G':
			cargarLetra(letraG,matriz,pos);
			break;
		case 'H':
			cargarLetra(letraH,matriz,pos);
			break;
		case 'I':
			cargarLetra(letraI,matriz,pos);
			break;
		case 'J':
			cargarLetra(letraJ,matriz,pos);
			break;
		case 'K':
			cargarLetra(letraK,matriz,pos);
			break;
		case 'L':
			cargarLetra(letraL,matriz,pos);
			break;
		case 'M':
			cargarLetra(letraM,matriz,pos);
			break;
		case 'N':
			cargarLetra(letraN,matriz,pos);
			break;
		case 'O':
			cargarLetra(letraO,matriz,pos);
			break;
		case 'P':
			cargarLetra(letraP,matriz,pos);
			break;
		case 'Q':
			cargarLetra(letraQ,matriz,pos);
			break;
		case 'R':
			cargarLetra(letraR,matriz,pos);
			break;
		case 'S':
			cargarLetra(letraS,matriz,pos);
			break;
		case 'T':
			cargarLetra(letraT,matriz,pos);
			break;
		case 'U':
			cargarLetra(letraU,matriz,pos);
			break;
		case 'V':
			cargarLetra(letraV,matriz,pos);
			break;
		case 'W':
			cargarLetra(letraW,matriz,pos);
			break;
		case 'X':
			cargarLetra(letraX,matriz,pos);
			break;
		case 'Y':
			cargarLetra(letraY,matriz,pos);
			break;
		case 'Z':
			cargarLetra(letraZ,matriz,pos);
			break;


		case 'a':
			cargarLetra(letraa,matriz,pos);
			break;
		case 'b':
			cargarLetra(letrab,matriz,pos);
			break;
		case 'c':
			cargarLetra(letrac,matriz,pos);
			break;
		case 'd':
			cargarLetra(letrad,matriz,pos);
			break;
		case 'e':
			cargarLetra(letrae,matriz,pos);
			break;
		case 'f':
			cargarLetra(letraf,matriz,pos);
			break;
		case 'g':
			cargarLetra(letrag,matriz,pos);
			break;
		case 'h':
			cargarLetra(letrah,matriz,pos);
			break;
		case 'i':
			cargarLetra(letrai,matriz,pos);
			break;
		case 'j':
			cargarLetra(letraj,matriz,pos);
			break;
		case 'k':
			cargarLetra(letrak,matriz,pos);
			break;
		case 'l':
			cargarLetra(letral,matriz,pos);
			break;
		case 'm':
			cargarLetra(letram,matriz,pos);
			break;
		case 'n':
			cargarLetra(letran,matriz,pos);
			break;
		case 'o':
			cargarLetra(letrao,matriz,pos);
			break;
		case 'p':
			cargarLetra(letrap,matriz,pos);
			break;
		case 'q':
			cargarLetra(letraq,matriz,pos);
			break;
		case 'r':
			cargarLetra(letrar,matriz,pos);
			break;
		case 's':
			cargarLetra(letras,matriz,pos);
			break;
		case 't':
			cargarLetra(letrat,matriz,pos);
			break;
		case 'u':
			cargarLetra(letrau,matriz,pos);
			break;
		case 'v':
			cargarLetra(letrav,matriz,pos);
			break;
		case 'w':
			cargarLetra(letraw,matriz,pos);
			break;
		case 'x':
			cargarLetra(letrax,matriz,pos);
			break;
		case 'y':
			cargarLetra(letray,matriz,pos);
			break;
		case 'z':
			cargarLetra(letraz,matriz,pos);
			break;


		case '0':
			cargarLetra(num0,matriz,pos);
			break;
		case '1':
			cargarLetra(num1,matriz,pos);
			break;
		case '2':
			cargarLetra(num2,matriz,pos);
			break;
		case '3':
			cargarLetra(num3,matriz,pos);
			break;
		case '4':
			cargarLetra(num4,matriz,pos);
			break;
		case '5':
			cargarLetra(num5,matriz,pos);
			break;
		case '6':
			cargarLetra(num6,matriz,pos);
			break;
		case '7':
			cargarLetra(num7,matriz,pos);
			break;
		case '8':
			cargarLetra(num8,matriz,pos);
			break;
		case '9':
			cargarLetra(num9,matriz,pos);
			break;

		case ' ':
			cargarLetra(espacio,matriz,pos);
			break;
		case '.':
			cargarLetra(punto,matriz,pos);
			break;
		case ',':
			cargarLetra(coma,matriz,pos);
			break;
		case '(':
			cargarLetra(parentesisA,matriz,pos);
			break;
		case ')':
			cargarLetra(parentesisC,matriz,pos);
			break;
		case '[':
			cargarLetra(corcheteA,matriz,pos);
			break;
		case ']':
			cargarLetra(corcheteC,matriz,pos);
			break;
		case '*':
			cargarLetra(asterisco,matriz,pos);
			break;
		case '+':
			cargarLetra(mas,matriz,pos);
			break;
		case '-':
			cargarLetra(menos,matriz,pos);
			break;
		case '/':
			cargarLetra(div_symbol,matriz,pos);
			break;
		case '!':
			cargarLetra(exclamacion,matriz,pos);
			break;
		case '#':
			cargarLetra(hashtag,matriz,pos);
			break;
		case '$':
			cargarLetra(pesos,matriz,pos);
			break;
		case '&':
			cargarLetra(ampersand,matriz,pos);
			break;
		case '\'':
		case '"':
			cargarLetra(comillasS,matriz,pos);
			break;
		case '%':
			cargarLetra(porcentaje,matriz,pos);
			break;
		case ':':
			cargarLetra(dosPuntos,matriz,pos);
			break;
		case ';':
			cargarLetra(puntoYcoma,matriz,pos);
			break;
		case '<':
			cargarLetra(menor,matriz,pos);
			break;
		case '=':
			cargarLetra(igual,matriz,pos);
			break;
		case '>':
			cargarLetra(mayor,matriz,pos);
			break;
		case '?':
			cargarLetra(interrogacion,matriz,pos);
			break;
		case '_':
			cargarLetra(guionBajo,matriz,pos);
			break;
		case '|':
			cargarLetra(or_simbol,matriz,pos);
			break;

		default:
			//no hace nada
			break;
	}
}

void enviaTexto(){
	int salida,m,p,i,j,retDesp,k,n;
	char funcionActiva=1;
	int letraActual=0;
	if (Texto[caracteres-1]!=' '){
		caracteres++;
	}
	if(caracteres!=0){
		siguienteLetra(letraActual,0);
		letraActual++;
		if(letraActual>caracteres-1){
			letraActual=0;
		}
		if(caracteres > 1){
			siguienteLetra(letraActual,1);
			letraActual++;
			if(letraActual>caracteres-1){
				letraActual=0;
			}
		}
		while(funcionActiva){
			switch(leerTeclado()){
				case 10://funcion 10: BOTON ACEPTAR
				case 12://funcion 12: BOTON ESCAPE
					funcionActiva=0;
					break;
				default:{
					for(i=0;i<dimLetraY;i++){
						for(j=0;j<8;j++){
							salida = ((matriz[i][j])*(i+2)<<4)|(matriz[i][j]*(j));// (((0 o 1) *(i+Altura))<<4) | (0 o 1) *j
							salida &= 0x77;//apaga los bits de enable 1 de los demultiplexores
							datosMATRIZ(salida);
							for(n=0;n<600;n++);
						}
					}
					if(retDesp>desp){
						retDesp=0;
						for(p=0;p<6;p++){
							for(m=0;m<11;m++){
								matriz[p][m] = matriz[p][m+1];
							}
						}
						k++;
						if(k>5){
							k=0;
							siguienteLetra(letraActual,1);
							letraActual++;
							if(letraActual>caracteres-1){
								letraActual=0;
							}
						}
					}
					retDesp++;

					break;
				}
			}
		}
	}else{
		lcd_L2();
		lcd_imprimir16("No hay texto ...");
	}
}

void velocidadDespMatriz(){
	int funcionActiva=1;
	int cambio=1;
	int vel;
	/*lcd_clear();
	//lcd_L1();
	lcd_imprimir16(" Desplazamiento ");*/
	while(funcionActiva){
		vel = 100 - desp/5;
		if(cambio==1){
			lcd_L2();
			lcd_imprimir8("Velocida");
			lcd_printchar('d');
			lcd_printchar(':');
			lcd_printchar(' ');
			if((vel/100)==1){
				lcd_printchar('1');
			}else{
				lcd_printchar(' ');
			}
			lcd_printchar(((vel%100)/10)+0x30);
			lcd_printchar((vel%10)+0x30);
			lcd_printchar('%');
			cambio=0;
		}
		switch(leerTeclado()){
			case 9:
				desp += 50;
				if(desp>500){
					desp=500;
				}
				cambio=1;
				break;
			case 7:
				desp -= 50;
				if(desp<0){
					desp=1;
				}
				cambio=1;
				break;
			case 10://funcion 10: BOTON ACEPTAR
			case 12://funcion 12: BOTON ESCAPE
				funcionActiva=0;
				break;
			default:
				break;
		}
	}
}

/*==================[external functions definition]==========================*/
/** \brief Main function
 *
 * This is the main entry point of the software.
 *
 * \returns 0
 *
 * \remarks This function never returns. Return value is only to avoid compiler
 *          warnings or errors.
 */
int main(void){
   //Inicializar Variables
    int pos;
    int indiceMenu;
	/* perform the needed initialization here */
	inicializarGPIO();

	datosMATRIZ(0x08);//la matriz de leds inicia apagada
	//datosLEDs(0x10); //Prender Led Rojo	lcd_init();
	lcd_init();

	lcd_imprimir16("** Bienvenido **");
	lcd_L2();
	lcd_imprimir16(" Elegir Funcion ");
	delay(120);
	while(1){
		//Imprime titulo
		lcd_clear();
		lcd_L1();
		lcd_imprimir16("  *** Menu ***  ");

		//Selección de función
		indiceMenu=elegirFuncion();

		//Imprime nombre de función
		lcd_clear();
		lcd_L1();
		lcd_imprimir16(funciones[indiceMenu]);
		lcd_L2();

		//Inicializa las variables y habilita las funciones
		switch(indiceMenu){
			case 0:
				//Escribir texto
				pos= posicion;
				escribirLCD(pos);
				break;
			case 1:
				enviaTexto();
				break;
			case 2:
				velocidadDespMatriz();
				break;
			default:
				//no hace nada
				break;
		}
	}
	return 0;
}


/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/


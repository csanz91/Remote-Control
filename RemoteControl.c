/*
 * RemoteControl.c
 *
 * Created: 20/09/2012 10:31:59
 *  Author: Cesar
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h> 

#define retro1	PD4
#define retro2	PD5
#define entrada_botones	PC0
#define salida_radio PD0
#define entrada_marcha_atras PD2
#define puerto PORTD
#define puerto_analog PORTC
#define puerto_odenador PORTB
#define DDR_puerto DDRD
#define DDR_ordenador DDRB
#define DDR_puerto_analog DDRC
#define PIN PIND


//Variables
	unsigned int VOL_UP[49]  = {0,1,0,0,0,1,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,1,0,0,1,0,0,1,0,1,0,1,0};
	unsigned int VOL_DN[49]  = {0,1,0,0,0,1,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,0,1,0,1,0};
	unsigned int MUTE[49]    = {0,1,0,0,0,1,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,1,0,1,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1,0,1,0,1,0};
	unsigned int SOURCE[49]  = {0,1,0,0,0,1,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,1,0,1,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,1,0,1,0,1,0};
	unsigned int SEEK_UP[49] = {0,1,0,0,0,1,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,1,0,1,0};
	unsigned int TRK_UP[49]  = {0,1,0,0,0,1,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,1,0,1,0,0,1,0,0,0,0,0,0,0,1,0,0,1,0,1,0,1,0,1,0};
	unsigned int TRK_DN[49]  = {0,1,0,0,0,1,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,0,1,0,1,0,1,0};
	//unsigned int ANSWER[49]  = {0,1,0,0,0,1,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0};
		
    unsigned char flag_vol_dn=0, flag_vol_up=0, flag_trk_up=0, flag_trk_dn=0, flag_source=0, posicion_retrovisor=0;
	volatile  int flag_marcha_atras=0, modo=0;
	
	
	const unsigned tiempo_mov_retrovisor = 210, tiempo_calibracion=5, flag_marcha_atras_eeprom = 1, retrovisor_activado_eeprom=2, posicion_retrovisor_eeprom = 3, numero_tomas = 10,
	limite_pulsacion_unica = 15, variacion_volumen = 9, vol_max=25, vol_inicial=12, amplitud_atenuacion = 9;

	int v_entrada=0;
	
void pulsacion_unica()
{
	if (flag_vol_dn>=1 & flag_vol_dn<limite_pulsacion_unica)
	{
		 if(!modo)
		 {
			 send(VOL_DN);
			 
		 }else
		 {
			 puerto_odenador|=(1<<PB1);
			 _delay_ms(100);
			 puerto_odenador&=~(1<<PB1);
		 }
		
	}else if (flag_vol_up>=1 & flag_vol_up<limite_pulsacion_unica)
		{
			 if(!modo)
			 {
				 send(VOL_UP);
			 
			 }else
			 {
				 puerto_odenador|=(1<<PB2);
				 _delay_ms(100);
				 puerto_odenador&=~(1<<PB2);
			 }	
		
	}else if (flag_trk_up>=1)
	{
		 if(!modo)
		 {
			 send(TRK_UP);
			 
		 }else
		 {
			 puerto_odenador|=(1<<PB3);
			 _delay_ms(100);
			 puerto_odenador&=~(1<<PB3);
		 }
			
	}else if (flag_trk_dn>=1)
	{
		 if(!modo)
		 {
			 send(TRK_DN);
			 
		 }else
		 {
			 puerto_odenador|=(1<<PB4);
			 _delay_ms(100);
			 puerto_odenador&=~(1<<PB4);
		 }

	}else if (flag_source>=1)
	{
		 if(!modo)
		 {
			 send(SOURCE);
			 
		 }else
		 {
			 puerto_odenador|=(1<<PB5);
			 _delay_ms(100);
			 puerto_odenador&=~(1<<PB5);
		 }
	}
	limpiar_flags();
}

void limpiar_flags() 
{
	flag_vol_dn = 0;
	flag_vol_up = 0;
	flag_trk_up = 0;
	flag_trk_dn = 0;
	flag_source = 0;
}

 
void send(unsigned int *command)
{
	puerto &= ~(1<<salida_radio);
	_delay_ms(10);
	puerto |= (1<<salida_radio);
	_delay_us(4500);
	
	for (int x = 0; x < 49; x++)
	{
		if (command[x] == 0)
		{
			puerto &= ~(1<<salida_radio);
		}
		else
		{
			puerto |= (1<<salida_radio);
		}
		_delay_us(1000);
		puerto |= (1<<salida_radio);
		_delay_us(200);
	}

}

void bajar_volumen(int unsigned numero_niveles) 
{
	 for (int x=0; x<numero_niveles; x++)
	 {
		 send(VOL_DN);
	 }
}

void subir_volumen(int numero_niveles)
{
	for (int x = 0; x<numero_niveles; x++)
	{
		send(VOL_UP);
	}
}


void inicio_marcha_atras()
{
	flag_marcha_atras=1;
	bajar_volumen(variacion_volumen);
	flag_marcha_atras=1;
	
}

void fin_marcha_atras()
{
	subir_volumen(variacion_volumen);
	flag_marcha_atras=0;
	if (eeprom_read_byte((uint8_t*)(retrovisor_activado_eeprom))){
		subir_retrovisor();
	}
	flag_marcha_atras=0;
}



char detectar_desbordamientos(char flag)
{
	if (flag>limite_pulsacion_unica)
	{
		return limite_pulsacion_unica;
	}
}

void detectar_entrada() 
{
	reiniciar_volumen();	
	while(1)
	{	
		v_entrada=ADCH;	
		v_entrada=ADC;
	
	
		if (!bit_is_clear(PIN,entrada_marcha_atras) && !flag_marcha_atras)
		{
			inicio_marcha_atras();
		}
		
		if (bit_is_clear(PIN,entrada_marcha_atras) && flag_marcha_atras)
		{
			fin_marcha_atras();
		}
	
		if ((v_entrada > 920) & (v_entrada < 999)) 
		{

			flag_vol_dn++;		
			_delay_ms(100);
			flag_vol_dn=detectar_desbordamientos(flag_vol_dn);
		}
		else if ((v_entrada > 850) & (v_entrada < 900))
		{
			flag_vol_up++;
			_delay_ms(100);
			flag_vol_up=detectar_desbordamientos(flag_vol_up);
		}
		else if ((v_entrada > 730) & (v_entrada < 780))
		{
			flag_trk_up++;
			_delay_ms(100);
			flag_trk_up=detectar_desbordamientos(flag_trk_up);
		}
		else if ((v_entrada > 600) & (v_entrada < 660))
		{
			flag_trk_dn++;
			_delay_ms(100);
			flag_trk_dn=detectar_desbordamientos(flag_trk_dn);
		}
		else if ((v_entrada > 470) & (v_entrada < 520))
		{
			flag_source++;
			_delay_ms(100);
			flag_source=detectar_desbordamientos(flag_source);
		}
		else if (flag_vol_dn>0 | flag_vol_up>0 | flag_trk_up>0 | flag_trk_dn>0 | flag_source>0)
		  {
			  pulsacion_unica();
		  }
			  
		 if (flag_source>=limite_pulsacion_unica)
		 {
			 if (modo==0)
			 {
				 modo=1;
			 }else
			 {
				 modo=0;
			 }
			 limpiar_flags();
			 _delay_ms(600);
		 }else if (flag_vol_dn>=limite_pulsacion_unica)
		 {
			 if(!modo)
			 {
				 bajar_volumen(variacion_volumen);
				 limpiar_flags();
				 _delay_ms(500);
			 }
			 
		 }else if (flag_vol_up>=limite_pulsacion_unica)
			{
				limpiar_flags();
				if (flag_marcha_atras & !eeprom_read_byte((uint8_t*)(retrovisor_activado_eeprom))){
					bajar_retrovisor();
			    }else if (flag_marcha_atras & eeprom_read_byte((uint8_t*)(retrovisor_activado_eeprom))){
					subir_retrovisor();
			    }
			}
	}	 
}

void subir_retrovisor()
{
	puerto |= (1<<retro1);
	puerto &= ~(1<<retro2);
		
	for(posicion_retrovisor; posicion_retrovisor<=numero_tomas; posicion_retrovisor++)
	{
		eeprom_update_byte(( uint8_t *) posicion_retrovisor_eeprom, posicion_retrovisor);
		_delay_ms(tiempo_mov_retrovisor+tiempo_calibracion);
	}
	eeprom_update_byte(( uint8_t *) posicion_retrovisor_eeprom, 0);
	posicion_retrovisor=0;
	puerto &= ~(1<<retro1);
	puerto &= ~(1<<retro2);
	eeprom_update_byte(( uint8_t *) flag_marcha_atras_eeprom, 0);
	eeprom_update_byte(( uint8_t *) retrovisor_activado_eeprom, 0);
	
	
}

void bajar_retrovisor()
{
	puerto &= ~(1<<retro1);
	puerto |= (1<<retro2);
	for(posicion_retrovisor; posicion_retrovisor<=numero_tomas;posicion_retrovisor++)
	{
		eeprom_update_byte(( uint8_t *) posicion_retrovisor_eeprom, posicion_retrovisor);
		_delay_ms(tiempo_mov_retrovisor);
	}
	eeprom_update_byte(( uint8_t *) posicion_retrovisor_eeprom, 0);
	posicion_retrovisor=0;
	puerto &= ~(1<<retro1);
	puerto &= ~(1<<retro2);
	eeprom_update_byte(( uint8_t *) flag_marcha_atras_eeprom, 1);
	eeprom_update_byte(( uint8_t *) retrovisor_activado_eeprom, 1);
	

}

void reiniciar_volumen()
{
	bajar_volumen(vol_max);
	if (flag_marcha_atras){
		subir_volumen(vol_inicial-variacion_volumen);
	}else{
		subir_volumen(vol_inicial);
	}
	
}

int main(void)
{
	
	
	//Inicialicacion puertos
	DDR_puerto |= (1<<salida_radio) | (0<<entrada_marcha_atras) | (1<<retro1) | (1<<retro2);
	DDR_puerto_analog &= ~(1<<entrada_botones);
	DDR_ordenador |= (1<<PINB1) | (1<<PINB2) | (1<<PINB3) | (1<<PINB4) | (1<<PINB5);
	
	//Inicializacion conversor A/D
	ADMUX &= ~(1<<REFS0) & ~(1<<REFS1);  //Seleccionamos referencia externa
	ADMUX &= ~(1<<MUX0) & ~(1<<MUX1) & ~(1<<MUX2) & ~(1<<MUX3);  //Seleccionamos ADC0
	ADCSRA |= (1<<ADPS0) | (1<<ADPS1) | (0<<ADPS2); //Factor de 8
	ADCSRA |= (1 << ADEN); //Habilitamos el ADC
	ADCSRA |= (1 << ADFR); //Habilitamos el free running mode
	ADCSRA |= (1 << ADSC); //Iniciamos el free running mode
	
    while(1)
	{
		posicion_retrovisor = eeprom_read_byte((uint8_t*)(posicion_retrovisor_eeprom));
		flag_marcha_atras = eeprom_read_byte((uint8_t*)(flag_marcha_atras_eeprom));
		
		if (posicion_retrovisor > 0){
			if (!flag_marcha_atras & (!bit_is_clear(PIN,entrada_marcha_atras))){
				bajar_retrovisor();
			}else if(!flag_marcha_atras & bit_is_clear(PIN,entrada_marcha_atras)){
				posicion_retrovisor=numero_tomas-posicion_retrovisor;
				subir_retrovisor();
			}else if(flag_marcha_atras & bit_is_clear(PIN,entrada_marcha_atras)){
				subir_retrovisor();
			}else if(flag_marcha_atras & !bit_is_clear(PIN,entrada_marcha_atras)){
				posicion_retrovisor=numero_tomas-posicion_retrovisor;
				bajar_retrovisor();
			}
		}
		detectar_entrada();
       
    }
}
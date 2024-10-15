
void spi_lis_mems_select () {
   gpio_clear(GPIOE,GPIO3);
}

void spi_lis_mems_deselect () {
   gpio_set(GPIOE,GPIO3);
}

void init_spi1 () {
   // baud rate : APB2 freq/16 : 82Mhz/16 = 5,125MHz  (< 10MHz required by mems)
   // Clock idle high, sample on rising edge : CPOl,CPHA = (1,1)
   // 8 bits frame, MSB bit first, no crc

   spi_mems_configure_pins ();

   spi_lis_mems_deselect();
 
   spi_disable_crc(SPI1);
   spi_init_master(SPI1,SPI_CR1_BAUDRATE_FPCLK_DIV_16,
                   SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_2,
                   SPI_CR1_DFF_8BIT,SPI_CR1_MSBFIRST);
   spi_set_full_duplex_mode(SPI1);
   spi_enable(SPI1);  
}

uint8_t spi_read_command (uint8_t add) {
   uint8_t result;
   uint8_t lire_cmd = 0x80 | add;
   spi_lis_mems_select();
   
   spi_send(SPI1,lire_cmd);
   spi_read(SPI1);

   spi_send(SPI1,lire_cmd);
   result=spi_read(SPI1);

   spi_lis_mems_deselect();

   return result;
}

void spi_write_command (uint8_t add,  uint8_t data) {
   uint8_t ecrire_cmd = add & ~(0x80);

   spi_lis_mems_select();
   
   spi_send(SPI1,ecrire_cmd);
   spi_read(SPI1);
   
   spi_send(SPI1,data);
   spi_read(SPI1);

   spi_lis_mems_deselect();
}

int main (void) {
   coldstart();
   config_leds_pins();
   init_spi1();
   // 1. on verifie s'il y a bien 0x3f dans WHO_AM_I (test lecture)
   v = spi_read_command(0x0F);
   // 2. on ecrit 0x22 dans OFF_X et on relit (test ecriture)
   spi_write_command(0x10,0x22);
   v = spi_read_command(0x10);
   // 3. si tout s'est bien passe, on pourra remettre 0x00 dans
   // OFF_X et puis continuer...
   spi_write_command(0x10,0x00);

   mems_init ();
   while (1) {
      mems_wait_sample();
      mems_read_sample();
      mems_display_sample();
   }
   return 0;
}

void write_leds_int (uint16_t intval) {
   GPIOD_ODR &= ~(0xf<<12);
   GPIOD_ODR |= intval << 12;
}

// Vérifie qu'il n'y a pas d'erreur de graine (renvoie 1 si pas d'erreur, 0 sinon)
int check_no_seed_error() {
  return (RNG_SR & RNG_SR_SEIS) == 0;
}

// Vérifie qu'il n'y a pas d'erreur d'horloge (renvoie 1 si pas d'erreur, 0 sinon)
int check_no_clock_error() {
  return (RNG_SR & RNG_SR_CEIS) == 0;
}

// Vérifie si un nouveau nombre aléatoire est disponible (renvoie 1 si disponible, 0 sinon)
int check_data_ready() {
  return (RNG_SR & RNG_SR_DRDY) ? 1 : 0;
}

void init_rng() {
  // Activer l'interruption RNG dans le NVIC
  nvic_enable_irq(NVIC_RCC_IRQ);

  // Activer l'interruption au niveau du RNG
  RNG_CR |= RNG_CR_RNGEN | RNG_CR_IE;  // Activer le RNG et l'interruption
}
 

hash_rng_isr(){
    if (check_no_seed_error() && check_no_clock_error()) {
    rnd = RNG_DR;
  } else {
    RNG_SR |= (1 << 5);  
    RNG_SR |= (1 << 6);  
    RNG_CR |= (1 << 2);   
  }
}

void wait_for_new_rnd() {
  // Boucle d'attente active jusqu'à ce qu'un nouveau nombre aléatoire soit disponible
  while (!check_data_ready()) {
    // Vérifier s'il y a une erreur de graine (seed error)
    if (check_seed_error()) {
      // Réinitialiser le RNG en réactivant le bit RNGEN
      RNG_CR |= RNG_CR_RNGEN;
    }

    // Vérifier s'il y a une erreur d'horloge (clock error)
    if (check_clock_error()) {
      // Réinitialiser le RNG en réactivant le bit RNGEN
      RNG_CR |= RNG_CR_RNGEN;
    }
  }
    // À ce stade, un nouveau nombre aléatoire est disponible
}

int main (void) {	
  coldstart(); 
  config_leds_pins();
  gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LEDS_MASK);
  systick_config(RELOAD_VAl_350MS);
  
  init_rng();
  while(1){
    sys_tick_handler();
  }
  return 0;
}

int main (void) {	
  coldstart(); 
  config_leds_pins();
  gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LEDS_MASK);
  systick_config(RELOAD_VAl_350MS);
  
  init_rng_polling(); 

  while (1) {
    // Attendre un nouveau nombre aléatoire
    wait_for_new_rnd();
        
    // Lire le nombre aléatoire depuis le registre RNG_DR
    rnd = RNG_DR;
    sys_tick_handler();
  }
  return 0;
}

#include <stdbool.h>
#include <stdint.h>

// System Timer registers
#define TIMER_BASE 0x3f003000
#define TIMER_CLO (volatile uint32_t *)(TIMER_BASE + 0x04)

#define GPSET0 (volatile uint32_t *)0x3f20001c
#define GPSET1 (volatile uint32_t *)0x3f200020
#define GPCLR0 (volatile uint32_t *)0x3f200028
#define GPCLR1 (volatile uint32_t *)0x3f20002c

#define GPLEV0 (volatile uint32_t *)0x3f200034

#define gpset0(pin) *GPSET0 = 1 << pin
#define gpset1(pin) *GPSET1 = 1 << (pin - 32)
#define gpclr0(pin) *GPCLR0 = 1 << pin
#define gpclr1(pin) *GPCLR1 = 1 << (pin - 32)

#define BUTTON 26
#define SWITCH 21

#define A 24
#define B 25
#define C 27
#define D 17
#define E 13
#define F 23
#define G 18
#define setA() gpclr0(A)
#define setB() gpclr0(B)
#define setC() gpclr0(C)
#define setD() gpclr0(D)
#define setE() gpclr0(E)
#define setF() gpclr0(F)
#define setG() gpclr0(G)
#define clrA() gpset0(A)
#define clrB() gpset0(B)
#define clrC() gpset0(C)
#define clrD() gpset0(D)
#define clrE() gpset0(E)
#define clrF() gpset0(F)
#define clrG() gpset0(G)

static bool counting_up = true;
static bool switch_on = true;

static void setDigit(char);
static void setDisplay(bool a, bool b, bool c, bool d, bool e, bool f, bool g);
static void delay_ms(uint32_t milliseconds);
static void check_button(void);
static void check_switch(void);

static void delay_ms(uint32_t milliseconds) {
  uint32_t start_time = *TIMER_CLO;
  uint32_t delay_time = milliseconds * 1000; // convert to microseconds

  while ((*TIMER_CLO - start_time) < delay_time) {
    check_button();
    check_switch();
  }
}

static void check_button(void) {
  static bool last_button_state = true;
  static uint32_t last_change_time = 0;
  const uint32_t DEBOUNCE_TIME = 50000;

  bool current_state = (*GPLEV0 & (1 << BUTTON)) != 0;

  if (current_state != last_button_state) {
    uint32_t current_time = *TIMER_CLO;
    // debouncing
    if ((current_time - last_change_time) > DEBOUNCE_TIME) {
      counting_up = (current_state) ? !counting_up : counting_up;
      last_button_state = current_state;
      last_change_time = current_time;
    }
  }
}

static void check_switch(void) {
  static bool last_switch_state = true;
  static uint32_t last_change_time = 0;
  const uint32_t DEBOUNCE_TIME = 50000;

  bool current_state = (*GPLEV0 & (1 << SWITCH)) != 0;

  if (current_state != last_switch_state) {
    uint32_t current_time = *TIMER_CLO;
    if ((current_time - last_change_time) > DEBOUNCE_TIME) {
      switch_on = (current_state) ? !switch_on : switch_on;
      last_switch_state = current_state;
      last_change_time = current_time;
    }
  }
}

int main(void) {
  int current_digit = 0;
  for (;;) {
    if (switch_on) {
      setDigit(current_digit + '0');
      delay_ms(1000);

      if (!switch_on) {
        continue;
      }

      if (counting_up) {
        current_digit = (current_digit + 1) % 10;
      } else {
        current_digit = (current_digit - 1 + 10) % 10;
      }
    } else {
      setDisplay(false, false, false, false, false, false, false);
      delay_ms(1000);
    }
  }
  return 0;
}

void setDigit(char Digit) {
  switch (Digit) {
  case '0':
    setDisplay(true, true, true, true, true, true, false);
    break;
  case '1':
    setDisplay(false, true, true, false, false, false, false);
    break;
  case '2':
    setDisplay(true, true, false, true, true, false, true);
    break;
  case '3':
    setDisplay(true, true, true, true, false, false, true);
    break;
  case '4':
    setDisplay(false, true, true, false, false, true, true);
    break;
  case '5':
    setDisplay(true, false, true, true, false, true, true);
    break;
  case '6':
    setDisplay(true, false, true, true, true, true, true);
    break;
  case '7':
    setDisplay(true, true, true, false, false, false, false);
    break;
  case '8':
    setDisplay(true, true, true, true, true, true, true);
    break;
  case '9':
    setDisplay(true, true, true, true, false, true, true);
    break;
  default:
    setDisplay(false, false, false, false, false, false, false);
  }
}

void setDisplay(bool a, bool b, bool c, bool d, bool e, bool f, bool g) {
  if (a)
    setA();
  else
    clrA();
  if (b)
    setB();
  else
    clrB();
  if (c)
    setC();
  else
    clrC();
  if (d)
    setD();
  else
    clrD();
  if (e)
    setE();
  else
    clrE();
  if (f)
    setF();
  else
    clrF();
  if (g)
    setG();
  else
    clrG();
}

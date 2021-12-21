#include <stm32h7xx.h>

// PE9  / TIM1_CH1  / phase U high-side
// PE8  / TIM1_CH1N / phase U low-side
// PE11 / TIM1_CH2  / phase V high-side
// PE10 / TIM1_CH2N / phase V low-side
// PE13 / TIM1_CH3  / phase W high-side
// PE12 / TIM1_CH3N / phase W low-side

// to drive channel high (pwm high side, !pwm low side) set OCxM to 0b0110 (pwm mode 1), CCxP to 0, CCxNP to 0
// to drive channel low (off high side, on low side) set OCxM to 0b0100 (forced low), CCxP to 0, CCxNP to 0
// to float channel (off high side, off low side), set OCxM to 0b0100 (forced low), CCxP to 0, CCxNP to 1

#define CCMR1_PHASE_U_HIGH TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1
#define CCMR1_PHASE_V_HIGH TIM_CCMR1_OC2PE | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1
#define CCMR1_PHASE_W_HIGH 0

#define CCMR1_PHASE_U_LOW TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M_2
#define CCMR1_PHASE_V_LOW TIM_CCMR1_OC2PE | TIM_CCMR1_OC2M_2
#define CCMR1_PHASE_W_LOW 0

#define CCMR1_PHASE_U_FLOAT TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M_2
#define CCMR1_PHASE_V_FLOAT TIM_CCMR1_OC2PE | TIM_CCMR1_OC2M_2
#define CCMR1_PHASE_W_FLOAT 0

#define CCMR2_PHASE_U_HIGH 0
#define CCMR2_PHASE_V_HIGH 0
#define CCMR2_PHASE_W_HIGH TIM_CCMR2_OC3PE | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1

#define CCMR2_PHASE_U_LOW 0
#define CCMR2_PHASE_V_LOW 0
#define CCMR2_PHASE_W_LOW TIM_CCMR2_OC3PE | TIM_CCMR2_OC3M_2

#define CCMR2_PHASE_U_FLOAT 0
#define CCMR2_PHASE_V_FLOAT 0
#define CCMR2_PHASE_W_FLOAT TIM_CCMR2_OC3PE | TIM_CCMR2_OC3M_2

#define CCER_PHASE_U_HIGH TIM_CCER_CC1E | TIM_CCER_CC1NE
#define CCER_PHASE_V_HIGH TIM_CCER_CC2E | TIM_CCER_CC2NE
#define CCER_PHASE_W_HIGH TIM_CCER_CC3E | TIM_CCER_CC3NE

#define CCER_PHASE_U_LOW TIM_CCER_CC1E | TIM_CCER_CC1NE
#define CCER_PHASE_V_LOW TIM_CCER_CC2E | TIM_CCER_CC2NE
#define CCER_PHASE_W_LOW TIM_CCER_CC3E | TIM_CCER_CC3NE

#define CCER_PHASE_U_FLOAT TIM_CCER_CC1E | TIM_CCER_CC1NE | TIM_CCER_CC1NP
#define CCER_PHASE_V_FLOAT TIM_CCER_CC2E | TIM_CCER_CC2NE | TIM_CCER_CC2NP
#define CCER_PHASE_W_FLOAT TIM_CCER_CC3E | TIM_CCER_CC3NE | TIM_CCER_CC3NP

#define _SET(REG, U, V, W) TIM1->REG = REG##_PHASE_U_##U | REG##_PHASE_V_##V | REG##_PHASE_W_##W
#define SET(U, V, W) _SET(CCMR1, U, V, W); _SET(CCMR2, U, V, W); _SET(CCER, U, V, W); TIM1->EGR = TIM_EGR_COMG;

static int step = 0;

static void stop() {
	SET(FLOAT, FLOAT, FLOAT);
}

static void step0() {
	step = 0;

	// phaseU low
	// phaseV high
	// phaseW floating
	SET(LOW, HIGH, FLOAT);
}

static void step1() {
	step = 1;

	// phaseU floating
	// phaseV high
	// phaseW low
	SET(FLOAT, HIGH, LOW);
}

static void step2() {
	step = 2;

	// phaseU high
	// phaseV floating
	// phaseW low
	SET(HIGH, FLOAT, LOW);
}

static void step3() {
	step = 3;

	// phaseU high
	// phaseV low
	// phaseW floating
	SET(HIGH, LOW, FLOAT);
}

static void step4() {
	step = 4;

	// phaseU floating
	// phaseV low
	// phaseW high
	SET(FLOAT, LOW, HIGH);
}

static void step5() {
	step = 5;

	// phaseU low
	// phaseV floating
	// phaseW high
	SET(LOW, FLOAT, HIGH);
}

static void set_duty_cycle(uint16_t val) {
	TIM1->CCR1 = val;
	TIM1->CCR2 = val;
	TIM1->CCR3 = val;
}

static void step_forward() {
	switch (step) {
		case 0: step1(); break;
		case 1: step2(); break;
		case 2: step3(); break;
		case 3: step4(); break;
		case 4: step5(); break;
		case 5: step0(); break;
	}
}

static void setup() {
	RCC->AHB4ENR = RCC_AHB4ENR_GPIOEEN;
	RCC->APB2ENR = RCC_APB2ENR_TIM1EN;

	GPIOE->MODER = ~(GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0 | GPIO_MODER_MODE10_0 | GPIO_MODER_MODE11_0 | GPIO_MODER_MODE12_0 | GPIO_MODER_MODE13_0);
	GPIOE->AFR[1] = GPIO_AFRH_AFSEL8_0 | GPIO_AFRH_AFSEL9_0 | GPIO_AFRH_AFSEL10_0 | GPIO_AFRH_AFSEL11_0 | GPIO_AFRH_AFSEL12_0 | GPIO_AFRH_AFSEL13_0;

	TIM1->BDTR = TIM_BDTR_MOE | TIM_BDTR_OSSR;
	TIM1->CR1 = TIM_CR1_CEN;
	TIM1->CR2 = TIM_CR2_CCPC;

	stop();
}

int main() {
	setup();
	set_duty_cycle(50000);

	// Setup LEDs and button for testing
	RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN | RCC_AHB4ENR_GPIOCEN;
	MODIFY_REG(GPIOC->MODER, GPIO_MODER_MODE13_Msk, 0);
	MODIFY_REG(GPIOC->PUPDR, GPIO_PUPDR_PUPD13_Msk, GPIO_PUPDR_PUPD13_1);
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE14_Msk, GPIO_MODER_MODE0_0 | GPIO_MODER_MODE14_0);
	MODIFY_REG(GPIOE->MODER, GPIO_MODER_MODE1_Msk, GPIO_MODER_MODE1_0);

	while (1) {
		for (volatile int i = 0; i < 100000; i ++);
		while ((GPIOC->IDR & GPIO_IDR_ID13));
		for (volatile int i = 0; i < 100000; i ++);
		while (!(GPIOC->IDR & GPIO_IDR_ID13));

		step_forward();

		if (step & 1) {
			GPIOB->BSRR = GPIO_BSRR_BS14;
		} else {
			GPIOB->BSRR = GPIO_BSRR_BR14;
		}

		if (step & 2) {
			GPIOE->BSRR = GPIO_BSRR_BS1;
		} else {
			GPIOE->BSRR = GPIO_BSRR_BR1;
		}

		if (step & 4) {
			GPIOB->BSRR = GPIO_BSRR_BS0;
		} else {
			GPIOB->BSRR = GPIO_BSRR_BR0;
		}
	}
}


/******************* Seting Alarm **************************/
#define RTC_IER_TAIE_MASK       0x4u
#define RTC_SR_TAF_MASK         0x4u

void rtcSetup(void)
{
   SIM_SCGC6 |= SIM_SCGC6_RTC;// enable RTC clock
   RTC_CR |= RTC_CR_OSCE;// enable RTC
}

void rtcSetAlarm(uint32_t nsec)
{
   RTC_TAR = RTC_TSR + nsec;
   RTC_IER |= RTC_IER_TAIE_MASK;
}

/********************LLWU**********************************/
#define LLWU_ME_WUME5_MASK       0x20u
#define LLWU_F3_MWUF5_MASK       0x20u

static void llwuISR(void)
{
    //
    LLWU_F3 |= LLWU_F3_MWUF5_MASK; // clear source in LLWU Flag register
    //
    RTC_IER = 0;// clear RTC interrupts
}

void llwuSetup(void)
{
	attachInterruptVector( IRQ_LLWU, llwuISR );
	NVIC_SET_PRIORITY( IRQ_LLWU, 2*16 );
//
	NVIC_CLEAR_PENDING( IRQ_LLWU );
	NVIC_ENABLE_IRQ( IRQ_LLWU );
//
	LLWU_PE1 = 0;
	LLWU_PE2 = 0;
	LLWU_PE3 = 0;
	LLWU_PE4 = 0;
	LLWU_ME  = LLWU_ME_WUME5_MASK; //rtc alarm
//   
    SIM_SOPT1CFG |= SIM_SOPT1CFG_USSWE;
    SIM_SOPT1 |= SIM_SOPT1_USBSSTBY;
//
    PORTA_PCR0 = PORT_PCR_MUX(0);
    PORTA_PCR1 = PORT_PCR_MUX(0);
    PORTA_PCR2 = PORT_PCR_MUX(0);
    PORTA_PCR3 = PORT_PCR_MUX(0);

    PORTB_PCR2 = PORT_PCR_MUX(0);
    PORTB_PCR3 = PORT_PCR_MUX(0);
}
/********************* go to deep sleep *********************/
#define SMC_PMPROT_AVLLS_MASK   0x2u
#define SMC_PMCTRL_STOPM_MASK   0x7u
#define SCB_SCR_SLEEPDEEP_MASK  0x4u

#define VLLS3 0x3
#define VLLS2 0x2
#define VLLS1 0x1

void gotoSleep(void);
{  
//	/* Make sure clock monitor is off so we don't get spurious reset */
//   MCG_C6 &= ~MCG_C6_CME0;
   //
   /* Write to PMPROT to allow all possible power modes */
   SMC_PMPROT = SMC_PMPROT_AVLLS_MASK;
   /* Set the STOPM field to 0b100 for VLLSx mode */
   SMC_PMCTRL &= ~SMC_PMCTRL_STOPM_MASK;
   SMC_PMCTRL |= SMC_PMCTRL_STOPM(0x4); // VLLSx

   SMC_VLLSCTRL =  SMC_VLLSCTRL_VLLSM(VLLS1); // VLLS3
   /*wait for write to complete to SMC before stopping core */
   (void) SMC_PMCTRL;

   SYST_CSR &= ~SYST_CSR_TICKINT;      // disable systick timer interrupt
   SCB_SCR |= SCB_SCR_SLEEPDEEP_MASK;  // Set the SLEEPDEEP bit to enable deep sleep mode (STOP)
       asm volatile( "wfi" );  // WFI instruction will start entry into STOP mode
   // will never return, but call ResetHandler() in mk20dx128.c
}

void hibernate(uint32_t nsec)
{  
   rtcSetup();
   llwuSetup();
   
   rtcSetAlarm(nsec);
   gotoSleep();
}

#ifdef TEST_HIBERNATE
void flashLed(int del)
{
   pinMode(LED_BUILTIN, OUTPUT);
   digitalWrite(LED_BUILTIN, HIGH);
   delay(del);
   digitalWrite(LED_BUILTIN, LOW);
   pinMode(LED_BUILTIN, INPUT);
}

void setup() {
   //
   // put your setup code here, to run once:
   flashLed(100);
   //
   hibernate(5);   
}

void loop() {
   // put your main code here, to run repeatedly:
   //
}
#endif
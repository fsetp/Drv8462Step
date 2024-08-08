//	TB6559FG.ino
//
// https://coskxlabsite.stars.ne.jp/html/for_students/M5Stack/PWMTest/PWMTest2.html
//
#include <M5Stack.h>

//#define	EX_BUTTON_USE
#ifdef	EX_BUTTON_USE
#define	EXBT_RED_PIN	36
#define	EXBT_BLUE_PIN	26
#endif

#define	USE_CURVE

#define	DUTY_BITS		8
#define	MAX_FREQ_PWM	160000.0
#define	MIN_FREQ_PWM	10000.0
#define	STEP_FREQ_PWM	10000.0
#define	DUTY_PWM		128

#define	PWM_CH			2	
#define	PWM_PIN			2	
#define	NSLEEP_PIN		5	
#define	EN_PIN			16	
#define	DIR_PIN			17	

#define	CYCLE_WAIT_MS	5
#define	EN_WAIT_MS		100
#define	CONST_MS		400	// 400ms
#define	CONST_TIMES		(CONST_MS / CYCLE_WAIT_MS)
#define	ACCEL_MS		((MAX_FREQ_PWM  - MIN_FREQ_PWM + STEP_FREQ_PWM) / STEP_FREQ_PWM * CYCLE_WAIT_MS)

double	g_nFreqPWM		= MIN_FREQ_PWM;		//
int		g_nCycleWaitMs	= CYCLE_WAIT_MS;	//

bool	g_bDir		= false;

const double g_AccTable[] = {	10000.0,
								10300.0,
								11000.0,
								12000.0,
								13000.0,
								15000.0,
								20000.0,
								30000.0,
								40000.0,
								50000.0,
								60000.0,
								70000.0,
								80000.0,
								90000.0,
								100000.0,
								110000.0,
								120000.0,
								130000.0,
								140000.0,
								150000.0,
								155000.0,
								157000.0,
								158000.0,
								159000.0,
								159600.0,
								160000.0
};

////////////////////////////////////////
//
void Drv8462StepInitialize()
{
	g_nFreqPWM		= MIN_FREQ_PWM;		//
	g_nCycleWaitMs	= CYCLE_WAIT_MS;	//
}

#ifdef	EX_BUTTON_USE
////////////////////////////////////////
//
int g_bLastRed  = 0;
int g_bLastBlue = 0;
bool CheckExButton()
{
	int cur_red  = digitalRead(EXBT_RED_PIN);
	int cur_blue = digitalRead(EXBT_BLUE_PIN);

	if (g_bLastRed != cur_red) {
		Serial.printf("Red %d\r\n", cur_red);
	}
	g_bLastRed = cur_red;

	if (g_bLastBlue != cur_blue) {
		Serial.printf("Red %d\r\n", cur_blue);
	}
	g_bLastBlue = cur_blue;
}
#endif

////////////////////////////////////////
//
void DisplayValue()
{
	M5.Lcd.setTextSize(3);
	M5.Lcd.setCursor(10, 10);
	M5.Lcd.printf("Start %6.0f pps", g_nFreqPWM);
	M5.Lcd.setCursor(10, 40);
	M5.Lcd.printf("Top   %6.0f pps", MAX_FREQ_PWM);
	M5.Lcd.setCursor(10, 70);
	M5.Lcd.printf("Stop  %6.0f pps", g_nFreqPWM);
	M5.Lcd.setCursor(10, 100);
	M5.Lcd.printf("Step  %6.0f pps", STEP_FREQ_PWM);
	M5.Lcd.setCursor(10, 130);
	M5.Lcd.printf("Accel %6d ms", (int)ACCEL_MS);
	M5.Lcd.setCursor(10, 160);
	M5.Lcd.printf("Const %6d ms", (int)CONST_MS);
	M5.Lcd.setCursor(10, 190);
	M5.Lcd.printf("Cycle %6d ms", g_nCycleWaitMs);
}

////////////////////////////////////////
//
#define	CHK_BTA		0
#define	CHK_BTB		1
#define	CHK_BTC		2

bool CheckButton(int bt)
{
	M5.update();

	switch (bt) {
		case CHK_BTA:
			if (M5.BtnA.wasPressed())
				return true;
			break;

		case CHK_BTB:
			if (M5.BtnB.wasPressed())
				return true;
			break;

		case CHK_BTC:
			if (M5.BtnC.wasPressed())
				return true;
			break;
	}
	return false;
}

////////////////////////////////////////
//
void SetEn(bool bEnable)
{
	digitalWrite(EN_PIN, bEnable ? HIGH : LOW);
}

////////////////////////////////////////
//
void SetDir(bool bDir)
{
	digitalWrite(DIR_PIN, bDir ? HIGH : LOW);
}

////////////////////////////////////////
//
void SetFrequency(double nFreq)
{
	ledcSetup(PWM_CH, nFreq, DUTY_BITS);

	if (nFreq == 0)
		ledcWrite(PWM_CH, 0);
	else
		ledcWrite(PWM_CH, DUTY_PWM);
}

////////////////////////////////////////
//
bool SendPulseCurve(bool bDir, int chkbt)
{
	SetDir(bDir);

	SetEn(true);
	delay(EN_WAIT_MS);

	int nAccNum = sizeof (g_AccTable) / sizeof (double);

	unsigned long micro1;
	unsigned long micro2;
	unsigned long span;

	micro1 = micros();

	// acceleration
	for (int i = 0; i < nAccNum; i++) {

		Serial.printf("%.1f\r\n", g_AccTable[i]);
		SetFrequency(g_AccTable[i]);
		delay(g_nCycleWaitMs);

		if (CheckButton(chkbt)) {
			SetEn(false);
			SetFrequency(0);
			return false;
		}
	}

	micro2 = micros();
	span = micro2 - micro1;
	Serial.printf("%ld ms\r\n", span / 1000);

	micro1 = micros();

	// constant
	double nFreq = MAX_FREQ_PWM;
	SetFrequency(nFreq);
	for (int i = 0; i < CONST_TIMES; i++) {
//		Serial.printf("%.1f\r\n", nFreq);
		delay(g_nCycleWaitMs);

		if (CheckButton(chkbt)) {
			SetEn(false);
			SetFrequency(0);
			return false;
		}
	}

	micro2 = micros();
	span = micro2 - micro1;
	Serial.printf("%ld ms\r\n", span / 1000);

	micro1 = micros();

	// deacceleration
	for (int i = 0; i < nAccNum; i++) {

		Serial.printf("%.1f\r\n", g_AccTable[nAccNum - 1 - i]);
		SetFrequency(g_AccTable[nAccNum - 1 - i]);
		delay(g_nCycleWaitMs);

		if (CheckButton(chkbt)) {
			SetEn(false);
			SetFrequency(0);
			return false;
		}
	}

	micro2 = micros();
	span = micro2 - micro1;
	Serial.printf("%ld ms\r\n", span / 1000);

	SetFrequency(0);

	delay(EN_WAIT_MS);
	SetEn(false);

	return true;
}

////////////////////////////////////////
//
bool SendPulseTrapezoid(bool bDir, int chkbt)
{
	SetDir(bDir);

	SetEn(true);
	delay(EN_WAIT_MS);

	unsigned long micro1;
	unsigned long micro2;
	unsigned long span;

	micro1 = micros();

	// acceleration
	double nFreq = g_nFreqPWM;
	while (nFreq <= MAX_FREQ_PWM) {
//		Serial.printf("%.1f\r\n", nFreq);
		SetFrequency(nFreq);
		delay(g_nCycleWaitMs);
		nFreq += STEP_FREQ_PWM;

		if (CheckButton(chkbt)) {
			SetEn(false);
			SetFrequency(0);
			return false;
		}
	}

	micro2 = micros();
	span = micro2 - micro1;
	Serial.printf("%ld ms\r\n", span / 1000);

	micro1 = micros();

	// constant
	nFreq = MAX_FREQ_PWM;
	SetFrequency(nFreq);
	for (int i = 0; i < CONST_TIMES; i++) {
//		Serial.printf("%.1f\r\n", nFreq);
		delay(g_nCycleWaitMs);

		if (CheckButton(chkbt)) {
			SetEn(false);
			SetFrequency(0);
			return false;
		}
	}

	micro2 = micros();
	span = micro2 - micro1;
	Serial.printf("%ld ms\r\n", span / 1000);

	micro1 = micros();

	// deacceleration
	nFreq = MAX_FREQ_PWM;
	while (nFreq >= g_nFreqPWM) {
//		Serial.printf("%.1f\r\n", nFreq);
		SetFrequency(nFreq);
		delay(g_nCycleWaitMs);
		nFreq -= STEP_FREQ_PWM;

		if (CheckButton(chkbt)) {
			SetEn(false);
			SetFrequency(0);
			return false;
		}
	}

	micro2 = micros();
	span = micro2 - micro1;
	Serial.printf("%ld ms\r\n", span / 1000);

	SetFrequency(0);

	delay(EN_WAIT_MS);
	SetEn(false);

	return true;
}

////////////////////////////////////////
//
void setup()
{
	M5.begin();
//	delay(10);
	M5.Power.begin();
//	delay(10);
	Serial.begin(115200);
//	delay(10);

	DisplayValue();

	pinMode(PWM_PIN,	OUTPUT); 
	pinMode(NSLEEP_PIN,	OUTPUT); 
	pinMode(EN_PIN,		OUTPUT); 
	pinMode(DIR_PIN,	OUTPUT); 

#ifdef	EX_BUTTON_USE
	pinMode(EXBT_RED_PIN, INPUT);
	pinMode(EXBT_BLUE_PIN, INPUT);
	g_bLastRed  = digitalRead(EXBT_RED_PIN);
	g_bLastBlue = digitalRead(EXBT_BLUE_PIN);

#endif

	digitalWrite(NSLEEP_PIN, HIGH);
	SetEn(false);
	SetDir(false);

	ledcAttachPin(PWM_PIN, PWM_CH);
	ledcWrite(PWM_CH, DUTY_PWM);

	SetEn(false);
	SetFrequency(0);
}

////////////////////////////////////////
//
void loop()
{
	int nLastFreqPWM = g_nFreqPWM;

#ifdef	EX_BUTTON_USE
	CheckExButton();
#endif

	M5.update();

	if (M5.BtnA.wasPressed()) {
#ifdef	USE_CURVE
		SendPulseCurve(false, CHK_BTA);
#else
		SendPulseTrapezoid(false, CHK_BTA);
#endif
	}

	if (M5.BtnB.wasPressed()) {
		bool bCont = true;
		for (int i = 0; i < 5; i++) {
#ifdef	USE_CURVE
			if (!SendPulseCurve(false, CHK_BTB))
				break;
#else
			if (!SendPulseTrapezoid(false, CHK_BTB))
				break;
#endif

			for (int j = 0; j < 5; j++) {
				if (CheckButton(CHK_BTB)) {
					bCont = false;
					break;
				}
				delay(100);
			}
			if (bCont == false)
				break;

#ifdef	USE_CURVE
			if (!SendPulseCurve(true, CHK_BTB))
				break;
#else
			if (!SendPulseTrapezoid(true, CHK_BTB))
				break;
#endif
			for (int j = 0; j < 5; j++) {
				if (CheckButton(CHK_BTB)) {
					bCont = false;
					break;
				}
				delay(100);
			}
			if (bCont == false)
				break;
		}
	}

	if (M5.BtnC.wasPressed()) {
#ifdef	USE_CURVE
		SendPulseCurve(true, CHK_BTC);
#else
		SendPulseTrapezoid(true, CHK_BTC);
#endif
	}

	if (nLastFreqPWM != g_nFreqPWM)
		DisplayValue();
}

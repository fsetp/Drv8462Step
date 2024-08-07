//	TB6559FG.ino
//
// https://coskxlabsite.stars.ne.jp/html/for_students/M5Stack/PWMTest/PWMTest2.html
//
#include <M5Stack.h>

#define	DUTY_BITS		8
#define	MAX_FREQ_PWM	160000.0
#define	MIN_FREQ_PWM	10000.0
#define	STEP_FREQ_PWM	10000.0
#define	DUTY_PWM		128		// デューティ n / 255
										// PWM周波数の最大値 Maxfreq = 80000000.0/2^n[Hz] = 312500[Hz]
#define	PWM_CH			2	// PWMチャンネル
#define	PWM_PIN			2	// PWM出力に使用するGPIO PIN番号
#define	NSLEEP_PIN		5	
#define	EN_PIN			16	
#define	DIR_PIN			17	

#define	CYCLE_WAIT_MS	5
#define	EN_WAIT_MS		100
//#define	CONST_TIMES		160
#define	CONST_TIMES		80

double	g_nFreqPWM	= MIN_FREQ_PWM;	//

bool	g_bDir		= false;
////////////////////////////////////////
//
void DisplayValue()
{
	M5.Lcd.setCursor(10, 0);
	M5.Lcd.setTextSize(3);
//	M5.Lcd.setTextFont(3);
	M5.Lcd.print("Start Frequency");
	M5.Lcd.setCursor(10, 40);
	M5.Lcd.printf("  %.1f	", g_nFreqPWM);
	M5.Lcd.setCursor(10, 80);
	M5.Lcd.print("Top Frequency");
	M5.Lcd.setCursor(10, 120);
	M5.Lcd.printf("  %.1f	", MAX_FREQ_PWM);
	M5.Lcd.setCursor(10, 160);
	M5.Lcd.print("Step Frequency");
	M5.Lcd.setCursor(10, 200);
	M5.Lcd.printf("  %.1f	", STEP_FREQ_PWM);
	M5.Lcd.setCursor(10, 240);
	M5.Lcd.print("Const Ms");
	M5.Lcd.setCursor(10, 300);
	M5.Lcd.printf("  %.1f	", CONST_TIMES * CYCLE_WAIT_MS);
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
	// チャンネルと周波数の分解能を設定
	ledcSetup(PWM_CH, nFreq, DUTY_BITS);

	if (nFreq == 0)
		ledcWrite(PWM_CH, 0);
	else
		ledcWrite(PWM_CH, DUTY_PWM);
}

////////////////////////////////////////
//
void SendPulseCurve(bool bDir)
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
		delay(CYCLE_WAIT_MS);
		nFreq += STEP_FREQ_PWM;
	}

	micro2 = micros();
	span = micro2 - micro1;
	Serial.printf("%ld ms\r\n", span / 1000);

	micro1 = micros();

	// constant
	nFreq = MAX_FREQ_PWM;
	for (int i = 0; i < CONST_TIMES; i++) {
//		Serial.printf("%.1f\r\n", nFreq);
		delay(CYCLE_WAIT_MS);
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
		delay(CYCLE_WAIT_MS);
		nFreq -= STEP_FREQ_PWM;
	}

	micro2 = micros();
	span = micro2 - micro1;
	Serial.printf("%ld ms\r\n", span / 1000);

	SetFrequency(0);

	delay(EN_WAIT_MS);
	SetEn(false);
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

	digitalWrite(NSLEEP_PIN, HIGH);
	SetEn(false);
	SetDir(false);

	// PWM出力ピンとチャンネルの設定
	ledcAttachPin(PWM_PIN, PWM_CH);

	// デューティーの設定と出力開始
	ledcWrite(PWM_CH, DUTY_PWM);

	SetEn(false);
	SetFrequency(0);
}

////////////////////////////////////////
//
void loop()
{
	int nLastFreqPWM = g_nFreqPWM;

	//static int ValueIndex = 0;
	M5.update();

	if (M5.BtnA.wasPressed()) {
#if 0
		g_nFreqPWM += 10000.0;

		if (g_nFreqPWM > MAX_FREQ_PWM)
			g_nFreqPWM = MAX_FREQ_PWM;

		Serial.printf("%.1f\r\n", g_nFreqPWM);
#else
		SendPulseCurve(false);
#endif
	}

	if (M5.BtnB.wasPressed()) {
#if	0
		SendPulseCurve(false);
#else
		for (int i = 0; i < 5; i++) {
			SendPulseCurve(false);
			delay(500);
			SendPulseCurve(true);
			delay(500);
		}
#endif
	}

	if (M5.BtnC.wasPressed()) {
#if	0
		g_nFreqPWM -= 10000.0;

		if (g_nFreqPWM < MIN_FREQ_PWM)
			g_nFreqPWM = MIN_FREQ_PWM;

		Serial.printf("%.1f\r\n", g_nFreqPWM);
#else
		SendPulseCurve(true);
#endif
	}

	if (nLastFreqPWM != g_nFreqPWM)
		DisplayValue();
}

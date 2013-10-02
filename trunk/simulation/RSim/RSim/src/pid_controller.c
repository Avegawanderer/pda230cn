/*
 * pid_controller.c
 *
 * Created: 17.09.2013 12:45:24
 *  Author: Avega
 */ 

#include "compilers.h"
#include "math.h"
#include "pid_controller.h"
#include "fir_filter.h"

#ifdef PID_INCREMENTAL 

 
int16_t dbg_PID_p_term;
int32_t dbg_PID_d_term;
int32_t dbg_PID_i_term;
int16_t dbg_PID_output;


#define Kp 8
#define P_scale	2000

#define Ki 100
#define I_scale 1

#define Kd 10
#define D_scale 2000

#define OUTPUT_SCALE 20000
// Output limits
#define PID_OUTPUT_MIN 0
#define PID_OUTPUT_MAX 100

int32_t U;
int32_t E_prev;
int32_t E_prev2;


void initPID(uint16_t processValue)
{
	U = 0;
	E_prev = 0;
	E_prev2 = 0;
}						
 
uint8_t processPID(uint16_t setPoint, uint16_t processValue)
{
	uint8_t temp = 0;
	int32_t E_current;
	int32_t U_tmp = U;
	int32_t PID_output;

	E_current = (int32_t)setPoint - (int32_t)processValue;

	// Incremental PID controller form
	U_tmp = U_tmp + P_scale * Kp * (E_current - E_prev);	// Proportional term

	if (abs(E_current) < 250)
		U_tmp = U_tmp + I_scale * Ki * E_current;				// Integral term

	U_tmp = U_tmp + D_scale * Kd * (E_current - 2 * E_prev + E_prev2);	// Differential term
	E_prev2 = E_prev;
	E_prev = E_current;
	
	// Saturation
	if (U_tmp > INT32_MAX)
		U = INT32_MAX;
	else if (U_tmp < INT32_MIN)
		U = INT32_MIN;
	else
		U = (int32_t)U_tmp; 

	// Scale output
	PID_output = U / OUTPUT_SCALE;

	// Saturate output
	if (PID_output < PID_OUTPUT_MIN)
		temp = PID_OUTPUT_MIN;
	else if (PID_output > PID_OUTPUT_MAX)
		temp = PID_OUTPUT_MAX;
	else
		temp = PID_output;


	//------- Debug --------//
	//dbg_PID_p_term = p_term;
	dbg_PID_d_term = E_current;
	dbg_PID_i_term = U_tmp;//U;
	dbg_PID_output = temp;
	
	
	return temp;
	
}



#endif

/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////

#ifdef PID_DIRECT
 
int16_t dbg_PID_p_term;
int16_t dbg_PID_d_term;
int16_t dbg_PID_i_term;
int16_t dbg_PID_output;

static filter8bit_core_t dterm_filter_core;	// PID d-term filter core
static int16_t pid_dterm_buffer[4];			// PID d-term buffer	
static int8_t dterm_coeffs[] = {64,66,64,59};
//static int8_t dterm_coeffs[] = {60,64,67,69,67,64,60,57};

static uint16_t lastProcessValue;
static int32_t integAcc;


void initPID(uint16_t processValue)
{
	uint8_t i;
	dterm_filter_core.n = 4;
	dterm_filter_core.dc_gain = 25;
	//dterm_filter_core.n = 8;
	//dterm_filter_core.dc_gain = 51;
	dterm_filter_core.coeffs = dterm_coeffs;

	for (i=0;i<4;i++)
	{
		pid_dterm_buffer[i] = 0;
	}
	lastProcessValue = processValue;
	integAcc = 0;
}						
 
uint8_t processPID(uint16_t setPoint, uint16_t processValue)
{
	int16_t error, p_term, i_term, d_term, temp;
	
	static int16_t error_p;

	// Get the error
	error = setPoint - processValue;
	
	//------ Calculate P term --------//
	if (error > (PROP_MAX / Kp))			// Compare before multiplication to avoid overflow
	{
		p_term = PROP_MAX;	
	}
	else if (error < (PROP_MIN / Kp))
	{
		p_term = PROP_MIN;	
	}
	else
	{
		p_term = error * Kp;
	}
	
	//------ Calculate I term --------//
	#ifdef INTEGRATOR_RANGE_LIMIT
	if ((error >= -INTEGRATOR_ENABLE_RANGE) &&	(error <= INTEGRATOR_ENABLE_RANGE))
		integAcc += error * Ki;	
	else
		integAcc = 0;	
	#else
	integAcc += error * Ki;	
	#endif

	if (integAcc > INTEGRATOR_MAX )
	{
		integAcc = INTEGRATOR_MAX;
	}
	else if (integAcc < INTEGRATOR_MIN)
	{
		integAcc = INTEGRATOR_MIN;
	}
	i_term = (int16_t)(integAcc / INTEGRATOR_SCALE);	// Sould not exceed MAXINT16

	//------ Calculate D term --------//
	//d_term = fir_i16_i8((lastProcessValue - processValue), pid_dterm_buffer, &dterm_filter_core);
	d_term = lastProcessValue - processValue;
	d_term = Kd * d_term;

	lastProcessValue = processValue;
	
	//--------- Summ terms -----------//
	temp = (int16_t)( ((int32_t)p_term + (int32_t)i_term + (int32_t)d_term) / SCALING_FACTOR );
	
	if (temp > PID_OUTPUT_MAX)
	{
		temp = PID_OUTPUT_MAX;
	}
	else if (temp < PID_OUTPUT_MIN)
	{
		temp = PID_OUTPUT_MIN;
	}
	
	error_p = error;
	
	//------- Debug --------//
	dbg_PID_p_term = p_term;
	dbg_PID_d_term = d_term;
	dbg_PID_i_term = i_term;
	dbg_PID_output = temp;
	
	
	return (uint8_t)temp;
	
}

#endif



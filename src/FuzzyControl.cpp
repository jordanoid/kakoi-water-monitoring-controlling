#include "FuzzyControl.hpp"
#include <Arduino.h>

// Number of inputs to the fuzzy inference system
const int fis_gcI = 2;
// Number of outputs to the fuzzy inference system
const int fis_gcO = 4;
// Number of rules to the fuzzy inference system
const int fis_gcR = 9;

#define FIS_TYPE float
#define FIS_RESOLUSION 101
#define FIS_MIN -3.4028235E+38
#define FIS_MAX 3.4028235E+38
typedef FIS_TYPE (*_FIS_MF)(FIS_TYPE, FIS_TYPE *);
typedef FIS_TYPE (*_FIS_ARR_OP)(FIS_TYPE, FIS_TYPE);
typedef FIS_TYPE (*_FIS_ARR)(FIS_TYPE *, int, _FIS_ARR_OP);

FIS_TYPE g_fisInput[fis_gcI];
FIS_TYPE g_fisOutput[fis_gcO];

int relay[4] = {17, 23, 18, 19};

//***********************************************************************
// Support functions for Fuzzy Inference System                          
//***********************************************************************
// Trapezoidal Member Function
FIS_TYPE fis_trapmf(FIS_TYPE x, FIS_TYPE* p)
{
    FIS_TYPE a = p[0], b = p[1], c = p[2], d = p[3];
    FIS_TYPE t1 = ((x <= c) ? 1 : ((d < x) ? 0 : ((c != d) ? ((d - x) / (d - c)) : 0)));
    FIS_TYPE t2 = ((b <= x) ? 1 : ((x < a) ? 0 : ((a != b) ? ((x - a) / (b - a)) : 0)));
    return (FIS_TYPE) min(t1, t2);
}

FIS_TYPE fis_min(FIS_TYPE a, FIS_TYPE b)
{
    return min(a, b);
}

FIS_TYPE fis_probor(FIS_TYPE a, FIS_TYPE b)
{
    return (a + b - (a * b));
}

FIS_TYPE fis_prod(FIS_TYPE a, FIS_TYPE b)
{
    return (a * b);
}

FIS_TYPE fis_sum(FIS_TYPE a, FIS_TYPE b)
{
    return (a + b);
}

FIS_TYPE fis_array_operation(FIS_TYPE *array, int size, _FIS_ARR_OP pfnOp)
{
    int i;
    FIS_TYPE ret = 0;

    if (size == 0) return ret;
    if (size == 1) return array[0];

    ret = array[0];
    for (i = 1; i < size; i++)
    {
        ret = (*pfnOp)(ret, array[i]);
    }

    return ret;
}


//***********************************************************************
// Data for Fuzzy Inference System                                       
//***********************************************************************
// Pointers to the implementations of member functions
_FIS_MF fis_gMF[] =
{
    fis_trapmf
};

// Count of member function for each Input
int fis_gIMFCount[] = { 3, 3 };

// Count of member function for each Output 
int fis_gOMFCount[] = { 2, 2, 2, 2 };

// Coefficients for the Input Member Functions
FIS_TYPE fis_gMFI0Coeff1[] = { 0, 0, 20, 22 };
FIS_TYPE fis_gMFI0Coeff2[] = { 20, 22, 28, 30 };
FIS_TYPE fis_gMFI0Coeff3[] = { 28, 30, 100, 100 };
FIS_TYPE* fis_gMFI0Coeff[] = { fis_gMFI0Coeff1, fis_gMFI0Coeff2, fis_gMFI0Coeff3 };
FIS_TYPE fis_gMFI1Coeff1[] = { 0, 0, 6, 6.5 };
FIS_TYPE fis_gMFI1Coeff2[] = { 6, 6.5, 7.5, 8 };
FIS_TYPE fis_gMFI1Coeff3[] = { 7.5, 8, 14, 14 };
FIS_TYPE* fis_gMFI1Coeff[] = { fis_gMFI1Coeff1, fis_gMFI1Coeff2, fis_gMFI1Coeff3 };
FIS_TYPE** fis_gMFICoeff[] = { fis_gMFI0Coeff, fis_gMFI1Coeff };

// Coefficients for the Output Member Functions
FIS_TYPE fis_gMFO0Coeff1[] = { 0, 0, 0 };
FIS_TYPE fis_gMFO0Coeff2[] = { 0, 0, 1 };
FIS_TYPE* fis_gMFO0Coeff[] = { fis_gMFO0Coeff1, fis_gMFO0Coeff2 };
FIS_TYPE fis_gMFO1Coeff1[] = { 0, 0, 0 };
FIS_TYPE fis_gMFO1Coeff2[] = { 0, 0, 1 };
FIS_TYPE* fis_gMFO1Coeff[] = { fis_gMFO1Coeff1, fis_gMFO1Coeff2 };
FIS_TYPE fis_gMFO2Coeff1[] = { 0, 0, 0 };
FIS_TYPE fis_gMFO2Coeff2[] = { 0, 0, 1 };
FIS_TYPE* fis_gMFO2Coeff[] = { fis_gMFO2Coeff1, fis_gMFO2Coeff2 };
FIS_TYPE fis_gMFO3Coeff1[] = { 0, 0, 0 };
FIS_TYPE fis_gMFO3Coeff2[] = { 0, 0, 1 };
FIS_TYPE* fis_gMFO3Coeff[] = { fis_gMFO3Coeff1, fis_gMFO3Coeff2 };
FIS_TYPE** fis_gMFOCoeff[] = { fis_gMFO0Coeff, fis_gMFO1Coeff, fis_gMFO2Coeff, fis_gMFO3Coeff };

// Input membership function set
int fis_gMFI0[] = { 0, 0, 0 };
int fis_gMFI1[] = { 0, 0, 0 };
int* fis_gMFI[] = { fis_gMFI0, fis_gMFI1};

// Output membership function set

int* fis_gMFO[] = {};

// Rule Weights
FIS_TYPE fis_gRWeight[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };

// Rule Type
int fis_gRType[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };

// Rule Inputs
int fis_gRI0[] = { 1, 1 };
int fis_gRI1[] = { 1, 2 };
int fis_gRI2[] = { 1, 3 };
int fis_gRI3[] = { 2, 1 };
int fis_gRI4[] = { 2, 2 };
int fis_gRI5[] = { 2, 3 };
int fis_gRI6[] = { 3, 1 };
int fis_gRI7[] = { 3, 2 };
int fis_gRI8[] = { 3, 3 };
int* fis_gRI[] = { fis_gRI0, fis_gRI1, fis_gRI2, fis_gRI3, fis_gRI4, fis_gRI5, fis_gRI6, fis_gRI7, fis_gRI8 };

// Rule Outputs
int fis_gRO0[] = { 2, 1, 2, 1 };
int fis_gRO1[] = { 2, 1, 1, 1 };
int fis_gRO2[] = { 2, 1, 1, 2 };
int fis_gRO3[] = { 1, 1, 2, 1 };
int fis_gRO4[] = { 1, 1, 1, 1 };
int fis_gRO5[] = { 1, 1, 1, 2 };
int fis_gRO6[] = { 1, 2, 2, 1 };
int fis_gRO7[] = { 1, 2, 1, 1 };
int fis_gRO8[] = { 1, 2, 1, 2 };
int* fis_gRO[] = { fis_gRO0, fis_gRO1, fis_gRO2, fis_gRO3, fis_gRO4, fis_gRO5, fis_gRO6, fis_gRO7, fis_gRO8 };

// Input range Min
FIS_TYPE fis_gIMin[] = { 0, 0 };

// Input range Max
FIS_TYPE fis_gIMax[] = { 100, 14 };

// Output range Min
FIS_TYPE fis_gOMin[] = { 0, 0, 0, 0 };

// Output range Max
FIS_TYPE fis_gOMax[] = { 1, 1, 1, 1 };

//***********************************************************************
// Data dependent support functions for Fuzzy Inference System           
//***********************************************************************
// None for Sugeno

//***********************************************************************
// Fuzzy Inference System                                                
//***********************************************************************
void fis_evaluate()
{
    FIS_TYPE fuzzyInput0[] = { 0, 0, 0 };
    FIS_TYPE fuzzyInput1[] = { 0, 0, 0 };
    FIS_TYPE* fuzzyInput[fis_gcI] = { fuzzyInput0, fuzzyInput1, };
    FIS_TYPE fuzzyOutput0[] = { 0, 0 };
    FIS_TYPE fuzzyOutput1[] = { 0, 0 };
    FIS_TYPE fuzzyOutput2[] = { 0, 0 };
    FIS_TYPE fuzzyOutput3[] = { 0, 0 };
    FIS_TYPE* fuzzyOutput[fis_gcO] = { fuzzyOutput0, fuzzyOutput1, fuzzyOutput2, fuzzyOutput3, };
    FIS_TYPE fuzzyRules[fis_gcR] = { 0 };
    FIS_TYPE fuzzyFires[fis_gcR] = { 0 };
    FIS_TYPE* fuzzyRuleSet[] = { fuzzyRules, fuzzyFires };
    FIS_TYPE sW = 0;

    // Transforming input to fuzzy Input
    int i, j, r, o;
    for (i = 0; i < fis_gcI; ++i)
    {
        for (j = 0; j < fis_gIMFCount[i]; ++j)
        {
            fuzzyInput[i][j] =
                (fis_gMF[fis_gMFI[i][j]])(g_fisInput[i], fis_gMFICoeff[i][j]);
        }
    }

    int index = 0;
    for (r = 0; r < fis_gcR; ++r)
    {
        if (fis_gRType[r] == 1)
        {
            fuzzyFires[r] = FIS_MAX;
            for (i = 0; i < fis_gcI; ++i)
            {
                index = fis_gRI[r][i];
                if (index > 0)
                    fuzzyFires[r] = fis_min(fuzzyFires[r], fuzzyInput[i][index - 1]);
                else if (index < 0)
                    fuzzyFires[r] = fis_min(fuzzyFires[r], 1 - fuzzyInput[i][-index - 1]);
                else
                    fuzzyFires[r] = fis_min(fuzzyFires[r], 1);
            }
        }
        else
        {
            fuzzyFires[r] = 0;
            for (i = 0; i < fis_gcI; ++i)
            {
                index = fis_gRI[r][i];
                if (index > 0)
                    fuzzyFires[r] = fis_probor(fuzzyFires[r], fuzzyInput[i][index - 1]);
                else if (index < 0)
                    fuzzyFires[r] = fis_probor(fuzzyFires[r], 1 - fuzzyInput[i][-index - 1]);
                else
                    fuzzyFires[r] = fis_probor(fuzzyFires[r], 0);
            }
        }

        fuzzyFires[r] = fis_gRWeight[r] * fuzzyFires[r];
        sW += fuzzyFires[r];
    }

    if (sW == 0)
    {
        for (o = 0; o < fis_gcO; ++o)
        {
            g_fisOutput[o] = ((fis_gOMax[o] + fis_gOMin[o]) / 2);
        }
    }
    else
    {
        for (o = 0; o < fis_gcO; ++o)
        {
            FIS_TYPE sWI = 0.0;
            for (j = 0; j < fis_gOMFCount[o]; ++j)
            {
                fuzzyOutput[o][j] = fis_gMFOCoeff[o][j][fis_gcI];
                for (i = 0; i < fis_gcI; ++i)
                {
                    fuzzyOutput[o][j] += g_fisInput[i] * fis_gMFOCoeff[o][j][i];
                }
            }

            for (r = 0; r < fis_gcR; ++r)
            {
                index = fis_gRO[r][o] - 1;
                sWI += fuzzyFires[r] * fuzzyOutput[o][index];
            }

            g_fisOutput[o] = sWI / sW;
        }
    }
}

// Setup routine runs once when you press reset:
void controllerSetup()
{
    for (int i = 0; i < 4; i++)
        pinMode(relay[i], OUTPUT);
    for (int i = 0; i < 4; i++)
    {
        digitalWrite(relay[i], HIGH);
        delay(100);
    }
    delay(500);
    for (int i = 0; i < 4; i++)
    {
        digitalWrite(relay[i], LOW);
        delay(100);
    }
}

// Loop routine runs over and over again forever:
void fuzzyControl(float currentTemp, float currentPH, float minTemp, float maxTemp, bool autoTemp, float minPH, float maxPH, bool autoPH)
{
    // Read Input: suhu
    g_fisInput[0] = currentTemp;
    // Read Input: ph
    g_fisInput[1] = currentPH;

    g_fisOutput[0] = 0;
    g_fisOutput[1] = 0;
    g_fisOutput[2] = 0;
    g_fisOutput[3] = 0;

    if (minTemp != -1 && maxTemp != -1)
    {
        fis_gMFI0Coeff1[2] = minTemp;
        fis_gMFI0Coeff1[3] = minTemp + 2;

        fis_gMFI0Coeff2[0] = minTemp;
        fis_gMFI0Coeff2[1] = minTemp + 2;
        fis_gMFI0Coeff2[2] = maxTemp - 2;
        fis_gMFI0Coeff2[3] = maxTemp;

        fis_gMFI0Coeff3[0] = maxTemp - 2;
        fis_gMFI0Coeff3[1] = maxTemp;
    }
    else
    {
        fis_gMFI0Coeff1[2] = 20;
        fis_gMFI0Coeff1[3] = 22;

        fis_gMFI0Coeff2[0] = 20;
        fis_gMFI0Coeff2[1] = 22;
        fis_gMFI0Coeff2[2] = 28;
        fis_gMFI0Coeff2[3] = 30;

        fis_gMFI0Coeff3[0] = 28;
        fis_gMFI0Coeff3[1] = 30;
    }

    if (minPH != -1 && maxPH != -1)
    {
        fis_gMFI0Coeff1[0] = minPH;
        fis_gMFI0Coeff1[1] = minPH + 0.5;

        fis_gMFI0Coeff2[0] = minPH;
        fis_gMFI0Coeff2[1] = minPH + 0.5;
        fis_gMFI0Coeff2[2] = maxPH - 0.5;
        fis_gMFI0Coeff2[3] = maxPH;

        fis_gMFI0Coeff3[0] = maxPH - 0.5;
        fis_gMFI0Coeff3[1] = maxPH;
    }
    else
    {
        fis_gMFI0Coeff1[0] = 6.5;
        fis_gMFI0Coeff1[1] = 7;

        fis_gMFI0Coeff2[0] = 6.5;
        fis_gMFI0Coeff2[1] = 7;
        fis_gMFI0Coeff2[2] = 7.5;
        fis_gMFI0Coeff2[3] = 8;

        fis_gMFI0Coeff3[0] = 7.5;
        fis_gMFI0Coeff3[1] = 8;
    }

    fis_evaluate();

    if (autoTemp == 1)
    {
        if (g_fisOutput[0] >= 0.5 && g_fisOutput[1] < 0.5)
        {
            digitalWrite(relay[2], HIGH);
            digitalWrite(relay[3], LOW);
        }
        else if (g_fisOutput[1] >= 0.5 && g_fisOutput[0] < 0.5)
        {
            digitalWrite(relay[2], LOW);
            digitalWrite(relay[3], HIGH);
        }
        else
        {
            digitalWrite(relay[2], LOW);
            digitalWrite(relay[3], LOW);
        }
    }
    else
    {
        digitalWrite(relay[2], LOW);
        digitalWrite(relay[3], LOW);
    }

    if (autoPH == 1)
    {
        if (g_fisOutput[2] >= 0.5 && g_fisOutput[3] < 0.5)
        {
            digitalWrite(relay[0], HIGH);
            digitalWrite(relay[1], LOW);
        }
        else if (g_fisOutput[3] >= 0.5 && g_fisOutput[2] < 0.5)
        {
            digitalWrite(relay[0], LOW);
            digitalWrite(relay[1], HIGH);
        }
        else
        {
            digitalWrite(relay[0], LOW);
            digitalWrite(relay[1], LOW);
        }
    }
    else
    {
        digitalWrite(relay[0], LOW);
        digitalWrite(relay[1], LOW);
    }

    // // Set output vlaue: heater
    // analogWrite(2, g_fisOutput[0]);
    // // Set output vlaue: peltier
    // analogWrite(3, g_fisOutput[1]);
    // // Set output vlaue: phup
    // analogWrite(4, g_fisOutput[2]);
    // // Set output vlaue: phdown
    // analogWrite(5, g_fisOutput[3]);

    // Set output vlaue: heater
    Serial.println(g_fisOutput[0]);
    // Set output vlaue: peltier
    Serial.println(g_fisOutput[1]);
    // Set output vlaue: phup
    Serial.println(g_fisOutput[2]);
    // Set output vlaue: phdown
    Serial.println(g_fisOutput[3]);
}

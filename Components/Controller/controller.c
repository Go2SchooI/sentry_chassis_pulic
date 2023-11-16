#include "controller.h"

/******************************** FUZZY PID **********************************/
// static float FuzzyRuleKpRAW[7][7] = {
//     PB, PB, PM, PM, PS, ZE, ZE,
//     PB, PB, PM, PS, PS, ZE, PS,
//     PM, PM, PM, PS, ZE, PS, PS,
//     PM, PM, PS, ZE, PS, PM, PM,
//     PS, PS, ZE, PS, PS, PM, PM,
//     PS, ZE, PS, PM, PM, PM, PB,
//     ZE, ZE, PM, PM, PM, PB, PB};

static float FuzzyRuleKpRAW[7][7] = {
    NB, NB, PM, PM, PS, ZE, ZE,
    NB, NB, PM, PS, PS, ZE, PS,
    NM, NM, PM, PS, ZE, PS, PS,
    NM, NM, PS, ZE, PS, PM, PM,
    NS, NS, ZE, PS, PS, PM, PM,
    NS, ZE, PS, PM, PM, PM, PB,
    ZE, ZE, PM, PM, PM, PB, PB};

static float FuzzyRuleKiRAW[7][7] = {
    PB, PB, PM, PM, PS, ZE, ZE,
    PB, PB, PM, PS, PS, ZE, ZE,
    PB, PM, PM, PS, ZE, PS, PS,
    PM, PM, PS, ZE, PS, PM, PM,
    PS, PS, ZE, PS, PS, PM, PB,
    ZE, ZE, PS, PS, PM, PB, PB,
    ZE, ZE, PS, PM, PM, PB, PB};

static float FuzzyRuleKdRAW[7][7] = {
    PS, PS, PB, PB, PB, PM, PS,
    PS, PS, PB, PM, PM, PS, ZE,
    ZE, PS, PM, PM, PS, PS, ZE,
    ZE, PS, PS, PS, PS, PS, ZE,
    ZE, ZE, ZE, ZE, ZE, ZE, ZE,
    PB, PS, PS, PS, PS, PS, PB,
    PB, PM, PM, PM, PS, PS, PB};

void Fuzzy_Rule_Init(FuzzyRule_t *fuzzyRule, float (*fuzzyRuleKp)[7], float (*fuzzyRuleKi)[7], float (*fuzzyRuleKd)[7],
                     float kpRatio, float kiRatio, float kdRatio,
                     float eStep, float ecStep)
{
    if (fuzzyRuleKp == NULL)
        fuzzyRule->FuzzyRuleKp = FuzzyRuleKpRAW;
    else
        fuzzyRule->FuzzyRuleKp = fuzzyRuleKp;
    if (fuzzyRuleKi == NULL)
        fuzzyRule->FuzzyRuleKi = FuzzyRuleKiRAW;
    else
        fuzzyRule->FuzzyRuleKi = fuzzyRuleKi;
    if (fuzzyRuleKd == NULL)
        fuzzyRule->FuzzyRuleKd = FuzzyRuleKdRAW;
    else
        fuzzyRule->FuzzyRuleKd = fuzzyRuleKd;

    fuzzyRule->KpRatio = kpRatio;
    fuzzyRule->KiRatio = kiRatio;
    fuzzyRule->KdRatio = kdRatio;

    if (eStep < 0.00001f)
        eStep = 1;
    if (ecStep < 0.00001f)
        ecStep = 1;
    fuzzyRule->eStep = eStep;
    fuzzyRule->ecStep = ecStep;
}
void Fuzzy_Rule_Implementation(FuzzyRule_t *fuzzyRule, float measure, float ref)
{
    float eLeftTemp, ecLeftTemp;
    float eRightTemp, ecRightTemp;
    int eLeftIndex, ecLeftIndex;
    int eRightIndex, ecRightIndex;

    fuzzyRule->dt = DWT_GetDeltaT((void *)&fuzzyRule->DWT_CNT);

    fuzzyRule->e = ref - measure;
    fuzzyRule->ec = (fuzzyRule->e - fuzzyRule->eLast) / fuzzyRule->dt; //
    fuzzyRule->eLast = fuzzyRule->e;

    //????????
    eLeftIndex = fuzzyRule->e >= 3 * fuzzyRule->eStep ? 6 : (fuzzyRule->e <= -3 * fuzzyRule->eStep ? 0 : (fuzzyRule->e >= 0 ? ((int)(fuzzyRule->e / fuzzyRule->eStep) + 3) : ((int)(fuzzyRule->e / fuzzyRule->eStep) + 2)));
    eRightIndex = fuzzyRule->e >= 3 * fuzzyRule->eStep ? 6 : (fuzzyRule->e <= -3 * fuzzyRule->eStep ? 0 : (fuzzyRule->e >= 0 ? ((int)(fuzzyRule->e / fuzzyRule->eStep) + 4) : ((int)(fuzzyRule->e / fuzzyRule->eStep) + 3)));
    ecLeftIndex = fuzzyRule->ec >= 3 * fuzzyRule->ecStep ? 6 : (fuzzyRule->ec <= -3 * fuzzyRule->ecStep ? 0 : (fuzzyRule->ec >= 0 ? ((int)(fuzzyRule->ec / fuzzyRule->ecStep) + 3) : ((int)(fuzzyRule->ec / fuzzyRule->ecStep) + 2)));
    ecRightIndex = fuzzyRule->ec >= 3 * fuzzyRule->ecStep ? 6 : (fuzzyRule->ec <= -3 * fuzzyRule->ecStep ? 0 : (fuzzyRule->ec >= 0 ? ((int)(fuzzyRule->ec / fuzzyRule->ecStep) + 4) : ((int)(fuzzyRule->ec / fuzzyRule->ecStep) + 3)));

    //		fuzzyRule->eIntervalCount[eLeftIndex] ++;
    //		fuzzyRule->ecIntervalCount[ecLeftIndex] ++;

    //??????
    eLeftTemp = fuzzyRule->e >= 3 * fuzzyRule->eStep ? 0 : (fuzzyRule->e <= -3 * fuzzyRule->eStep ? 1 : (eRightIndex - fuzzyRule->e / fuzzyRule->eStep - 3));
    eRightTemp = fuzzyRule->e >= 3 * fuzzyRule->eStep ? 1 : (fuzzyRule->e <= -3 * fuzzyRule->eStep ? 0 : (fuzzyRule->e / fuzzyRule->eStep - eLeftIndex + 3));
    ecLeftTemp = fuzzyRule->ec >= 3 * fuzzyRule->ecStep ? 0 : (fuzzyRule->ec <= -3 * fuzzyRule->ecStep ? 1 : (ecRightIndex - fuzzyRule->ec / fuzzyRule->ecStep - 3));
    ecRightTemp = fuzzyRule->ec >= 3 * fuzzyRule->ecStep ? 1 : (fuzzyRule->ec <= -3 * fuzzyRule->ecStep ? 0 : (fuzzyRule->ec / fuzzyRule->ecStep - ecLeftIndex + 3));

    fuzzyRule->eStatue = eLeftIndex;
    fuzzyRule->ecStatue = ecLeftIndex;

    fuzzyRule->KpFuzzyTemp[0] = eLeftTemp * ecLeftTemp * fuzzyRule->FuzzyRuleKp[eLeftIndex][ecLeftIndex];
    fuzzyRule->KpFuzzyTemp[1] = eLeftTemp * ecRightTemp * fuzzyRule->FuzzyRuleKp[eRightIndex][ecLeftIndex];
    fuzzyRule->KpFuzzyTemp[2] = eRightTemp * ecLeftTemp * fuzzyRule->FuzzyRuleKp[eLeftIndex][ecRightIndex];
    fuzzyRule->KpFuzzyTemp[3] = eRightTemp * ecRightTemp * fuzzyRule->FuzzyRuleKp[eRightIndex][ecRightIndex];

    fuzzyRule->KpFuzzy = fuzzyRule->KpFuzzyTemp[0] + fuzzyRule->KpFuzzyTemp[1] + fuzzyRule->KpFuzzyTemp[2] + fuzzyRule->KpFuzzyTemp[3];
    //		fuzzyRule->KpFuzzy = eLeftTemp * ecLeftTemp * fuzzyRule->FuzzyRuleKp[eLeftIndex][ecLeftIndex] +
    //                         eLeftTemp * ecRightTemp * fuzzyRule->FuzzyRuleKp[eLeftIndex][ecRightIndex] +
    //                         eRightTemp * ecLeftTemp * fuzzyRule->FuzzyRuleKp[eRightIndex][ecLeftIndex] +
    //                         eRightTemp * ecRightTemp * fuzzyRule->FuzzyRuleKp[eRightIndex][ecRightIndex];

    fuzzyRule->KpFuzzy *= fuzzyRule->KpRatio;
    //		fuzzyRule->KpFuzzy = (fuzzyRule->KpFuzzy + fuzzyRule->LastKpFuzzy) / 2;
    //		fuzzyRule->LastKpFuzzy = fuzzyRule->KpFuzzy;

    fuzzyRule->KiFuzzy = eLeftTemp * ecLeftTemp * fuzzyRule->FuzzyRuleKi[eLeftIndex][ecLeftIndex] +
                         eLeftTemp * ecRightTemp * fuzzyRule->FuzzyRuleKi[eRightIndex][ecLeftIndex] +
                         eRightTemp * ecLeftTemp * fuzzyRule->FuzzyRuleKi[eLeftIndex][ecRightIndex] +
                         eRightTemp * ecRightTemp * fuzzyRule->FuzzyRuleKi[eRightIndex][ecRightIndex];
    fuzzyRule->KiFuzzy *= fuzzyRule->KiRatio;

    fuzzyRule->KdFuzzy = eLeftTemp * ecLeftTemp * fuzzyRule->FuzzyRuleKd[eLeftIndex][ecLeftIndex] +
                         eLeftTemp * ecRightTemp * fuzzyRule->FuzzyRuleKd[eRightIndex][ecLeftIndex] +
                         eRightTemp * ecLeftTemp * fuzzyRule->FuzzyRuleKd[eLeftIndex][ecRightIndex] +
                         eRightTemp * ecRightTemp * fuzzyRule->FuzzyRuleKd[eRightIndex][ecRightIndex];
    fuzzyRule->KdFuzzy *= fuzzyRule->KdRatio;
}

/******************************* PID CONTROL *********************************/
// PID??????????????
static void f_Trapezoid_Intergral(PID_t *pid);
static void f_Integral_Limit(PID_t *pid);
static void f_Derivative_On_Measurement(PID_t *pid);
static void f_Changing_Integration_Rate(PID_t *pid);
static void f_Output_Filter(PID_t *pid);
static void f_Derivative_Filter(PID_t *pid);
static void f_Output_Limit(PID_t *pid);
static void f_Proportion_Limit(PID_t *pid);
static void f_PID_ErrorHandle(PID_t *pid);

/**
 * @brief          PID?????   PID initialize
 * @param[in]      PID????   PID structure
 * @param[in]      ??
 * @retval         ?????      null
 */
void PID_Init(
    PID_t *pid,
    float max_out,
    float intergral_limit,
    float deadband,

    float kp,
    float Ki,
    float Kd,

    float A,
    float B,

    float output_lpf_rc,
    float derivative_lpf_rc,

    uint16_t ols_order,

    uint8_t improve)
{
    pid->DeadBand = deadband;
    pid->IntegralLimit = intergral_limit;
    pid->MaxOut = max_out;
    pid->Ref = 0;

    pid->Kp = kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->ITerm = 0;

    // ??????????
    // coefficient of changing integration rate
    pid->CoefA = A;
    pid->CoefB = B;

    pid->Output_LPF_RC = output_lpf_rc;

    pid->Derivative_LPF_RC = derivative_lpf_rc;

    // ??��?????????????????
    // differential signal is distilled by OLS
    pid->OLS_Order = ols_order;
    OLS_Init(&pid->OLS, ols_order);

    // DWT?????????????????
    // reset DWT Timer count counter
    pid->DWT_CNT = 0;

    // ????PID???????
    pid->Improve = improve;

    // ????PID?????? ??????????????????
    pid->ERRORHandler.ERRORCount = 0;
    pid->ERRORHandler.ERRORType = PID_ERROR_NONE;

    pid->Output = 0;
}

/**
 * @brief          PID????
 * @param[in]      PID????
 * @param[in]      ?????
 * @param[in]      ?????
 * @retval         ?????
 */
float PID_Calculate(PID_t *pid, float measure, float ref)
{
    if (pid->Improve & ErrorHandle)
        f_PID_ErrorHandle(pid);

    pid->dt = DWT_GetDeltaT((void *)&pid->DWT_CNT);

    pid->Measure = measure;
    pid->Ref = ref;
    if (!isnormal(pid->Measure))
        pid->Measure = 0;
    if (!isnormal(pid->Ref))
        pid->Ref = 0;
    pid->Err = pid->Ref - pid->Measure;

    if (pid->User_Func1_f != NULL)
        pid->User_Func1_f(pid);

    if (abs(pid->Err) > pid->DeadBand)
    {
        if (pid->FuzzyRule == NULL)
        {
            pid->Pout = pid->Kp * pid->Err;
            pid->ITerm = pid->Ki * pid->Err * pid->dt;
            // if (pid->OLS_Order > 2)
            //     pid->Dout = pid->Kd * OLS_Derivative(&pid->OLS, pid->dt, pid->Err);
            // else
            pid->Dout = pid->Kd * (pid->Err - pid->Last_Err) / pid->dt;
        }
        else
        {
            pid->Pout = (pid->Kp + pid->FuzzyRule->KpFuzzy) * pid->Err;
            pid->ITerm = (pid->Ki + pid->FuzzyRule->KiFuzzy) * pid->Err * pid->dt;
            // if (pid->OLS_Order > 2)
            //     pid->Dout = (pid->Kd + pid->FuzzyRule->KdFuzzy) * OLS_Derivative(&pid->OLS, pid->dt, pid->Err);
            // else
            pid->Dout = (pid->Kd + pid->FuzzyRule->KdFuzzy) * (pid->Err - pid->Last_Err) / pid->dt;
        }

        if (pid->User_Func2_f != NULL)
            pid->User_Func2_f(pid);

        // ???��???
        if (pid->Improve & Trapezoid_Intergral)
            f_Trapezoid_Intergral(pid);
        // ???????
        if (pid->Improve & ChangingIntegrationRate)
            f_Changing_Integration_Rate(pid);
        // ???????
        if (pid->Improve & Derivative_On_Measurement)
            f_Derivative_On_Measurement(pid);
        // ????????
        if (pid->Improve & DerivativeFilter)
            f_Derivative_Filter(pid);
        // ???????
        if (pid->Improve & Integral_Limit)
            f_Integral_Limit(pid);

        if (!isnormal(pid->ITerm))
            pid->ITerm = 0;

        pid->Iout += pid->ITerm;

        if (!isnormal(pid->Pout))
            pid->Pout = 0;
        if (!isnormal(pid->Iout))
            pid->Iout = 0;
        if (!isnormal(pid->Dout))
            pid->Dout = 0;

        pid->Output = pid->Pout + pid->Iout + pid->Dout;

        // ??????
        if (pid->Improve & OutputFilter)
            f_Output_Filter(pid);

        // ??????
        f_Output_Limit(pid);

        // ?????
        f_Proportion_Limit(pid);
    }

    pid->Last_Measure = pid->Measure;
    pid->Last_Output = pid->Output;
    pid->Last_Dout = pid->Dout;
    pid->Last_Err = pid->Err;
    pid->Last_ITerm = pid->ITerm;

    return pid->Output;
}
static void f_Trapezoid_Intergral(PID_t *pid)
{
    if (pid->FuzzyRule == NULL)
        pid->ITerm = pid->Ki * ((pid->Err + pid->Last_Err) / 2) * pid->dt;
    else
        pid->ITerm = (pid->Ki + pid->FuzzyRule->KiFuzzy) * ((pid->Err + pid->Last_Err) / 2) * pid->dt;
}

static void f_Changing_Integration_Rate(PID_t *pid)
{
    if (pid->Err * pid->Iout > 0)
    {
        // ????????????
        // Integral still increasing
        if (abs(pid->Err) <= pid->CoefB)
            return; // Full integral
        if (abs(pid->Err) <= (pid->CoefA + pid->CoefB))
            pid->ITerm *= (pid->CoefA - abs(pid->Err) + pid->CoefB) / pid->CoefA;
        else
            pid->ITerm = 0;
    }
}

static void f_Integral_Limit(PID_t *pid)
{
    static float temp_Output, temp_Iout;
    temp_Iout = pid->Iout + pid->ITerm;
    temp_Output = pid->Pout + pid->Iout + pid->Dout;
    if (abs(temp_Output) > pid->MaxOut)
    {
        if (pid->Err * pid->Iout > 0)
        {
            // ????????????
            // Integral still increasing
            pid->ITerm = 0;
        }
    }

    if (temp_Iout > pid->IntegralLimit)
    {
        pid->ITerm = 0;
        pid->Iout = pid->IntegralLimit;
    }
    if (temp_Iout < -pid->IntegralLimit)
    {
        pid->ITerm = 0;
        pid->Iout = -pid->IntegralLimit;
    }
}

static void f_Derivative_On_Measurement(PID_t *pid)
{
    if (pid->FuzzyRule == NULL)
    {
        // if (pid->OLS_Order > 2)
        //     pid->Dout = pid->Kd * OLS_Derivative(&pid->OLS, pid->dt, -pid->Measure);
        // else
        pid->Dout = pid->Kd * (pid->Last_Measure - pid->Measure) / pid->dt;
    }
    else
    {
        // if (pid->OLS_Order > 2)
        //     pid->Dout = (pid->Kd + pid->FuzzyRule->KdFuzzy) * OLS_Derivative(&pid->OLS, pid->dt, -pid->Measure);
        // else
        pid->Dout = (pid->Kd + pid->FuzzyRule->KdFuzzy) * (pid->Last_Measure - pid->Measure) / pid->dt;
    }
}

static void f_Derivative_Filter(PID_t *pid)
{
    pid->Dout = pid->Dout * pid->dt / (pid->Derivative_LPF_RC + pid->dt) +
                pid->Last_Dout * pid->Derivative_LPF_RC / (pid->Derivative_LPF_RC + pid->dt);
}

static void f_Output_Filter(PID_t *pid)
{
    pid->Output = pid->Output * pid->dt / (pid->Output_LPF_RC + pid->dt) +
                  pid->Last_Output * pid->Output_LPF_RC / (pid->Output_LPF_RC + pid->dt);
}

static void f_Output_Limit(PID_t *pid)
{
    if (pid->Output > pid->MaxOut)
    {
        pid->Output = pid->MaxOut;
    }
    if (pid->Output < -(pid->MaxOut))
    {
        pid->Output = -(pid->MaxOut);
    }
}

static void f_Proportion_Limit(PID_t *pid)
{
    if (pid->Pout > pid->MaxOut)
    {
        pid->Pout = pid->MaxOut;
    }
    if (pid->Pout < -(pid->MaxOut))
    {
        pid->Pout = -(pid->MaxOut);
    }
}

// PID ERRORHandle Function
static void f_PID_ErrorHandle(PID_t *pid)
{
    /*Motor Blocked Handle*/
    if (pid->Output < pid->MaxOut * 0.001f || fabsf(pid->Ref) < 0.0001f)
        return;

    if ((fabsf(pid->Ref - pid->Measure) / fabsf(pid->Ref)) > 0.95f)
    {
        // Motor blocked counting
        pid->ERRORHandler.ERRORCount++;
    }
    else
    {
        pid->ERRORHandler.ERRORCount = 0;
    }

    if (pid->ERRORHandler.ERRORCount > 500)
    {
        // Motor blocked over 1000times
        pid->ERRORHandler.ERRORType = Motor_Blocked;
    }
}

/*************************** FEEDFORWARD CONTROL *****************************/
/**
 * @brief          ???????????
 * @param[in]      ??????????
 * @param[in]      ??
 * @retval         ?????
 */
void Feedforward_Init(
    Feedforward_t *ffc,
    float max_out,
    float *c,
    float lpf_rc,
    uint16_t ref_dot_ols_order,
    uint16_t ref_ddot_ols_order)
{
    ffc->MaxOut = max_out;

    // ????????????????? ????????????�G??
    // set parameters of feed-forward controller (see struct definition)
    if (c != NULL && ffc != NULL)
    {
        ffc->c[0] = c[0];
        ffc->c[1] = c[1];
        ffc->c[2] = c[2];
    }
    else
    {
        ffc->c[0] = 0;
        ffc->c[1] = 0;
        ffc->c[2] = 0;
        ffc->MaxOut = 0;
    }

    ffc->LPF_RC = lpf_rc;

    // ??��?????????????????
    // differential signal is distilled by OLS
    ffc->Ref_dot_OLS_Order = ref_dot_ols_order;
    ffc->Ref_ddot_OLS_Order = ref_ddot_ols_order;
    if (ref_dot_ols_order > 2)
        OLS_Init(&ffc->Ref_dot_OLS, ref_dot_ols_order);
    if (ref_ddot_ols_order > 2)
        OLS_Init(&ffc->Ref_ddot_OLS, ref_ddot_ols_order);

    ffc->DWT_CNT = 0;

    ffc->Output = 0;
}

/**
 * @brief          PID????
 * @param[in]      PID????
 * @param[in]      ?????
 * @param[in]      ?????
 * @retval         ?????
 */
float Feedforward_Calculate(Feedforward_t *ffc, float ref)
{
    ffc->dt = DWT_GetDeltaT((void *)&ffc->DWT_CNT);

    ffc->Ref = ref * ffc->dt / (ffc->LPF_RC + ffc->dt) +
               ffc->Ref * ffc->LPF_RC / (ffc->LPF_RC + ffc->dt);

    if (!isnormal(ffc->Ref))
        ffc->Ref = 0;
    // ??????????
    // calculate first derivative
    // if (ffc->Ref_dot_OLS_Order > 2)
    //     ffc->Ref_dot = OLS_Derivative(&ffc->Ref_dot_OLS, ffc->dt, ffc->Ref);
    // else
    ffc->Ref_dot = (ffc->Ref - ffc->Last_Ref) / ffc->dt;

    if (!isnormal(ffc->Ref_dot))
        ffc->Ref_dot = 0;

    // ??????????
    // calculate second derivative
    // if (ffc->Ref_ddot_OLS_Order > 2)
    //     ffc->Ref_ddot = OLS_Derivative(&ffc->Ref_ddot_OLS, ffc->dt, ffc->Ref_dot);
    // else
    ffc->Ref_ddot = (ffc->Ref_dot - ffc->Last_Ref_dot) / ffc->dt;

    if (!isnormal(ffc->Ref_ddot))
        ffc->Ref_ddot = 0;

    // ??????????????
    // calculate feed-forward controller output
    ffc->Output = ffc->c[0] * ffc->Ref + ffc->c[1] * ffc->Ref_dot + ffc->c[2] * ffc->Ref_ddot;

    ffc->Output = float_constrain(ffc->Output, -ffc->MaxOut, ffc->MaxOut);

    if (!isnormal(ffc->Output))
        ffc->Output = 0;

    ffc->Last_Ref = ffc->Ref;
    ffc->Last_Ref_dot = ffc->Ref_dot;

    return ffc->Output;
}

/*************************LINEAR DISTURBANCE OBSERVER *************************/
void LDOB_Init(
    LDOB_t *ldob,
    float max_d,
    float deadband,
    float *c,
    float lpf_rc,
    uint16_t measure_dot_ols_order,
    uint16_t measure_ddot_ols_order)
{
    ldob->Max_Disturbance = max_d;

    ldob->DeadBand = deadband;

    // ???????????????????? ???LDOB???�G??
    // set parameters of linear disturbance observer (see struct definition)
    if (c != NULL && ldob != NULL)
    {
        ldob->c[0] = c[0];
        ldob->c[1] = c[1];
        ldob->c[2] = c[2];
    }
    else
    {
        ldob->c[0] = 0;
        ldob->c[1] = 0;
        ldob->c[2] = 0;
        ldob->Max_Disturbance = 0;
    }

    // ????Q(s)????  Q(s)????????????
    // set bandwidth of Q(s)    Q(s) is chosen as a first-order low-pass form
    ldob->LPF_RC = lpf_rc;

    // ??��?????????????????
    // differential signal is distilled by OLS
    ldob->Measure_dot_OLS_Order = measure_dot_ols_order;
    ldob->Measure_ddot_OLS_Order = measure_ddot_ols_order;
    if (measure_dot_ols_order > 2)
        OLS_Init(&ldob->Measure_dot_OLS, measure_dot_ols_order);
    if (measure_ddot_ols_order > 2)
        OLS_Init(&ldob->Measure_ddot_OLS, measure_ddot_ols_order);

    ldob->DWT_CNT = 0;

    ldob->Disturbance = 0;
}

float LDOB_Calculate(LDOB_t *ldob, float measure, float u)
{
    ldob->dt = DWT_GetDeltaT((void *)&ldob->DWT_CNT);

    ldob->Measure = measure;

    ldob->u = u;

    // ??????????
    // calculate first derivative
    // if (ldob->Measure_dot_OLS_Order > 2)
    //     ldob->Measure_dot = OLS_Derivative(&ldob->Measure_dot_OLS, ldob->dt, ldob->Measure);
    // else
    ldob->Measure_dot = (ldob->Measure - ldob->Last_Measure) / ldob->dt;

    // ??????????
    // calculate second derivative
    // if (ldob->Measure_ddot_OLS_Order > 2)
    //     ldob->Measure_ddot = OLS_Derivative(&ldob->Measure_ddot_OLS, ldob->dt, ldob->Measure_dot);
    // else
    ldob->Measure_ddot = (ldob->Measure_dot - ldob->Last_Measure_dot) / ldob->dt;

    // ?????????
    // estimate external disturbances and internal disturbances caused by model uncertainties
    ldob->Disturbance = ldob->c[0] * ldob->Measure + ldob->c[1] * ldob->Measure_dot + ldob->c[2] * ldob->Measure_ddot - ldob->u;
    ldob->Disturbance = ldob->Disturbance * ldob->dt / (ldob->LPF_RC + ldob->dt) +
                        ldob->Last_Disturbance * ldob->LPF_RC / (ldob->LPF_RC + ldob->dt);

    ldob->Disturbance = float_constrain(ldob->Disturbance, -ldob->Max_Disturbance, ldob->Max_Disturbance);

    // ??????????
    // deadband of disturbance output
    if (abs(ldob->Disturbance) > ldob->DeadBand * ldob->Max_Disturbance)
        ldob->Output = ldob->Disturbance;
    else
        ldob->Output = 0;

    ldob->Last_Measure = ldob->Measure;
    ldob->Last_Measure_dot = ldob->Measure_dot;
    ldob->Last_Disturbance = ldob->Disturbance;

    return ldob->Output;
}

/*************************** Tracking Differentiator ***************************/
void TD_Init(TD_t *td, float r, float h0)
{
    td->r = r;
    td->h0 = h0;

    td->x = 0;
    td->dx = 0;
    td->ddx = 0;
    td->last_dx = 0;
    td->last_ddx = 0;
}
float TD_Calculate(TD_t *td, float input)
{
    static float d, a0, y, a1, a2, a, fhan;

    td->dt = DWT_GetDeltaT((void *)&td->DWT_CNT);

    if (td->dt > 0.5f)
        return 0;

    td->Input = input;

    d = td->r * td->h0 * td->h0;
    a0 = td->dx * td->h0;
    y = td->x - td->Input + a0;
    a1 = sqrt(d * (d + 8 * abs(y)));
    a2 = a0 + sign(y) * (a1 - d) / 2;
    a = (a0 + y) * (sign(y + d) - sign(y - d)) / 2 + a2 * (1 - (sign(y + d) - sign(y - d)) / 2);
    fhan = -td->r * a / d * (sign(a + d) - sign(a - d)) / 2 -
           td->r * sign(a) * (1 - (sign(a + d) - sign(a - d)) / 2);

    td->ddx = fhan;
    td->dx += (td->ddx + td->last_ddx) * td->dt / 2;
    td->x += (td->dx + td->last_dx) * td->dt / 2;

    td->last_ddx = td->ddx;
    td->last_dx = td->dx;

    return td->x;
}

/************** Second Order System based Tracking Differentiator **************/
void ThirdOrder_TD_Init(ThirdOrderTD_t *td, float omega)
{
    td->Omega = omega;

    td->x = 0;
    td->dx = 0;
    td->ddx = 0;
    td->last_dx = 0;
    td->last_ddx = 0;
    td->last_dddx = 0;
}

float ThirdOrder_TD_Calculate(ThirdOrderTD_t *td, float input)
{
    td->dt = DWT_GetDeltaT((void *)&td->DWT_CNT);

    td->Input = input;

    td->dddx = td->Omega * td->Omega * td->Omega * (td->Input - td->x) -
               3 * td->Omega * td->Omega * td->dx -
               3 * td->Omega * td->ddx;

    // Trapezoid intergral
    td->ddx += (td->last_dddx + td->dddx) * td->dt / 2;
    td->dx += (td->last_ddx + td->ddx) * td->dt / 2;
    td->x += (td->last_dx + td->dx) * td->dt / 2;

    td->last_dx = td->dx;
    td->last_ddx = td->ddx;
    td->last_dddx = td->dddx;

    return td->x;
}

/**************************** SYSTEM IDENTIFICATION ****************************/
//?????????? ?????? ??��??????��?????
// void SI_Init(
//    SI_t *si,
//    float *c,
//    uint8_t order,
//    uint8_t fric,
//    float a,
//    uint8_t *param_to_idf,
//    uint16_t sample_size,
//    float lpf_rc)
//{
//    si->a = a;

//    si->c[0] = c[0];
//    si->c[1] = c[1];
//    si->c[2] = c[2];
//    si->c[3] = c[3];

//    si->System_Order = order;
//    si->Dynamic_Friction = fric;

//    si->u = user_malloc(sizeof(float) * sample_size);
//    si->x = user_malloc(sizeof(float) * sample_size);
//    si->x_dot = user_malloc(sizeof(float) * sample_size);
//    if (si->System_Order > 2)
//        si->x_ddot = user_malloc(sizeof(float) * sample_size);
//    else
//        c[2] = 0;

//    if (si->Dynamic_Friction == 0)
//        c[3] = 0;

//    si->LPF_RC = lpf_rc;

//    si->DWT_CNT = 0;
//}

// void SI_Sample_Update(SI_t *si, float x, float u)
//{
//     si->dt = DWT_GetDeltaT(&si->DWT_CNT);

//    for (uint16_t i = 0; i < si->Sample_Size - 1; ++i)
//    {
//        si->u[i] = si->u[i + 1];
//        si->x[i] = si->x[i + 1];
//        si->x_dot[i] = si->x_dot[i + 1];
//        if (si->System_Order > 2)
//            si->x_ddot[i] = si->x_ddot[i + 1];
//    }

//    si->u[si->Sample_Size - 1] = u * si->LPF_RC / (si->LPF_RC + si->dt) +
//                                 si->u[si->Sample_Size - 2] * si->dt / (si->LPF_RC + si->dt);
//    si->x[si->Sample_Size - 1] = x * si->LPF_RC / (si->LPF_RC + si->dt) +
//                                 si->x[si->Sample_Size - 2] * si->dt / (si->LPF_RC + si->dt);
//    si->x_dot[si->Sample_Size - 1] = (si->x[si->Sample_Size - 1] - si->x[si->Sample_Size - 2]) / si->dt;
//    if (si->System_Order > 2)
//        si->x_ddot[si->Sample_Size - 1] = (si->x_dot[si->Sample_Size - 1] - si->x_dot[si->Sample_Size - 2]) / si->dt;
//}

// static float Cost_Func(SI_t *si, float c0, float c2, float c1, float c3)
//{
//     static float Cost = 0, signx;
//     Cost = 0;
//     if (si->System_Order > 2)
//     {
//         for (uint16_t i = 0; i < si->Sample_Size; ++i)
//         {
//             if (si->x[i] > 0)
//                 signx = 1.0f;
//             else
//                 signx = -1.0f;

//            Cost += (si->u[i] - c0 * si->x[i] - c1 * si->x_dot[i] -
//                     c2 * si->x_ddot[i] - c3 * signx * si->Dynamic_Friction) *
//                    (si->u[i] - c0 * si->x[i] - c1 * si->x_dot[i] -
//                     c2 * si->x_ddot[i] - c3 * signx * si->Dynamic_Friction);
//        }
//    }
//    else
//    {
//        for (uint16_t i = 0; i < si->Sample_Size; ++i)
//        {
//            if (si->x[i] > 0)
//                signx = 1.0f;
//            else
//                signx = -1.0f;

//            Cost += (si->u[i] - c0 * si->x[i] - c1 * si->x_dot[i] -
//                     c3 * signx * si->Dynamic_Friction) *
//                    (si->u[i] - c0 * si->x[i] - c1 * si->x_dot[i] -
//                     c3 * signx * si->Dynamic_Friction);
//        }
//    }
//    return Cost;
//}

// void SI_Iterate(SI_t *si)
//{
// }

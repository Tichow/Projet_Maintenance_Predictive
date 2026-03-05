/**
  ******************************************************************************
  * @file    main.c
  * @brief   reconnaissance de chiffres MNIST sur ecran tactile STM32L4R9I-DISCO
  *          base sur l'exemple BSP - LCD + tactile + X-CUBE-AI
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_x-cube-ai.h"
#include "mnist.h"
#include <math.h>

/* Private defines -----------------------------------------------------------*/
#define MFX_IRQ_PENDING_GPIO    0x01
#define MFX_IRQ_PENDING_IDD     0x02
#define MFX_IRQ_PENDING_ERROR   0x04
#define MFX_IRQ_PENDING_TS_DET  0x08
#define MFX_IRQ_PENDING_TS_NE   0x10
#define MFX_IRQ_PENDING_TS_TH   0x20
#define MFX_IRQ_PENDING_TS_FULL 0x40
#define MFX_IRQ_PENDING_TS_OVF  0x80

#define LTDC_ACTIVE_LAYER       LTDC_DEFAULT_ACTIVE_LAYER

/* disposition du canvas de dessin */
#define CANVAS_X        69
#define CANVAS_Y        40
#define CANVAS_SIZE     252       /* 252 = 28 * 9, divisible par 28 */
#define BLOCK_SIZE      (CANVAS_SIZE / 28)  /* = 9 px par cellule */
#define BRUSH_RADIUS    12

/* parametres du modele */
#define IMG_ROWS        28
#define IMG_COLS        28
#define NUM_CLASSES     10

/* Private variables ---------------------------------------------------------*/
#if defined(LCD_DIMMING)
RTC_HandleTypeDef RTCHandle;
__IO uint32_t display_dimmed     = 0;
__IO uint32_t maintain_display   = 0;
#if defined(LCD_OFF_AFTER_DIMMING)
__IO uint32_t display_turned_off = 0;
#endif
TIM_HandleTypeDef    TimHandle;
static void Timer_Init(TIM_HandleTypeDef * TimHandle);
uint32_t brightness = 0;
extern DSI_HandleTypeDef    hdsi_discovery;
#endif

uint32_t AudioPlayback = 0;

FlagStatus LcdInitialized = RESET;
FlagStatus TsInitialized  = RESET;
FlagStatus LedInitialized = RESET;
FlagStatus JoyInitialized = RESET;

__IO JOYState_TypeDef JoyState = JOY_NONE;
__IO FlagStatus MfxItOccurred = RESET;
__IO uint32_t TouchEvent = 0;

/* grille de dessin 28x28, chaque cellule = intensite du pixel */
static uint8_t drawing_grid[IMG_ROWS][IMG_COLS];
static float model_input[IMG_ROWS * IMG_COLS];

/* coordonnees du point tactile precedent pour tracer des lignes */
static int16_t prev_tx = -1, prev_ty = -1;

/* prototypes des fonctions privees */
static void DrawUI(void);
static void MnistTouchscreen_Process(void);
static void UpdateGrid(uint16_t touch_x, uint16_t touch_y);
static void UpdateGridLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
static void ClearCanvas(void);
static void RunInference(void);
static void DisplayResult(int predicted, float *probabilities);

/**
  * @brief  programme principal
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  SystemHardwareInit();

  /* initialisation du reseau de neurones */
  MX_X_CUBE_AI_Init();

  /* afficher l'interface de dessin */
  while(BSP_LCD_IsFrameBufferAvailable() != LCD_OK);
  DrawUI();
  BSP_LCD_Refresh();

  /* boucle principale */
  while (1)
  {
    MnistTouchscreen_Process();
  }
}

/* ======== application mnist ================================================ */

/**
  * @brief  dessine l'interface utilisateur (titre, canvas noir, instructions)
  */
static void DrawUI(void)
{
  BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);
  BSP_LCD_SetFont(&Font24);

  /* fond blanc */
  BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
  BSP_LCD_Clear(LCD_COLOR_WHITE);

  /* titre */
  BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
  BSP_LCD_DisplayStringAt(0, 20, (uint8_t *)"MNIST AI", CENTER_MODE);

  /* canvas noir pour dessiner */
  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_DrawRect(CANVAS_X - 1, CANVAS_Y - 1, CANVAS_SIZE + 2, CANVAS_SIZE + 2);
  BSP_LCD_FillRect(CANVAS_X, CANVAS_Y, CANVAS_SIZE, CANVAS_SIZE);

  /* instructions en bas */
  BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
  BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
  BSP_LCD_SetFont(&Font16);
  BSP_LCD_DisplayStringAt(0, CANVAS_Y + CANVAS_SIZE + 10,
                          (uint8_t *)"SEL=Predict  LEFT=Clear", CENTER_MODE);
}

/**
  * @brief  boucle de traitement : lecture tactile, dessin, joystick
  */
static void MnistTouchscreen_Process(void)
{
  TS_StateTypeDef ts_state;

  /* le bouton SEL est sur un GPIO direct (pas via MFX), on le traite ici */
  if (JoyState == JOY_SEL)
  {
    JoyState = JOY_NONE;
    RunInference();
  }

  /* evenements MFX (directions du joystick) */
  if (MfxItOccurred == SET)
  {
    Mfx_Event();

#if defined(LCD_DIMMING)
    maintain_display = 1;
    if (display_dimmed)
    {
      display_dimmed = 0;
      HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x51, BRIGHTNESS_MAX);
    }
#if defined(LCD_OFF_AFTER_DIMMING)
    if (display_turned_off)
    {
      display_turned_off = 0;
      BSP_LCD_DisplayOn();
    }
#endif
#endif

    if (JoyState == JOY_LEFT)
    {
      ClearCanvas();
    }
  }

  /* lecture tactile en polling continu pour un dessin fluide */
  BSP_TS_GetState(&ts_state);

  if (ts_state.touchDetected)
  {
    uint16_t tx = ts_state.touchX[0];
    uint16_t ty = ts_state.touchY[0];

    /* verifier que le toucher est dans la zone du canvas */
    if (tx >= CANVAS_X && tx < (CANVAS_X + CANVAS_SIZE) &&
        ty >= CANVAS_Y && ty < (CANVAS_Y + CANVAS_SIZE))
    {
      while (BSP_LCD_IsFrameBufferAvailable() != LCD_OK);
      BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

      if (prev_tx >= 0 && prev_ty >= 0)
      {
        /* tracer des cercles le long de la ligne pour un trait uniforme */
        int dx = tx - prev_tx;
        int dy = ty - prev_ty;
        int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
        if (steps > 0)
        {
          float xinc = (float)dx / steps;
          float yinc = (float)dy / steps;
          float px = prev_tx, py = prev_ty;
          int step = BRUSH_RADIUS / 2;
          if (step < 1) step = 1;
          for (int i = 0; i <= steps; i += step)
          {
            BSP_LCD_FillCircle((uint16_t)(px + 0.5f), (uint16_t)(py + 0.5f), BRUSH_RADIUS / 2);
            px += xinc * step;
            py += yinc * step;
          }
        }
        BSP_LCD_FillCircle(tx, ty, BRUSH_RADIUS / 2);
        UpdateGridLine(prev_tx, prev_ty, tx, ty);
      }
      else
      {
        BSP_LCD_FillCircle(tx, ty, BRUSH_RADIUS / 2);
        UpdateGrid(tx, ty);
      }

      BSP_LCD_Refresh();
      prev_tx = tx;
      prev_ty = ty;
    }
  }
  else
  {
    /* doigt leve, on reset les coordonnees precedentes */
    prev_tx = -1;
    prev_ty = -1;
  }

  HAL_Delay(10);  /* polling a ~100 Hz */
}

/**
  * @brief  met a jour la grille 28x28 autour du point touche
  */
static void UpdateGrid(uint16_t touch_x, uint16_t touch_y)
{
  /* conversion coordonnees ecran -> coordonnees grille */
  int cx = (touch_x - CANVAS_X) / BLOCK_SIZE;
  int cy = (touch_y - CANVAS_Y) / BLOCK_SIZE;
  int r = 2;  /* rayon de ~2 cellules, similaire a l'epaisseur MNIST */

  for (int dy = -r; dy <= r; dy++)
  {
    for (int dx = -r; dx <= r; dx++)
    {
      int gx = cx + dx;
      int gy = cy + dy;
      if (gx >= 0 && gx < IMG_COLS && gy >= 0 && gy < IMG_ROWS)
      {
        float dist = sqrtf((float)(dx * dx + dy * dy));
        if (dist <= r)
        {
          drawing_grid[gy][gx] = 255;
        }
      }
    }
  }
}

/**
  * @brief  met a jour la grille le long d'une ligne entre deux points
  */
static void UpdateGridLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int steps = (dx > dy) ? dx : dy;
  if (steps == 0) { UpdateGrid(x0, y0); return; }

  float xinc = (float)(x1 - x0) / steps;
  float yinc = (float)(y1 - y0) / steps;
  float x = x0, y = y0;

  /* echantillonner tous les BLOCK_SIZE/2 pixels */
  int step_size = BLOCK_SIZE / 2;
  if (step_size < 1) step_size = 1;

  for (int i = 0; i <= steps; i += step_size)
  {
    UpdateGrid((uint16_t)(x + 0.5f), (uint16_t)(y + 0.5f));
    x += xinc * step_size;
    y += yinc * step_size;
  }
  UpdateGrid(x1, y1);
}

/**
  * @brief  efface le canvas et reinitialise la grille
  */
static void ClearCanvas(void)
{
  memset(drawing_grid, 0, sizeof(drawing_grid));
  prev_tx = -1;
  prev_ty = -1;

  while (BSP_LCD_IsFrameBufferAvailable() != LCD_OK);
  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_FillRect(CANVAS_X, CANVAS_Y, CANVAS_SIZE, CANVAS_SIZE);

  /* effacer la zone de resultat */
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_FillRect(0, CANVAS_Y + CANVAS_SIZE + 30, BSP_LCD_GetXSize(), 40);

  BSP_LCD_Refresh();
}

/**
  * @brief  lance l'inference du modele MNIST sur la grille dessinee
  */
static void RunInference(void)
{
  /* flou gaussien 3x3 pour lisser les traits (simule l'anti-aliasing MNIST) */
  static const uint8_t kernel[3][3] = {
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1}
  };  /* somme = 16 */

  for (int i = 0; i < IMG_ROWS; i++)
  {
    for (int j = 0; j < IMG_COLS; j++)
    {
      uint32_t sum = 0;
      for (int ki = -1; ki <= 1; ki++)
      {
        for (int kj = -1; kj <= 1; kj++)
        {
          int ni = i + ki;
          int nj = j + kj;
          if (ni >= 0 && ni < IMG_ROWS && nj >= 0 && nj < IMG_COLS)
          {
            sum += drawing_grid[ni][nj] * kernel[ki + 1][kj + 1];
          }
        }
      }
      /* normalisation [0, 1] comme le dataset MNIST */
      model_input[i * IMG_COLS + j] = (sum / 16.0f) / 255.0f;
    }
  }

  /* copier dans le buffer d'entree du reseau */
  memcpy(data_ins[0], model_input, AI_MNIST_IN_1_SIZE_BYTES);

  /* lancer l'inference */
  if (ai_run() != 0)
  {
    DisplayResult(-1, NULL);
    return;
  }

  /* lire la sortie : 10 probabilites softmax */
  float *output = (float *)data_outs[0];

  /* chercher l'argmax (chiffre le plus probable) */
  int predicted = 0;
  float max_prob = output[0];
  for (int i = 1; i < NUM_CLASSES; i++)
  {
    if (output[i] > max_prob)
    {
      max_prob = output[i];
      predicted = i;
    }
  }

  DisplayResult(predicted, output);
}

/**
  * @brief  affiche le resultat de la prediction sur l'ecran
  */
static void DisplayResult(int predicted, float *probabilities)
{
  char text[40];

  while (BSP_LCD_IsFrameBufferAvailable() != LCD_OK);

  /* effacer la zone de resultat */
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_FillRect(0, CANVAS_Y + CANVAS_SIZE + 30, BSP_LCD_GetXSize(), 40);

  if (predicted < 0)
  {
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font24);
    BSP_LCD_DisplayStringAt(0, CANVAS_Y + CANVAS_SIZE + 35,
                            (uint8_t *)"Erreur inference!", CENTER_MODE);
  }
  else
  {
    BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font24);
    int pct = (int)(probabilities[predicted] * 100.0f);
    sprintf(text, "Predit: %d  (%d%%)", predicted, pct);
    BSP_LCD_DisplayStringAt(0, CANVAS_Y + CANVAS_SIZE + 35,
                            (uint8_t *)text, CENTER_MODE);
  }

  BSP_LCD_Refresh();

  /* feedback visuel avec la led */
  BSP_LED_On(LED1);
  HAL_Delay(200);
  BSP_LED_Off(LED1);
}

/* ======== infrastructure BSP (repris de l'exemple BSP ST) ================== */

void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
  __HAL_RCC_PWR_CLK_DISABLE();

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    while(1);
  }

  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    while(1);
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    while(1);
  }
}

void SystemHardwareInit(void)
{
  if(LedInitialized != SET)
  {
    BSP_LED_Init(LED1);
    BSP_LED_Init(LED2);
    LedInitialized = SET;
  }

  if(JoyInitialized != SET)
  {
    BSP_JOY_Init(JOY_MODE_EXTI);
    JoyInitialized = SET;
  }

  if(LcdInitialized != SET)
  {
    if(BSP_LCD_Init() != LCD_OK)
    {
      Error_Handler();
    }
    LcdInitialized = SET;
  }

  if(TsInitialized != SET)
  {
    if(BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) != TS_OK)
    {
      Error_Handler();
    }
    BSP_TS_ITConfig();
    TsInitialized = SET;
  }

  __HAL_RCC_I2C1_CLK_ENABLE();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == MFX_INT_PIN)
  {
    HAL_NVIC_DisableIRQ(MFX_INT_EXTI_IRQn);
    MfxItOccurred = SET;
  }
  else if(GPIO_Pin == SEL_JOY_PIN)
  {
    JoyState = JOY_SEL;
  }
}

__IO uint32_t errorSrc = 0;
__IO uint32_t errorMsg = 0;
void Mfx_Event(void)
{
  uint32_t irqPending;

  JoyState = JOY_NONE;

  irqPending = MFX_IO_Read(IO_I2C_ADDRESS, MFXSTM32L152_REG_ADR_IRQ_PENDING);

  if(irqPending & MFX_IRQ_PENDING_GPIO)
  {
    uint32_t statusGpio = BSP_IO_ITGetStatus(RIGHT_JOY_PIN | LEFT_JOY_PIN | UP_JOY_PIN | DOWN_JOY_PIN | TS_INT_PIN);

    TouchEvent = statusGpio & TS_INT_PIN;

    uint32_t JoystickStatus = statusGpio & (RIGHT_JOY_PIN | LEFT_JOY_PIN | UP_JOY_PIN | DOWN_JOY_PIN);

    if(JoystickStatus != 0)
    {
      if(JoystickStatus == RIGHT_JOY_PIN)
        JoyState = JOY_RIGHT;
      else if(JoystickStatus == LEFT_JOY_PIN)
        JoyState = JOY_LEFT;
      else if(JoystickStatus == UP_JOY_PIN)
        JoyState = JOY_UP;
      else if(JoystickStatus == DOWN_JOY_PIN)
        JoyState = JOY_DOWN;
    }

    BSP_IO_ITClear(statusGpio);
  }

  if(irqPending & MFX_IRQ_PENDING_ERROR)
  {
    errorSrc = MFX_IO_Read(IO_I2C_ADDRESS, MFXSTM32L152_REG_ADR_ERROR_SRC);
    errorMsg = MFX_IO_Read(IO_I2C_ADDRESS, MFXSTM32L152_REG_ADR_ERROR_MSG);
  }

  irqPending &= ~MFX_IRQ_PENDING_GPIO;
  if(irqPending)
  {
    MFX_IO_Write(IO_I2C_ADDRESS, MFXSTM32L152_REG_ADR_IRQ_ACK, irqPending);
  }

  MfxItOccurred = RESET;
  HAL_NVIC_EnableIRQ(MFX_INT_EXTI_IRQn);
}

void Error_Handler(void)
{
  BSP_LED_On(LED_ORANGE);
  while(1) {}
}

void BSP_ErrorHandler(void)
{
  Error_Handler();
}

#if defined(LCD_DIMMING)
void Dimming_Timer_Enable(RTC_HandleTypeDef * RTCHandle)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef rtcclk;

    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    rtcclk.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    rtcclk.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    HAL_RCCEx_PeriphCLKConfig(&rtcclk);
    __HAL_RCC_RTC_ENABLE();

    HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 0x0F, 0x0F);
    HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);

    RTCHandle->Instance = RTC;
    RTCHandle->Init.HourFormat     = RTC_HOURFORMAT_12;
    RTCHandle->Init.AsynchPrediv   = 0x7F;
    RTCHandle->Init.SynchPrediv    = 0xF9;
    RTCHandle->Init.OutPut         = RTC_OUTPUT_DISABLE;
    RTCHandle->Init.OutPutRemap    = RTC_OUTPUT_REMAP_NONE;
    RTCHandle->Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    RTCHandle->Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;

    if(HAL_RTC_Init(RTCHandle) != HAL_OK) Error_Handler();
    if(HAL_RTCEx_SetWakeUpTimer_IT(RTCHandle, DIMMING_COUNTDOWN, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK) Error_Handler();
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
#if defined(LCD_OFF_AFTER_DIMMING)
  static uint32_t dimming_period_counter = 0;
  if (display_turned_off == 0)
  {
#endif
    if (display_dimmed == 0)
    {
      if (maintain_display == 0)
      {
        brightness = BRIGHTNESS_MAX;
        Timer_Init(&TimHandle);
        display_dimmed = 1;
      }
    }
#if defined(LCD_OFF_AFTER_DIMMING)
    else
    {
      dimming_period_counter++;
      if (dimming_period_counter == LCD_OFF_COUNTDOWN)
      {
        dimming_period_counter = 0;
        display_dimmed         = 0;
        display_turned_off     = 1;
        BSP_LCD_DisplayOff();
      }
    }
  }
#endif
  maintain_display = 0;
}

static void Timer_Init(TIM_HandleTypeDef * TimHandle)
{
  uint32_t uwPrescalerValue = (uint32_t)(SystemCoreClock / 10000) - 1;
  TimHandle->Instance = TIM3;
  TimHandle->Init.Period            = 100 - 1;
  TimHandle->Init.Prescaler         = uwPrescalerValue;
  TimHandle->Init.ClockDivision     = 0;
  TimHandle->Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimHandle->Init.RepetitionCounter = 0;
  if (HAL_TIM_Base_Init(TimHandle) != HAL_OK) Error_Handler();
  if (HAL_TIM_Base_Start_IT(TimHandle) != HAL_OK) Error_Handler();
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  __HAL_RCC_TIM3_CLK_ENABLE();
  HAL_NVIC_SetPriority(TIM3_IRQn, 0x0F, 0x0F);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  HAL_DSI_ShortWrite(&hdsi_discovery, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x51, brightness);
  brightness--;
  if (brightness < BRIGHTNESS_MIN)
  {
    HAL_TIM_Base_Stop_IT(htim);
  }
}
#endif /* LCD_DIMMING */

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  while (1) {}
}
#endif

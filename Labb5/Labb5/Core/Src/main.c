/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
#define MPU_ADDR (0x68 << 1)

#define WHO_AM_I_REG     0x75
#define PWR_MGMT_1_REG   0x6B
#define SMPLRT_DIV_REG   0x19
#define ACCEL_CONFIG_REG 0x1C
#define ACCEL_XOUT_H_REG 0x3B
#define ACCEL_XOUT_L_REG    0x3C
#define ACCEL_YOUT_H_REG    0x3D
#define ACCEL_YOUT_L_REG    0x3E
#define ACCEL_ZOUT_H_REG    0x3F
#define ACCEL_ZOUT_L_REG    0x40

#define RAD2DEG        57.2957795f
#define MAX_ANGLE_DEG  20.0f


float angle_offset = 0.0f;
static volatile uint8_t calibrate_request = 0;

uint16_t leds[8] = {
		LED0_Pin,
		LED1_Pin,
		LED2_Pin,
		LED3_Pin,
		LED4_Pin,
		LED5_Pin,
		LED6_Pin,
		LED7_Pin
};



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
HAL_StatusTypeDef MPU6050_WriteRegister(uint8_t reg, uint8_t data){
	return HAL_I2C_Mem_Write(&hi2c1, MPU_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

HAL_StatusTypeDef MPU6050_ReadRegister(uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Read(&hi2c1, MPU_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, 100);
}


HAL_StatusTypeDef MPU6050_ReadRegisters(uint8_t start_reg, uint8_t *buffer, uint8_t length)
{
	return HAL_I2C_Mem_Read(&hi2c1, MPU_ADDR, start_reg, I2C_MEMADD_SIZE_8BIT, buffer, length, 100);
}

HAL_StatusTypeDef MPU6050_Init(void)
{
	uint8_t check;


	if (MPU6050_ReadRegister(WHO_AM_I_REG, &check) != HAL_OK)
		return HAL_ERROR;

	if (check != 0x68)
		return HAL_ERROR;


	if (MPU6050_WriteRegister(PWR_MGMT_1_REG, 0x00) != HAL_OK)
		return HAL_ERROR;


	MPU6050_WriteRegister(SMPLRT_DIV_REG, 0x07);


	MPU6050_WriteRegister(ACCEL_CONFIG_REG, 0x00);

	return HAL_OK;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == B1_Pin)
	{
		calibrate_request = 1;
	}
}

static void AllLeds_Off(void)
{
	HAL_GPIO_WritePin(GPIOC,
			LED0_Pin|LED1_Pin|LED2_Pin|LED3_Pin|
			LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin,
			GPIO_PIN_RESET);
}

static void Led_SetByIndex(int idx)
{
	switch (idx)
	{
	case 0: HAL_GPIO_WritePin(GPIOC, LED0_Pin, GPIO_PIN_SET); break;
	case 1: HAL_GPIO_WritePin(GPIOC, LED1_Pin, GPIO_PIN_SET); break;
	case 2: HAL_GPIO_WritePin(GPIOC, LED2_Pin, GPIO_PIN_SET); break;
	case 3: HAL_GPIO_WritePin(GPIOC, LED3_Pin, GPIO_PIN_SET); break;
	case 4: HAL_GPIO_WritePin(GPIOC, LED4_Pin, GPIO_PIN_SET); break;
	case 5: HAL_GPIO_WritePin(GPIOC, LED5_Pin, GPIO_PIN_SET); break;
	case 6: HAL_GPIO_WritePin(GPIOC, LED6_Pin, GPIO_PIN_SET); break;
	case 7: HAL_GPIO_WritePin(GPIOC, LED7_Pin, GPIO_PIN_SET); break;
	default: break;
	}
}

static void Uart_PrintAngle(float angle_deg)
{
	char msg[64];
	int len = snprintf(msg, sizeof(msg), "Angle: %.2f deg\r\n", angle_deg);
	if (len > 0)
	{
		(void)HAL_UART_Transmit(&huart2, (uint8_t*)msg, (uint16_t)len, 100);
	}
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	MX_I2C1_Init();
	/* USER CODE BEGIN 2 */

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	if (MPU6050_Init() != HAL_OK)
	{
		const char *err = "MPU6050 init FAILED\r\n";
		(void)HAL_UART_Transmit(&huart2, (uint8_t*)err, (uint16_t)strlen(err), 100);
		Error_Handler();
	}
	calibrate_request = 1;

	while (1)
	{
		uint8_t b[6];
		int16_t ay, az;

		if (MPU6050_ReadRegisters(ACCEL_XOUT_H_REG, b, 6) == HAL_OK)
		{
			/* b[0..1]=ax, b[2..3]=ay, b[4..5]=az */
			ay = (int16_t)((b[2] << 8) | b[3]);
			az = (int16_t)((b[4] << 8) | b[5]);

			/* Säkrare än atan(ay/az) */
			float angle = atan2f((float)ay, (float)az) * RAD2DEG;

			/* Kalibrering på knapptryck */
			if (calibrate_request)
			{
				angle_offset = angle;
				calibrate_request = 0;
			}

			angle -= angle_offset;

			/* Logga till terminal */
			Uart_PrintAngle(angle);

			/* LED-visualisering */
			AllLeds_Off();

			float abs_angle = fabsf(angle);

			if (abs_angle < 1.0f)
			{
				/* mitten */
				HAL_GPIO_WritePin(GPIOC, LED3_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC, LED4_Pin, GPIO_PIN_SET);
			}
			else
			{
				/* clamp till ±MAX_ANGLE_DEG */
				if (angle >  MAX_ANGLE_DEG) angle =  MAX_ANGLE_DEG;
				if (angle < -MAX_ANGLE_DEG) angle = -MAX_ANGLE_DEG;

				/* map [-MAX..+MAX] -> [0..7] */
				int idx = (int)((angle + MAX_ANGLE_DEG) * 7.0f / (2.0f * MAX_ANGLE_DEG));

				if (idx < 0) idx = 0;
				if (idx > 7) idx = 7;

				Led_SetByIndex(idx);
			}
		}

		/* 5 Hz */
		HAL_Delay(200);

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */


	}

	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 10;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x10D19CE4;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, LED0_Pin|LED1_Pin|LED2_Pin|LED3_Pin
			|LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, SMPS_EN_Pin|SMPS_V1_Pin|SMPS_SW_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : B1_Pin */
	GPIO_InitStruct.Pin = B1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LED0_Pin LED1_Pin LED2_Pin LED3_Pin
                           LED4_Pin LED5_Pin LED6_Pin LED7_Pin */
	GPIO_InitStruct.Pin = LED0_Pin|LED1_Pin|LED2_Pin|LED3_Pin
			|LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : SMPS_EN_Pin SMPS_V1_Pin SMPS_SW_Pin */
	GPIO_InitStruct.Pin = SMPS_EN_Pin|SMPS_V1_Pin|SMPS_SW_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : SMPS_PG_Pin */
	GPIO_InitStruct.Pin = SMPS_PG_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(SMPS_PG_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : LD4_Pin */
	GPIO_InitStruct.Pin = LD4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LD4_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

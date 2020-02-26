/*
  Скетч к проекту "Универсальный контроллер"
  Страница проекта (схемы, описания): https://alexgyver.ru/gyvercontrol/
  Исходники на GitHub: https://github.com/AlexGyver/gyvercontrol
  Нравится, как написан и закомментирован код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver Technologies, 2019
  http://AlexGyver.ru/
*/

/*
  ===== Версия 1.5 =====
  - Облегчённые библиотеки:
    - Для часов используется библиотека microDS3231
    - Для дисплея используется библиотека microLiquidCrystal_I2C
    - Для ds18b20 используется библиотека microDS18B20
    - Для BME280 используется библиотека GyverBME280
  - Чуть оптимизации кода
  - Исправлен баг с выводом инверсных состояний реле в окне DEBUG
  - Добавлена возможность работы с отрицательными температурами в режиме Sensor
  - Исправлено незапоминание настроек SP и PP в Service
  - ServoSmooth обновлена, работа серво улучшена
  - Исправлен баг с сохранением настроек
    - Ваши настройки при переходе на 1.5 будут сброшены!
  - В ПИД установка переделана на десятичные дроби
  - BME и Dallas выводят температуру в десятичных дробях
  - Шаги настроек изменены на более мелкие
  - Исправлена настройка времени в сервисе
  - Добавлен режим ПИД для каналов 1 и 2 (низкочастотный ШИМ). Каналы помечены *
  - Для "обратного" режима ПИД нужно ставить отрицательные коэффициенты!
  - В недельке можно выбрать время включения меньше времени выключения
*/

// -------------------- НАСТРОЙКИ ---------------------
// ======= ЭНКОДЕР =======
#define ENCODER_TYPE 1    // тип энкодера (0 или 1). Если энкодер работает некорректно (пропуск шагов/2 шага), смените тип
#define ENC_REVERSE 1     // 1 - инвертировать направление энкодера, 0 - нет
#define CONTROL_TYPE 1    // тип управления энкодером:
// 0 - удерживание и поворот для изменения значения
// 1 - клик для входа в изменение, повторный клик для выхода (стрелочка меняется на галочку)

// ======= РАЗНОЕ =======
#define DRIVER_LEVEL 1    // 1 или 0 - уровень сигнала на драйвер/реле для привода
#define SERVO1_RELAY 0    // 1 - заменить серво 1 на реле. 0 - ничего не делать
#define SERVO2_RELAY 0    // 1 - заменить серво 2 на реле. 0 - ничего не делать
#define PWM_RELAY_HZ 1    // частота ШИМ для ШИМ-реле, Гц (раз в секунду) можно десятичные дроби (0.1 - период будет 10 секунд)

// === НАЗВАНИЯ КАНАЛОВ === (только английские буквы)
const char *channelNames[] = {
  "Channel 1",
  "Channel 2",
  "Channel 3",
  "Channel 4",
  "Channel 5",
  "Channel 6",
  "Channel 7",
  "Servo 1",
  "Servo 2",
  "Drive",
};

// ======== СИСТЕМА =======
#define WDT_ENABLE 0        // 1 - включить, 0 - отключить watchdog (только для optiboot)
#define USE_PLOTS 0         // 1 - включить, 0 - отключить вывод графиков
#define USE_PID 0           // включает/отключает поддержку ПИД регулятора на каналах 2, 3, серво и привода
#define USE_PID_RELAY 0     // включает/отключает поддержку ПИД регулятора на каналах 0 и 1 (ШИМ-реле)
#define USE_DAWN 0          // включает/отключает поддержку режима РАССВЕТ на каналах 2, 3, серво

#define SETT_TIMEOUT 100    // таймаут неактивности (секунд) после которого автоматически откроется DEBUG и сохранятся настройки
#define LCD_ADDR 0x3f       // адрес дисплея - 0x27 или 0x3f . Смени если не работает!!
#define BME_ADDR 0x76       // адрес BME280 - 0x76 или 0x77. Смени если не работает!! (добавлено в v1.1.1)

// ======== ДАТЧИКИ =======
// цифровой датчик температуры и влажности bme280 (шина i2c)
#define USE_BME 1           // 1 - использовать BME280, 0 - не использовать

// цифровой датчик температуры ds18b20 (вход SENS1)
#define DALLAS_SENS1 0      // 1 - ко входу SENS1 подключен ds18b20, 0 - подключен обычный аналоговый датчик

// цифровой датчик температуры и влажности DHT11/DHT22 (вход SENS2) - вместо BME280
#define DHT_SENS2 0         // 1 - ко входу SENS2 подключен DHT11/DHT22, 0 - подключен обычный аналоговый датчик
#define DHT_TYPE DHT22       // тип DHT датчика: DHT11 или DHT22

// термисторы
#define THERM1 0            // 1 - ко входу SENS1 подключен термистор, 0 - подключен другой аналоговый датчик
#define BETA_COEF1 3435     // температурный коэффициент термистора 1 (см. даташит)

#define THERM2 0            // 1 - ко входу SENS2 подключен термистор, 0 - подключен другой аналоговый датчик
#define BETA_COEF2 3435     // температурный коэффициент термистора 2 (см. даташит)

#define THERM3 0            // 1 - ко входу SENS3 подключен термистор, 0 - подключен другой аналоговый датчик
#define BETA_COEF3 3435     // температурный коэффициент термистора 3 (см. даташит)

#define THERM4 0            // 1 - ко входу SENS4 подключен термистор, 0 - подключен другой аналоговый датчик
#define BETA_COEF4 3435     // температурный коэффициент термистора 4 (см. даташит)

// -------------------- ПИНЫ ---------------------
#define SW        0
#define RELAY_0   1
#define DT        2
#define CLK       3
#define RELAY_1   4
#define RELAY_2   5
#define RELAY_3   6
#define RELAY_4   7
#define RELAY_5   8
#define RELAY_6   9
#define DRV_SIGNAL1 10
#define DRV_PWM     11
#define DRV_SIGNAL2 12
#define SERVO_0   13
#define SERVO_1   A0
#define SENS_VCC  A1
#define SENS_1    A2
#define SENS_2    A3
#define SENS_3    A6
#define SENS_4    A7

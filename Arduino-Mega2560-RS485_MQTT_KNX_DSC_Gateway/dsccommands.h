// Commands to DSC
const char POLL[]                               PROGMEM = "000";
const char STATUS_REPORT[]                      PROGMEM = "001";
const char SET_TIME_DATE[]                      PROGMEM = "010";
const char COMMAND_OUTPUT_CONTROL[]             PROGMEM = "020";
const char PARTITION_ARM_AWAY[]                 PROGMEM = "030";
const char PARTITION_ARM_STAY[]                 PROGMEM = "031";
const char PARTITION_ARM_ZERO_ENTRY[]           PROGMEM = "032";
const char PARTITION_ARM_CONTROL[]              PROGMEM = "033"; // Needs PIN Code
const char PARTITION_DISARM_CONTROL[]           PROGMEM = "040"; // Needs PIN Code
const char DESCRIPTIVE_ARMING_ENABLE[]          PROGMEM = "050";

const char CODE_SEND[]                          PROGMEM = "200";

// Commands from DSC
const uint16_t COMMAND_ACKNOWLEDGE              PROGMEM = 500;
const uint16_t COMMAND_ERROR                    PROGMEM = 501;
const uint16_t SYSTEM_ERROR                     PROGMEM = 502;
const uint16_t TIME_DATE_BROADCAST              PROGMEM = 550;

const uint16_t ZONE_ALARM                       PROGMEM = 601; // MQTT 2
const uint16_t ZONE_ALARM_RESTORE               PROGMEM = 602; // MQTT 3
const uint16_t ZONE_TAMPER                      PROGMEM = 603; // MQTT 4
const uint16_t ZONE_TAMPER_RESTORE              PROGMEM = 604; // MQTT 5
const uint16_t ZONE_FAULT                       PROGMEM = 605; // MQTT 6
const uint16_t ZONE_FAULT_RESTORE               PROGMEM = 606; // MQTT 7
const uint16_t ZONE_OPEN                        PROGMEM = 609; // MQTT 1
const uint16_t ZONE_RESTORED                    PROGMEM = 610; // MQTT 0

const uint16_t PARTITION_READY                  PROGMEM = 650; // MQTT 4
const uint16_t PARTITION_NOT_READY              PROGMEM = 651; // MQTT 5
const uint16_t PARTITION_ARMED                  PROGMEM = 652; // MQTT 0, 1, 2, 3
const uint16_t PARTITION_IN_ALARM               PROGMEM = 654; // MQTT 6
const uint16_t PARTITION_DISARMED               PROGMEM = 655; // MQTT 7
const uint16_t EXIT_DELAY_IN_PROGRESS           PROGMEM = 656; // MQTT 8
const uint16_t ENTRY_DELAY_IN_PROGRESS          PROGMEM = 657; // MQTT 9
const uint16_t KEYPAD_LOCKOUT                   PROGMEM = 658;
const uint16_t INVALID_ACCES_CODE               PROGMEM = 670;
const uint16_t FUNCTION_NOT_AVAILABLE           PROGMEM = 671;
const uint16_t USER_CLOSING                     PROGMEM = 700;
const uint16_t SPECIAL_CLOSING                  PROGMEM = 701;
const uint16_t PARTIAL_CLOSING                  PROGMEM = 702;
const uint16_t USER_OPENING                     PROGMEM = 750;
const uint16_t SPECIAL_OPENING                  PROGMEM = 751;
const uint16_t PANEL_BATTERY_TROUBLE            PROGMEM = 800;
const uint16_t PANEL_BATTERY_TROUBLE_RESTORE    PROGMEM = 801;
const uint16_t PANEL_AC_TROUBLE                 PROGMEM = 802;
const uint16_t PANEL_AC_TROUBLE_RESTORE         PROGMEM = 803;

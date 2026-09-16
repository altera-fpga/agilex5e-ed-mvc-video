#ifndef _UTILS_H_
#define _UTILS_H_
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include "types.h"

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif // UNUSED

//==================================================================

/**
 * @brief Enumeration representing the logging levels.
 */
typedef enum _eLogLevel
{
    kSilent = 0, /**< Silent logging level, no output. */
    kError,      /**< Error logging level. */
    kWarning,    /**< Warning logging level. */
    kInfo,       /**< Info logging level. */
    kDebug,      /**< Debug logging level. */
    kVerbose,    /**< Verbose logging level. */
    kMaxLogLevel, /**< Maximum logging level. */
    kSuppress = 0x80000000 /**< OR'ed with the level to indicate suppression of the log level string. */
} eLogLevel;

/**
 * @brief Sets the logging level for the application.
 *
 * @param[in] new_level The new logging level to set.
 * @return The old logging level.
 */
eLogLevel set_log_level(const eLogLevel new_level);

/**
 * @brief Elevates the logging level to at least the specified new level.
 *
 * @param[in] new_level The new logging level to elevate to.
 * @return The old logging level.
 */
eLogLevel elevate_log_level(const eLogLevel new_level);

/**
 * @brief Gets the current logging level.
 *
 * @return The current logging level.
 */
eLogLevel get_log_level();

/**
 * @brief Converts a logging level to its string representation.
 *
 * @param[in] level The logging level to convert.
 * @return A string representation of the logging level.
 */
const char *logLevelToString(eLogLevel level);

/**
 * @brief Logs a message with the specified logging level and identifier.
 *
 * @param[in] level The logging level of the message.
 * @param[in] pId The identifier for the log message.
 * @param[in] format The format string for the log message.
 * @param[in] ... Additional arguments for the format string.
 */
void logMessagePrint(eLogLevel level, const char *pId, const char *format, ...);

/**
 * @brief Helper macro to ensure logPrint macro is expanded correctly.
 */
#define LOG_EXPAND(x) x

struct MenuItem;

//==================================================================
// Menu Subsystem

/**
 * @brief Max command length.
 */
#define MAX_MENU_CODE (3)

/**
 * @brief Structure representing a menu buffer.
 */
typedef struct _menu_buffer_t
{
    char input[MAX_MENU_CODE + 1];                      /**< Buffer to hold the menu input buffer. */
    uint32_t input_idx;                                 /**< Index of the input. */
} menu_buffer_t;

/**
 * @brief Forward declaration of an application context which is used by your application.
 */
typedef struct app_context_t app_context_t;

/**
 * @brief Structure representing a structure used to hold allow the menu-ing system to work.
 */
typedef struct _utils_context_t
{
    app_context_t *pApp;
    menu_buffer_t menu;
} utils_context_t;

/**
 * @brief Function pointer type for menu actions.
 */
typedef bool (*MenuFunction)(utils_context_t *pContext, const char *pCode, const struct MenuItem *pMenuList, const uint32_t metadata);

/**
 * @brief Structure representing a menu item.
 */
struct MenuItem
{
    char code[MAX_MENU_CODE + 1];  /**< Code to select the action. */
    char *pDescription;            /**< Description of the menu item. */
    MenuFunction action;           /**< Function to execute the menu item action. */
    const struct MenuItem  *pExtItems; /**< A Second list of menu items to be scanned */
    uint32_t metadata;
};

/**
 * @brief Executes a menu item based on the provided code.
 *
 * This function executes the action associated with the menu item matching the provided code.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the menu item to execute.
 * @param[in] pMenuList Pointer to the start of a menu list
 * @return True if the menu item is successfully executed, false otherwise.
 */
bool executeMenuItem(utils_context_t *pContext, const char *pCode, const struct MenuItem *pMenuList);

/**
 * @brief Checks and processes menu input.
 *
 * This function checks for user input and processes menu commands based on the input.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pMenuItems Pointer to an array of MenuItems
 */
void CheckMenuInput(utils_context_t *pContext, const struct MenuItem *pMenuItems);

/**
 * @brief Executes utility menu functions based on the provided code.
 *
 * This function performs actions such as setting log levels and clearing the screen based on the provided code.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool UtilsMenuFunction(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata);

/**
 * @brief List of Logging Menu Items
 */
extern struct MenuItem _LogMenuItems[];
#endif // _UTILS_H_
